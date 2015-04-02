//press space to start

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include <vector>
#include <string>

#include <fstream>
#include <iostream>
#include <sstream>

#include <SDL_mixer.h>
using namespace std;

SDL_Window* displayWindow;

// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
float timeLeftOver = 0.0f;

#define SPRITE_COUNT_X 14
#define SPRITE_COUNT_Y 8
#define TILE_SIZE 0.0625f

enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };
int state;
float gravity_x = 0.0f;
float gravity_y = -9.8f;
unsigned int mapWidth;
unsigned int mapHeight;
unsigned char** levelData;


Mix_Music* music;
Mix_Chunk* pressSpaceSound;
Mix_Chunk* randomSound;

string winLose = "lose";
bool done = false;


class Entity {
public:
	~Entity();
	Entity(float width, float height) :
		width(width), height(height)
	{
		speed = 0.0f;
		velocity_x = 0.0f;
		velocity_y = 0.0f;
		acceleration_x = 0.0f;
		acceleration_y = 0.0f;
		friction_x = 0.0f;
		friction_y = 0.0f;
		alive = true;
	}
	bool operator==(const Entity* other) const;
	void render(GLuint& spriteTexture);

	float xPos, yPos, width, height;
	float speed, velocity_x, velocity_y, acceleration_x, acceleration_y, friction_x, friction_y;

	bool collidedTop, collidedBottom, collidedLeft, collidedRight;

	string type;
	bool alive;

};

bool Entity::operator== (const Entity* other) const {
	return this == other;
}


//vectors
vector<Entity*> entities;
vector<float> vertexData;
vector<float> texCoordData;

float lerp(float v0, float v1, float t) {
	return (1.0f - t)*v0 + t*v1;
}

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	SDL_FreeSurface(surface);
	return textureID;
}

GLuint textSheet;
GLuint mapSheet;
GLuint charSheet;


void drawTexture(float x, float y, GLfloat* vertexVector, GLfloat* textureVector, GLuint textureID){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glVertexPointer(2, GL_FLOAT, 0, vertexVector);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, textureVector);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
}


void drawSprite(float x, float y, GLuint spriteTexture, int index, int spriteCountX, int spriteCountY) {
	//width and height = tile_size
	float width = 0.0625f;
	float height = 0.0625f;
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0f / (float)spriteCountX;
	float spriteHeight = 1.0f / (float)spriteCountY;
	GLfloat quad[] = { x + 0.5f * width, y - 0.5f * height, x + 0.5f * width, y + 0.5f * height, x - 0.5f * width, y + 0.5f * height, x - 0.5f * width, y - 0.5f * height }; 
	GLfloat quadUVs[] = { u, v, u, v - spriteHeight, u + spriteWidth, v - spriteHeight, u + spriteWidth, v };
	drawTexture(x, y, quad, quadUVs, spriteTexture);
}

void drawMap(float x, float y, GLuint spriteTexture, int index, int spriteCountX, int spriteCountY) {
	//width and height = tile_size
	float width = 0.0625f;
	float height = 0.0625f;
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0f / (float)spriteCountX;
	float spriteHeight = 1.0f / (float)spriteCountY;
	drawTexture(x, y, vertexData.data(), texCoordData.data(), spriteTexture);
}

void Entity::render(GLuint& spriteTexture){
	if (type == "player") {
		drawSprite(xPos, yPos, spriteTexture, 24, 12, 8);
	}
	else if (type == "enemy") {
		drawSprite(xPos, yPos, spriteTexture, 88, 12, 8);
	}
	else if (type == "platform"){
		drawSprite(xPos, yPos, spriteTexture, 11, 14, 8);
	}
}

bool isColliding(Entity* e1, Entity* e2){

	float e1Top = e1->yPos + e1->height * 0.5f;
	float e1Bot = e1->yPos - e1->height * 0.5f;
	float e1Left = e1->xPos - e1->width * 0.5f;
	float e1Right = e1->xPos + e1->width * 0.5f;

	float e2Top = e2->yPos + e2->height * 0.5f;
	float e2Bot = e2->yPos - e2->height * 0.5f;
	float e2Left = e2->xPos - e2->width * 0.5f;
	float e2Right = e2->xPos + e2->width * 0.5f;

	if (e1Bot > e2Top){
		return false;
	}
	if (e1Top < e2Bot){
		return false;
	}
	if (e1Left > e2Right){
		return false;
	}
	if (e1Right < e2Left){
		return false;
	}
	return true;
}

void handleCollisionY(Entity* entity){

}

void handleCollisionX(Entity* entity){
	
}

void DrawText(int fontTexture, string text, float size, float spacing, float r, float g, float b, float a) {
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexDataText;
	vector<float> texCoordDataText;
	vector<float> colorDataText;
	for (size_t i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		colorDataText.insert(colorDataText.end(), { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a });
		vertexDataText.insert(vertexDataText.end(), { ((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) +
			(-0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f
			* size });
		texCoordDataText.insert(texCoordDataText.end(), { texture_x, texture_y, texture_x, texture_y + texture_size, texture_x +
			texture_size, texture_y + texture_size, texture_x + texture_size, texture_y });
	}
	glColorPointer(4, GL_FLOAT, 0, colorDataText.data());
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertexDataText.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordDataText.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, text.size() * 4);

	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


bool readHeader(std::ifstream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (size_t i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool readLayer(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (size_t y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (size_t x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}
/*
void placeTile(){
	float x;
	float y;
	for (size_t j = 0; j < mapHeight; j++) {
		y = j * -0.0625f;
		for (size_t i = 0; i < mapWidth; i++) {
			x = i * 0.0625f;
			if (levelData[j][i] == 39 || levelData[j][i] == 11) {
				Entity* entity = new Entity(0.625f, 0.625f);
				entity->type = "static";
				entity->xPos = x + 0.5f * entity->width;
				entity->yPos = y - 0.5f * entity->height;
				tiles.push_back(entity);
			}
		}
	}
}
*/

void placeEntity(string& type, float& x, float& y){
	if (type == "player") {
		Entity* entity = new Entity(0.0625f, 0.0625f);
		entity->type = type;
		entity->xPos = x + 0.5f * entity->width;
		entity->yPos = y + 0.5f * entity->height;
		entities.push_back(entity);
	}
	else if (type == "enemy") {
		Entity* entity = new Entity(0.0625f, 0.0625f);
		entity->type = type;
		entity->xPos = x + 0.5f * entity->width;
		entity->yPos = y + 0.5f * entity->width;
		entities.push_back(entity);
	}
	else if (type == "platform") {
		Entity* entity = new Entity(0.0625f, 0.0625f);
		entity->type = type;
		entity->xPos = x + 0.5f * entity->width;
		entity->yPos = y + 0.5f * entity->width;
		entities.push_back(entity);
	}
}

bool readEntity(ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { continue; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str()) * 0.0625f;
			float placeY = atoi(yPosition.c_str()) * -0.0625f;
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}
void insertLevelData(){
	for (size_t y = 0; y < mapHeight; y++) {
		for (size_t x = 0; x < mapWidth; x++) {
			float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
			float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
			vertexData.insert(vertexData.end(), {
				TILE_SIZE * x, -TILE_SIZE * y,
				TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
			});
			texCoordData.insert(texCoordData.end(), { u, v,
				u, v + (spriteHeight),
				u + spriteWidth, v + (spriteHeight),
				u + spriteWidth, v
			});
		}
	}
}

void Setup(){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, 800, 600);
	//glOrtho(-2.66, 2.66, -2.0, 2.0, -2.0, 2.0);
	glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	textSheet = LoadTexture("scrollAndSound/font2.png");
	mapSheet = LoadTexture("scrollAndSound/sheet_4.png");
	charSheet = LoadTexture("scrollAndSound/characters_1.png");

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	music = Mix_LoadMUS("music.mp3");
	pressSpaceSound = Mix_LoadWAV("spaceSound.wav");
	randomSound = Mix_LoadWAV("randomSound.wav");


	Mix_Volume(1, 50);
	Mix_Volume(2, 50);
	Mix_VolumeMusic(20);
	Mix_PlayMusic(music, -1);
	
	ifstream infile("infinite_run.txt");
	if (!(readHeader(infile))) { done = true; }
	if (!(readLayer(infile))) { done = true; }
}

void updateMenu(float elapsed){
	SDL_Event event;


	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				state = STATE_GAME_LEVEL;
				ifstream infile("infinite_run.txt");
				if (!(readEntity(infile))) { done = true; }
				//else{ placeTile(); }

				Mix_PlayChannel(1, pressSpaceSound, 0);
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
				state = STATE_GAME_OVER;
				Mix_PlayChannel(2, randomSound, 0);
			}
		}
	}
}

void updateGame(float elapsed){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {


		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			Mix_FreeMusic(music);
			Mix_FreeChunk(randomSound);
			Mix_FreeChunk(pressSpaceSound);
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
			state = STATE_GAME_OVER;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			for (size_t i = 0; i < entities.size(); i++){
				if (entities[i]->type == "player"){
					if (entities[i]->collidedBottom){
						entities[i]->collidedBottom = false;
						entities[i]->velocity_y = 3.0f;
					}
				}
			}
		}


	}
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT]) {	
		for (size_t i = 0; i < entities.size(); i++){
			if (entities[i]->type == "player"){
				if (entities[i]->velocity_x > 0.0f)
					entities[i]->velocity_x = 0.0f;
				if (entities[i]->velocity_x > -2.5f)
					entities[i]->acceleration_x = -2.8f;
			}
		}
		
		
	}

	if (keys[SDL_SCANCODE_RIGHT]) {
		for (size_t i = 0; i < entities.size(); i++){
			if (entities[i]->type == "player"){
				if (entities[i]->velocity_x < 0.0f)
					entities[i]->velocity_x = 0.0f;
				if (entities[i]->velocity_x < 2.5f)
					entities[i]->acceleration_x = 2.8f;
			}
			
			if (entities[i]->type == "player" && entities[i]->xPos >= 1.25f){
			
			}
			
		}
		
		
	}

	if ((!keys[SDL_SCANCODE_RIGHT]) && (!keys[SDL_SCANCODE_LEFT])){
		for (size_t i = 0; i < entities.size(); i++){
			if (entities[i]->type == "player"){
				entities[i]->velocity_x = 0.0f;
				entities[i]->acceleration_x = 0.0f;
			}
		}
	}

	for (size_t i = 0; i < entities.size(); i++){
		for (size_t j = 0; j < entities.size(); j++){
			if (entities[i]->type == "player" && entities[j]->type == "enemy"){
				if (isColliding(entities[i], entities[j])){
					winLose = "lose";
					state = STATE_GAME_OVER;
				}
			}
		}
	}

	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i]->type == "player" && entities[i]->yPos < -2.5f){
			winLose = "lose";
			state = STATE_GAME_OVER;
		}
	}

	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i]->type == "player" && entities[i]->xPos > 5.0f){
			winLose = "win";
			state = STATE_GAME_OVER;
		}
	}
		
}

void updateGameOver(float elapsed){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}

void fixedUpdate(){
	
	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i]->collidedTop){
			entities[i]->velocity_y = 0.0f;
		}
		if (entities[i]->collidedBottom){
			entities[i]->velocity_y = 0.0f;
		}
		if (entities[i]->collidedLeft){
			entities[i]->velocity_x = 0.0f;
		}
		if (entities[i]->collidedRight){
			entities[i]->velocity_x = 0.0f;
		}
		
		entities[i]->collidedTop = false;
		entities[i]->collidedBottom = false;
		entities[i]->collidedLeft = false;
		entities[i]->collidedRight = false;
		

		if (entities[i]->type != "platform"){
		entities[i]->velocity_x += gravity_x * FIXED_TIMESTEP;
		entities[i]->velocity_y += gravity_y * FIXED_TIMESTEP;
		}
		

		entities[i]->velocity_x = lerp(entities[i]->velocity_x, 0.0f, FIXED_TIMESTEP * entities[i]->friction_x);
		entities[i]->velocity_y = lerp(entities[i]->velocity_y, 0.0f, FIXED_TIMESTEP * entities[i]->friction_y);
		entities[i]->velocity_x += entities[i]->acceleration_x * FIXED_TIMESTEP;
		entities[i]->velocity_y += entities[i]->acceleration_y * FIXED_TIMESTEP;
		
		entities[i]->yPos += entities[i]->velocity_y * FIXED_TIMESTEP;
		for (size_t j = 0; j < entities.size(); j++){
			if (isColliding(entities[i], entities[j]) && entities[i] != entities[j]){
				if (entities[j]->type == "platform"){
					float y_penetration = fabs(fabs(entities[i]->yPos - entities[j]->yPos) - entities[i]->height / 2.0f - entities[j]->height / 2.0f);
					if (entities[i]->yPos > entities[j]->yPos){
						entities[i]->yPos += y_penetration + 0.0001f;
						entities[i]->velocity_y = 0.0f;
						entities[i]->collidedBottom = true;
					}
					else if (entities[i]->yPos < entities[j]->yPos){
						entities[i]->yPos -= y_penetration + 0.0001f;
						entities[i]->velocity_y = 0.0f;
						entities[i]->collidedTop = true;
					}
				}
			}
		}

		entities[i]->xPos += entities[i]->velocity_x * FIXED_TIMESTEP;
		for (size_t j = 0; j < entities.size(); j++){
			if (isColliding(entities[i], entities[j])){
				if (entities[j]->type == "platform"){
					float x_penetration = fabs(fabs(entities[i]->xPos - entities[j]->xPos) - entities[i]->width / 2.0f - entities[j]->width / 2.0f);
					if (entities[i]->xPos > entities[j]->xPos){
						entities[i]->xPos += x_penetration + 0.001f;
						entities[i]->collidedLeft = true;
					}
					else if (entities[i]->xPos < entities[j]->xPos){
						entities[i]->xPos -= x_penetration + 0.001f;
						entities[i]->collidedRight = true;
					}
				}
			}
		}

	}

}
		

	
		



void update(float elapsed) {
	switch (state) {
	case STATE_MAIN_MENU:
		updateMenu(elapsed);
		break;
	case STATE_GAME_LEVEL:
		updateGame(elapsed);
		break;
	case STATE_GAME_OVER:
		updateGameOver(elapsed);
		break;

	}
}

void renderMenu() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.0f, 0.3f, 0.0f);
	DrawText(textSheet, "Press Space to Jump, Go right", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-1.0f, 0.0f, 0.0f);
	DrawText(textSheet, "Watch out for cracks", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-1.0f, -0.3f, 0.0f);
	DrawText(textSheet, "Press SPACE to start the game, BACKSPACE to QUIT", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	//USE TO cout VALUES
	/*
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);
	DrawText(textTexture, to_string(ships[0]->v), 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	*/

}


void renderGame() {


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	int playerEntitiesIndex = 0;
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->type == "player") { playerEntitiesIndex = i; }
	}
	glTranslatef(-entities[playerEntitiesIndex]->xPos, -entities[playerEntitiesIndex]->yPos, 0.0);
	
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->alive && !(entities[i]->type == "platform")) {
			entities[i]->render(charSheet);
		}
	}
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->alive && entities[i]->type == "platform") {
			entities[i]->render(charSheet);
		}
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
}
void renderGameOver() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-0.8f, 0.0f, 0.0f);
	if (winLose == "win"){
		DrawText(textSheet, "YOU WIN", 0.2f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (winLose == "lose"){
		DrawText(textSheet, "YOU LOSE", 0.2f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}


void render() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (state) {
	case STATE_MAIN_MENU:
		renderMenu();
		break;
	case STATE_GAME_LEVEL:
		renderGame();
		break;
	case STATE_GAME_OVER:
		renderGameOver();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);

}

int main(int argc, char *argv[]){

	Setup();

	insertLevelData();

	float lastFrameTicks = 0.0f;
	
	while (!done) {
		
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		float fixedElapsed = elapsed + timeLeftOver;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
			fixedUpdate();
		}
		timeLeftOver = fixedElapsed;
		
		update(fixedElapsed);
		render();	
	}
	SDL_Quit();
	return 0;
}