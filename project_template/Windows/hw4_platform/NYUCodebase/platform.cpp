//press space to start

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include <vector>
#include <string>
using namespace std;

SDL_Window* displayWindow;
bool done = false;
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };
int state;
float gravity_x = 0.0f;
float gravity_y = -9.8f;

// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
float timeLeftOver = 0.0f;


GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	SDL_FreeSurface(surface);
	return textureID;
}


float lerp(float v0, float v1, float t) {
	return (1.0f - t)*v0 + t*v1;
}

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(int textureID, float u, float v, float width, float height) :
		textureID(textureID), u(u), v(v), width(width), height(height)
	{
		xPos = 0.0f;
		yPos = 0.0f;
		width = 0.0f;
		height = 0.2f;
		rotation = 0.0f;
		scale = 0.0f;
		velocity_x = 0.0f;
		velocity_y = 0.0f;
		acceleration_x = 0.0f;
		acceleration_y = 0.0f;
		friction_x = 0.0f;
		friction_y = 0.0f;
		isStatic = false;
		collidedTop = false;
		collidedBottom = false;
		collidedLeft = false;
		collidedRight = false;

		isStatic = false;

		isPlayer = false;
		canJump = false;

		isItem = false;
		isCollected = false;
	}
	void Draw(float scale, float x, float y, float rotation);
	bool operator==(const SheetSprite* other) const;
	int textureID;
	float u;
	float v;
	float width;
	float height;
	float xPos;
	float yPos;
	float rotation;
	float scale;

	float x_dir;
	float y_dir;
	float speed;
	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;
	float friction_x;
	float friction_y;

	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;

	bool isStatic;

	bool isPlayer;
	bool canJump;

	bool isItem;
	bool isCollected;

};

vector<SheetSprite*> entities;

bool SheetSprite::operator== (const SheetSprite* other) const {
	return this == other;
}

void SheetSprite::Draw(float scale, float x, float y, float rotation) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 0.0f);
	//glRotatef(rotation, 0.0f, 0.0f, 1.0f);
	//glScalef(width, height, 1.0f);

	GLfloat quad[] = { -width * scale, height * scale, -width * scale, -height * scale,
		width * scale, -height * scale, width * scale, height * scale };

	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);

	GLfloat quadUVs[] = { u, v, u, v + height, u + width, v + height, u + width, v };

	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);

	// draw arrays
}

bool isColliding(SheetSprite* e1, SheetSprite* e2){
	if (e1->yPos - e1->height * 0.5 > e2->yPos + e2->height * 0.5){
		return false;
	}
	if (e1->yPos + e1->height * 0.5 < e2->yPos - e2->height * 0.5){
		return false;
	}
	if (e1->xPos - e1->width * 0.5 > e2->xPos + e2->width * 0.5){
		return false;
	}
	if (e1->xPos + e1->width * 0.5 < e2->xPos - e2->width * 0.5){
		return false;
	}
	return true;
}
/*
bool isCollidingY(SheetSprite* e1, SheetSprite* e2){
	if (e1->yPos + e1->height * 0.5 < e2->yPos - e2->height * 0.5){
		return false;
	}
	if (e1->yPos - e1->height * 0.5 > e2->yPos + e2->height * 0.5){
		return false;
	}
	return true;
}

bool isCollidingX(SheetSprite* e1, SheetSprite* e2){
	if (e1->xPos - e1->width * 0.5 > e2->xPos + e2->width * 0.5){
		return false;
	}
	if (e1->xPos + e1->width * 0.5 < e2->xPos - e2->width * 0.5){
		return false;
	}
	return true;
}
*/



void DrawText(int fontTexture, string text, float size, float spacing, float r, float g, float b, float a) {
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	vector<float> colorData;
	for (size_t i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		colorData.insert(colorData.end(), { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a });
		vertexData.insert(vertexData.end(), { ((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) +
			(-0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f
			* size });
		texCoordData.insert(texCoordData.end(), { texture_x, texture_y, texture_x, texture_y + texture_size, texture_x +
			texture_size, texture_y + texture_size, texture_x + texture_size, texture_y });
	}
	glColorPointer(4, GL_FLOAT, 0, colorData.data());
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, text.size() * 4);

	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

bool allCollected(){
	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i]->isItem && !entities[i]->isCollected){
			return false;
		}
	}
	return true;
}

void Setup(){
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, 800, 600);
	glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	state = STATE_MAIN_MENU;
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
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
				state = STATE_GAME_OVER;
			}
		}
	}
}

void updateGame(float elapsed, int spriteSheetTexture){
	SDL_Event event;

	if (allCollected()){
		state = STATE_GAME_OVER;
	}

	while (SDL_PollEvent(&event)) {


		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
			state = STATE_GAME_OVER;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			for (size_t i = 0; i < entities.size(); i++){
				if (entities[i]->isPlayer){
					if (entities[i]->collidedBottom){
						entities[i]->collidedBottom = false;
						entities[i]->velocity_y = 5.0f;
					}
				}
			}
		}


	}
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT]) {	
		for (size_t i = 0; i < entities.size(); i++){
			if (entities[i]->isPlayer && entities[i]->xPos > -1.25f){
				if (entities[i]->velocity_x > 0.0f)
					entities[i]->velocity_x = 0.0f;
				if (entities[i]->velocity_x > -2.5f)
					entities[i]->acceleration_x = -9.8f;
			}
			if (entities[i]->isPlayer && entities[i]->xPos <= -1.25f){
				entities[i]->velocity_x = 0.0f;
			}
		}
	}

	else if (keys[SDL_SCANCODE_RIGHT]) {
		for (size_t i = 0; i < entities.size(); i++){
			if (entities[i]->isPlayer && entities[i]->xPos < 1.25f){
				if (entities[i]->velocity_x < 0.0f)
					entities[i]->velocity_x = 0.0f;
				if (entities[i]->velocity_x < 2.5f)
					entities[i]->acceleration_x = 9.8f;
			}
			if (entities[i]->isPlayer && entities[i]->xPos >= 1.25f){
				entities[i]->velocity_x = 0.0f;
			}
		}
	}
		

	else{
		for (size_t i = 0; i < entities.size(); i++){
			if (entities[i]->isPlayer){
				entities[i]->velocity_x = 0.0f;
				entities[i]->acceleration_x = 0.0f;
			}
		}
	}
	for (size_t i = 0; i < entities.size(); i++){
		for (size_t j = 0; j < entities.size(); j++){
			if (entities[i]->isPlayer && entities[j]->isItem){
				if (isColliding(entities[i], entities[j])){
					entities[j]->isCollected = true;
				}
			}
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
			entities[i]->canJump = true;
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


		if (!entities[i]->isStatic){
			entities[i]->velocity_x += gravity_x * FIXED_TIMESTEP;
			entities[i]->velocity_y += gravity_y * FIXED_TIMESTEP;
			
			entities[i]->velocity_x = lerp(entities[i]->velocity_x, 0.0f, FIXED_TIMESTEP * entities[i]->friction_x);
			entities[i]->velocity_y = lerp(entities[i]->velocity_y, 0.0f, FIXED_TIMESTEP * entities[i]->friction_y);
			entities[i]->velocity_x += entities[i]->acceleration_x * FIXED_TIMESTEP;
			entities[i]->velocity_y += entities[i]->acceleration_y * FIXED_TIMESTEP;
			

			entities[i]->yPos += entities[i]->velocity_y * FIXED_TIMESTEP;
			for (size_t j = 0; j < entities.size(); j++){
				if (isColliding(entities[i], entities[j]) && entities[i] != entities[j]){
					if (entities[j]->isStatic){
						float y_penetration = fabs(fabs(entities[i]->yPos - entities[j]->yPos) - entities[i]->height / 2.0f - entities[j]->height / 2.0f);
						if (entities[i]->yPos > entities[j]->yPos){
							entities[i]->yPos += y_penetration + 0.001f;
							entities[i]->collidedBottom = true;
							entities[i]->canJump = true;
						}
						else if (entities[i]->yPos < entities[j]->yPos){
							entities[i]->yPos -= y_penetration + 0.001f;
							entities[i]->collidedTop = true;
						}
					}
				}
			}
			
			entities[i]->xPos += entities[i]->velocity_x * FIXED_TIMESTEP;
			for (size_t j = 0; j < entities.size(); j++){
				if (isColliding(entities[i], entities[j])){
					if (entities[j]->isStatic){
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
		

		if (!entities[i]->collidedBottom){
			entities[i]->canJump = false;
		}

	}
		
}


void update(float elapsed, int bulletTexture) {
	switch (state) {
	case STATE_MAIN_MENU:
		updateMenu(elapsed);
		break;
	case STATE_GAME_LEVEL:
		updateGame(elapsed, bulletTexture);
		break;
	case STATE_GAME_OVER:
		updateGameOver(elapsed);
		break;

	}
}

void renderMenu(int textTexture) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.2f, 0.3f, 0.0f);
	DrawText(textTexture, "Press Space to Jump, Arrow Keys to Move", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-1.2f, 0.0f, 0.0f);
	DrawText(textTexture, "Collect the items to win, watch out for cracks", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-1.2f, -0.3f, 0.0f);
	DrawText(textTexture, "Press SPACE to start the game, BACKSPACE to QUIT", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	//USE TO cout VALUES
	/*
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);
	DrawText(textTexture, to_string(ships[0]->v), 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	*/

}
void renderGame(int textTexture) {

	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i]->isStatic){
			entities[i]->Draw(1.0f, entities[i]->xPos, entities[i]->yPos, 0.0f);
		}
	}
	
	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i]->isItem && !entities[i]->isCollected){
			entities[i]->Draw(2.0f, entities[i]->xPos, entities[i]->yPos, 0.0f);
		}
	}

	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i]->isPlayer){
			entities[i]->Draw(1.0f, entities[i]->xPos, entities[i]->yPos, 0.0f);
		}
	}
	


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
}
void renderGameOver(int textTexture) {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-0.9f, 0.0f, 0.0f);
	DrawText(textTexture, "GAME OVER", 0.2f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}


void render(int textTexture) {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (state) {
	case STATE_MAIN_MENU:
		renderMenu(textTexture);
		break;
	case STATE_GAME_LEVEL:
		renderGame(textTexture);
		break;
	case STATE_GAME_OVER:
		renderGameOver(textTexture);
		break;
	}
	SDL_GL_SwapWindow(displayWindow);

}

void loadSheets(int spriteSheet){
	//buttonBlue.png" x="0" y="78" width="222" height="39"
	//"meteorBrown_big1.png" x="224" y="664" width="101" height="84"
	//"ufoRed.png" x="444" y="0" width="91" height="91"

	SheetSprite* platform1 = new SheetSprite(spriteSheet, 0.0f / 1024.0f, 78.0f / 1024.0f, 222.0f / 1024.0f, 39.0f / 1024.0f);
	platform1->xPos = 0.0f;
	platform1->yPos = -0.8f;
	platform1->isStatic = true;
	entities.push_back(platform1);

	SheetSprite* platform2 = new SheetSprite(spriteSheet, 0.0f / 1024.0f, 78.0f / 1024.0f, 222.0f / 1024.0f, 39.0f / 1024.0f);
	platform2->xPos = -0.5f;
	platform2->yPos = -0.8f;
	platform2->isStatic = true;
	entities.push_back(platform2);

	SheetSprite* platform3 = new SheetSprite(spriteSheet, 0.0f / 1024.0f, 78.0f / 1024.0f, 222.0f / 1024.0f, 39.0f / 1024.0f);
	platform3->xPos = 0.5f;
	platform3->yPos = -0.8f;
	platform3->isStatic = true;
	entities.push_back(platform3);

	SheetSprite* platform4 = new SheetSprite(spriteSheet, 0.0f / 1024.0f, 78.0f / 1024.0f, 222.0f / 1024.0f, 39.0f / 1024.0f);
	platform4->xPos = -1.0f;
	platform4->yPos = -0.8f;
	platform4->isStatic = true;
	entities.push_back(platform4);

	SheetSprite* platform5 = new SheetSprite(spriteSheet, 0.0f / 1024.0f, 78.0f / 1024.0f, 222.0f / 1024.0f, 39.0f / 1024.0f);
	platform5->xPos = 1.0f;
	platform5->yPos = -0.8f;
	platform5->isStatic = true;
	entities.push_back(platform5);

	SheetSprite* item1 = new SheetSprite(spriteSheet, 224.0f / 1024.0f, 664.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f);
	item1->xPos = -0.5f;
	item1->yPos = -0.5f;
	item1->isItem = true;
	entities.push_back(item1);

	SheetSprite* item2 = new SheetSprite(spriteSheet, 224.0f / 1024.0f, 664.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f);
	item2->xPos = 0.5f;
	item2->yPos = -0.5f;
	item2->isItem = true;
	entities.push_back(item2);

	SheetSprite* player = new SheetSprite(spriteSheet, 444.0f / 1024.0f, 0.0f / 1024.0f, 91.0f / 1024.0f, 91.0f / 1024.0f);
	player->xPos = 0.0f;
	player->yPos = 0.8f;
	player->isPlayer = true;
	player->friction_x = 5.0f;
	player->friction_y = 5.0f;
	entities.push_back(player);

}


int main(int argc, char *argv[]){

	Setup();
	 
	GLuint spriteSheet = LoadTexture("platform/sheet.png");
	GLuint text = LoadTexture("platform/font2.png");

	loadSheets(spriteSheet);

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
		
		update(fixedElapsed, spriteSheet);
		render(text);
		
	}



	SDL_Quit();
	return 0;
}