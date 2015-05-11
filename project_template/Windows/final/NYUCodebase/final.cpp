//Final Project "Snake2.0" by Lucas Chen

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <string>
#include <random>
#include <time.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <SDL_mixer.h>
#include "PerlinNoise.h"
using namespace std;

SDL_Window* displayWindow;

// 60 FPS (1.0f/60.0f)
//#define FIXED_TIMESTEP 0.0166666f
// 30 FPS (1.0f/30.0f)
#define FIXED_TIMESTEP 0.0333333f
#define MAX_TIMESTEPS 6
float timeLeftOver = 0.0f;

#define SPRITE_COUNT_X 14
#define SPRITE_COUNT_Y 8
#define TILE_SIZE 0.0625f

int gamestate;
//float gravity_x = 0.0f;
//float gravity_y = -9.8f;
unsigned int mapWidth;
unsigned int mapHeight;
unsigned char** levelData;

float elapsed;

bool screenShaking;
float shakeTimer = 0.0f;
float screenShakeValue = 0.0f;
float screenShakeSpeed = 0.0f;
float screenShakeIntensity = 0.0f;

float perlinValue;

Mix_Music* music;
Mix_Chunk* pressSpaceSound;
Mix_Chunk* randomSound;
Mix_Chunk* bombSound;
Mix_Chunk* collectSound;
Mix_Chunk* crashSound;
Mix_Chunk* beforecrashSound;

float crashTimer = 2.0f;
bool wasState1;
float gameTimer = 60.00f;
int p1current = 0;
int p2current= 0;
int currentGem = 0;
int currentEnemy = 0;
int menuOption = 0;
int p1score = 0;
int p2score = 0;
string winLose = "neither";
bool done = false;


class Entity {
public:
	Entity(){};
	Entity(float width, float height) :
		width(width), height(height)
	{
		speed = 0.0f;
		velocity_x = 0.0f;
		velocity_y = 0.0f;
		turnTimer = 1.0f;
	}
	bool operator==(const Entity* other) const;
	void render(GLuint& spriteTexture);

	float xPos, yPos, width, height;
	float speed, velocity_x, velocity_y;

	bool collidedTop, collidedBottom, collidedLeft, collidedRight;

	string direction;
	bool next;
	string type;
	bool alive;
	float turnTimer;

};

class Snake {
public:
	Snake(){};
	void render(GLuint& spriteTexture);
	vector<Entity*> body;
	bool crashed;
};

bool Entity::operator== (const Entity* other) const {
	return this == other;
}

//vector<Entity*> entities;
vector<Entity*> snake1vect;
vector<Entity*> snake2vect;
vector<Entity*> gems;
vector<Entity*> enemies;
Snake* player1;
Snake* player2;

int random1(){
	//negative or positive one
	int negoneToOne = 0;
	//srand(time(NULL)); // Seed the time
	while (negoneToOne == 0){
		negoneToOne = rand() % 3 + 1;
		negoneToOne = negoneToOne - 2;
	}
	return negoneToOne;
}

int random1To10(){
	int oneToTen;
	//srand(time(NULL)); // Seed the time
	oneToTen = rand() % 10 + 1;
	return oneToTen;
}

int random1To3(){
	int oneToThree;
	//srand(time(NULL)); // Seed the time
	oneToThree = rand() % 3 + 1;
	return oneToThree;
}

int random2To3(){
	int oneToThree = 1;
	//srand(time(NULL)); // Seed the time
	while (oneToThree == 1){
		oneToThree = rand() % 3 + 1;
	}
	return oneToThree;
}

int random1To2(){
	int oneTo2;
	//srand(time(NULL)); // Seed the time
	oneTo2 = rand() % 2 + 1;
	return oneTo2;
}

int random1To90(){
	int oneTo90;
	//srand(time(NULL)); // Seed the time
	oneTo90 = rand() % 90 + 1;
	return oneTo90;
}

int random1To120(){
	int oneTo120;
	//srand(time(NULL)); // Seed the time
	oneTo120 = rand() % 120 + 1;
	return oneTo120;
}

int random1To8Music(){
	int oneTo8;
	//srand(time(NULL)); // Seed the time
	oneTo8 = rand() % 8 + 1;
	return oneTo8;
}


//vectors
vector<float> vertexData;
vector<float> texCoordData;

float lerp(float from, float to, float time) {
	return (1.0f - time)*from + time*to;
}

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
	float retVal = dstMin + ((value - srcMin) / (srcMax - srcMin) * (dstMax - dstMin));
	if (retVal < dstMin) {
		retVal = dstMin;
	}
	if (retVal > dstMax) {
		retVal = dstMax;
	}
	return retVal;
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
GLuint etcSheet;


void drawTexture(float x, float y, GLfloat* vertexVector, GLfloat* textureVector, GLuint textureID){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glVertexPointer(2, GL_FLOAT, 0, vertexVector);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, textureVector);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

void drawTextureGlow(float x, float y, GLfloat* vertexVector, GLfloat* textureVector, GLuint textureID){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
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
	//GLfloat quad[] = { y - 0.5f * height, x - 0.5f * width, y + 0.5f * height, x - 0.5f * width, y + 0.5f * height, x + 0.5f * width, y - 0.5f * height, x + 0.5f * width };
	//GLfloat quadUVs[] = { u, v, u, v - spriteHeight, u + spriteWidth, v - spriteHeight, u + spriteWidth, v };
	//GLfloat quadUVs[] = { u, v, u, v + spriteHeight, u + spriteWidth, v + spriteHeight, u + spriteWidth, v };
	GLfloat quadUVs[] = { u + spriteWidth, v + spriteHeight, u + spriteWidth, v, u, v, u, v + spriteHeight };
	drawTexture(x, y, quad, quadUVs, spriteTexture);
}

void drawSpriteGlow(float x, float y, GLuint spriteTexture, int index, int spriteCountX, int spriteCountY) {
	//width and height = tile_size
	float width = 0.0625f;
	float height = 0.0625f;
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0f / (float)spriteCountX;
	float spriteHeight = 1.0f / (float)spriteCountY;
	GLfloat quad[] = { x + 0.5f * width, y - 0.5f * height, x + 0.5f * width, y + 0.5f * height, x - 0.5f * width, y + 0.5f * height, x - 0.5f * width, y - 0.5f * height };
	//GLfloat quad[] = { y - 0.5f * height, x - 0.5f * width, y + 0.5f * height, x - 0.5f * width, y + 0.5f * height, x + 0.5f * width, y - 0.5f * height, x + 0.5f * width };
	//GLfloat quadUVs[] = { u, v, u, v - spriteHeight, u + spriteWidth, v - spriteHeight, u + spriteWidth, v };
	//GLfloat quadUVs[] = { u, v, u, v + spriteHeight, u + spriteWidth, v + spriteHeight, u + spriteWidth, v };
	GLfloat quadUVs[] = { u + spriteWidth, v + spriteHeight, u + spriteWidth, v, u, v, u, v + spriteHeight };
	drawTextureGlow(x, y, quad, quadUVs, spriteTexture);
	glDisable(GL_BLEND);
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
	if (type == "gem") {
		drawSprite(xPos, yPos, spriteTexture, 671, 64, 32);
	}
	else if (type == "enemy") {
		drawSprite(xPos, yPos, spriteTexture, 426, 64, 32);
	}
}

void Snake::render(GLuint& spriteTexture){
	for (size_t i = 0; i < body.size(); i++){
		if (body[i]->type == "p1up"){
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 36, 12, 8);
		}
		else if (body[i]->type == "p1down") {
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 0, 12, 8);
		}
		else if (body[i]->type == "p1left") {
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 12, 12, 8);
		}
		else if (body[i]->type == "p1right") {
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 24, 12, 8);
		}
		else if (body[i]->type == "p2up") {
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 42, 12, 8);
		}
		else if (body[i]->type == "p2down") {
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 6, 12, 8);
		}
		else if (body[i]->type == "p2left") {
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 18, 12, 8);
		}
		else if (body[i]->type == "p2right") {
			drawSpriteGlow(body[i]->xPos, body[i]->yPos, spriteTexture, 30, 12, 8);
		}


		else if (body[i]->type == "p1tail"){
			drawSprite(body[i]->xPos, body[i]->yPos, spriteTexture, 45, 12, 8);
		}
		else if (body[i]->type == "p2tail"){
			drawSprite(body[i]->xPos, body[i]->yPos, spriteTexture, 90, 12, 8);
		}
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

void DrawTextGlow(int fontTexture, string text, float size, float spacing, float r, float g, float b, float a) {
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
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


void placeEntity(string& type, float& x, float& y){
	if (type == "p1right") {
		Entity* entity = new Entity(0.0625f, 0.0625f);
		entity->xPos = -1.2f;
		entity->yPos = 0.0f;
		entity->velocity_x = 1.0f;
		entity->velocity_y = 1.0f;
		entity->type = "p1right";
		entity->direction = "right";
		entity->next = false;
		snake1vect.push_back(entity);
	}
	else if (type == "p2left") {
		Entity* entity = new Entity(0.0625f, 0.0625f);
		entity->xPos = 1.2f;
		entity->yPos = 0.0f;
		entity->velocity_x = 1.0f;
		entity->velocity_y = 1.0f;
		entity->type = "p2left";
		entity->direction = "left";
		entity->next = false;
		snake2vect.push_back(entity);
	}
	else if (type == "gem"){
		Entity* entity = new Entity(0.0625f, 0.0625f);
		entity->xPos = 0.01f * random1To120() * random1();
		entity->yPos = 0.01f * random1To90() * random1();
		entity->type = "gem";
		gems.push_back(entity);

		for (size_t i = 1; i < 1000; i++){
			Entity* entity = new Entity(0.0625f, 0.0625f);
			entity->xPos = 10.0f;
			entity->yPos = 10.0f;
			entity->type = "gem";
			gems.push_back(entity);
		}
	}

	else if (type == "enemy"){
		for (size_t i = 0; i < 1000; i++){
			Entity* entity = new Entity(0.0625f, 0.0625f);
			entity->xPos = 10.0f;
			entity->yPos = 10.0f;
			entity->type = "enemy";
			enemies.push_back(entity);
		}
	}


	Snake* snake1 = new Snake();
	Snake* snake2 = new Snake();
	snake1->body = snake1vect;
	snake2->body = snake2vect;
	snake1->crashed = false;
	snake2->crashed = false;
	player1 = snake1;
	player2 = snake2;

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
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 750, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, 1000, 750);
	//glOrtho(-2.66, 2.66, -2.0, 2.0, -2.0, 2.0);
	glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	textSheet = LoadTexture("final/font2.png");
	mapSheet = LoadTexture("final/sheet_4.png");
	charSheet = LoadTexture("final/characters_1.png");
	etcSheet = LoadTexture("final/tux_bros.png");

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	int oneTo8 = random1To8Music();
	switch (oneTo8) {
		case 1:
			music = Mix_LoadMUS("music2.mp3");
			break;
		case 2:
			music = Mix_LoadMUS("music3.mp3");
			break;
		case 3:
			music = Mix_LoadMUS("music5.mp3");
			break;
		case 4:
			music = Mix_LoadMUS("music6.mp3");
			break;
		case 5:
			music = Mix_LoadMUS("music7.mp3");
			break;
		case 6:
			music = Mix_LoadMUS("music8.mp3");
			break;
		case 7:
			music = Mix_LoadMUS("music12.mp3");
			break;
		case 8:
			music = Mix_LoadMUS("music13.mp3");
			break;
	}
	pressSpaceSound = Mix_LoadWAV("spaceSound.wav");
	randomSound = Mix_LoadWAV("randomSound.wav");
	bombSound = Mix_LoadWAV("bombSound.wav");
	crashSound = Mix_LoadWAV("crashSound.wav");
	collectSound = Mix_LoadWAV("collectSound.wav");
	beforecrashSound = Mix_LoadWAV("beforecrashSound.wav");


	Mix_Volume(1, 50); //spacesound
	Mix_Volume(2, 50); //randomsound
	Mix_Volume(3, 100); //bombsound
	Mix_Volume(4, 50); //crashsound
	Mix_Volume(5, 50); //collectsound
	Mix_Volume(6, 100); //beforecrashsound
	Mix_VolumeMusic(20);
	Mix_PlayMusic(music, -1);
	

	ifstream infile("final.txt");
	if (!(readHeader(infile))) { done = true; }
	if (!(readLayer(infile))) { done = true; }
}

float easeIn(float from, float to, float time) {
	float tVal = time*time*time*time*time;
	return (1.0f - tVal)*from + tVal*to;
}

float easeOut(float from, float to, float time) {
	float oneMinusT = 1.0f - time;
	float tVal = 1.0f - (oneMinusT * oneMinusT * oneMinusT *
		oneMinusT * oneMinusT);
	return (1.0f - tVal)*from + tVal*to;
}

float animationTime = 5.0f;
float animationStart1 = 10.0f;
float animationStart2 = 20.0f;
float animationStart3 = 30.0f;
float animationStart4 = 40.0f;
float animationEnd = 0.0f;
float animationValue1, animationValue2, animationValue3, animationValue4;

void updateMenu(float elapsed){
	SDL_Event event;

	animationTime = animationTime + elapsed;
	animationValue1 = mapValue(animationTime, animationStart1,
		animationEnd, 0.0, 1.0);
	animationTime = animationTime + elapsed;
	animationValue2 = mapValue(animationTime, animationStart2,
		animationEnd, 0.0, 1.0);
	animationTime = animationTime + elapsed;
	animationValue3 = mapValue(animationTime, animationStart3,
		animationEnd, 0.0, 1.0);
	animationTime = animationTime + elapsed;
	animationValue4 = mapValue(animationTime, animationStart4,
		animationEnd, 0.0, 1.0);

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			Mix_FreeMusic(music);
			Mix_FreeChunk(randomSound);
			Mix_FreeChunk(pressSpaceSound);
			Mix_FreeChunk(bombSound);
			Mix_FreeChunk(crashSound);
			Mix_FreeChunk(collectSound);
			Mix_FreeChunk(beforecrashSound);
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				ifstream infile("final.txt");
				if (!(readEntity(infile))) { done = true; }
				if (menuOption == 0){
					wasState1 = true;
					gamestate = 1;
					Mix_PlayChannel(1, pressSpaceSound, 0);
				}
				else if (menuOption == 1){
					wasState1 = false;
					gamestate = 2;
					Mix_PlayChannel(1, pressSpaceSound, 0);
				}
				else if (menuOption == 2){
					wasState1 = false;
					gamestate = 3;
					Mix_PlayChannel(1, pressSpaceSound, 0);
				}
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
				gamestate = 4;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_UP || event.key.keysym.scancode == SDL_SCANCODE_W) {
				if (menuOption == 0){
					Mix_PlayChannel(2, randomSound, 0);
					menuOption = 2;
				}
				else if (menuOption == 1){
					Mix_PlayChannel(2, randomSound, 0);
					menuOption = 0;
				}
				else if (menuOption == 2){
					Mix_PlayChannel(2, randomSound, 0);
					menuOption = 1;
				}
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_DOWN || event.key.keysym.scancode == SDL_SCANCODE_S) {
				if (menuOption == 0){
					Mix_PlayChannel(2, randomSound, 0);
					menuOption = 1;
				}
				else if (menuOption == 1){
					Mix_PlayChannel(2, randomSound, 0);
					menuOption = 2;
				}
				else if (menuOption == 2){
					Mix_PlayChannel(2, randomSound, 0);
					menuOption = 0;
				}
			}
		}
	}
}

void updateGameLevel1(float elapsed){
	SDL_Event event;

	//perlinValue += elapsed;
	if (shakeTimer > 0.0f && screenShaking){
		screenShakeValue += elapsed;
		shakeTimer = shakeTimer--;
	}
	else{
		screenShakeValue = 0.0f;
		screenShakeSpeed = 0.0f;
		screenShakeIntensity = 0.0f;
		screenShaking = false;
	}

	gameTimer = gameTimer - elapsed;
	if (gameTimer < 0.0f && p2score > p1score){
		gameTimer = 0.0f;
		winLose = "p2win";
		gamestate = 4;
	}
	if (gameTimer < 0.0f && p1score > p2score){
		gameTimer = 0.0f;
		winLose = "p1win";
		gamestate = 4;
	}
	if (gameTimer < 0.0f && p1score == p2score){
		gameTimer = 0.0f;
		winLose = "neither";
		gamestate = 4;
	}

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			Mix_FreeMusic(music);
			Mix_FreeChunk(randomSound);
			Mix_FreeChunk(pressSpaceSound);
			Mix_FreeChunk(bombSound);
			Mix_FreeChunk(crashSound);
			Mix_FreeChunk(collectSound);
			Mix_FreeChunk(beforecrashSound);
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
			winLose = "neither";
			gamestate = 4;
		}

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_UP] && player2->body[0]->type != "p2down" && player2->body[0]->type != "p2up") {
			player2->body[0]->type = "p2up";
			player2->body[0]->direction = "up";
		}
		else if (keys[SDL_SCANCODE_DOWN] && player2->body[0]->type != "p2up" && player2->body[0]->type != "p2down") {
			player2->body[0]->type = "p2down";
			player2->body[0]->direction = "down";
		}
		else if (keys[SDL_SCANCODE_LEFT] && player2->body[0]->type != "p2right" && player2->body[0]->type != "p2left") {
			player2->body[0]->type = "p2left";
			player2->body[0]->direction = "left";
		}
		else if (keys[SDL_SCANCODE_RIGHT] && player2->body[0]->type != "p2left" && player2->body[0]->type != "p2right") {
			player2->body[0]->type = "p2right";
			player2->body[0]->direction = "right";
		}
		else if (keys[SDL_SCANCODE_W] && player1->body[0]->type != "p1down" && player1->body[0]->type != "p1up") {
			player1->body[0]->type = "p1up";
			player1->body[0]->direction = "up";
		}
		else if (keys[SDL_SCANCODE_S] && player1->body[0]->type != "p1up" && player1->body[0]->type != "p1down") {
			player1->body[0]->type = "p1down";
			player1->body[0]->direction = "down";
		}
		else if (keys[SDL_SCANCODE_A] && player1->body[0]->type != "p1right" && player1->body[0]->type != "p1left") {
			player1->body[0]->type = "p1left";
			player1->body[0]->direction = "left";
		}
		else if (keys[SDL_SCANCODE_D] && player1->body[0]->type != "p1left" && player1->body[0]->type != "p1right") {
			player1->body[0]->type = "p1right";
			player1->body[0]->direction = "right";
		}
	}

	//collision into other player
	for (size_t i = 0; i < player2->body.size(); i++){
		if (isColliding((player1->body[0]), player2->body[i]) && !player1->crashed && !player2->crashed){
			if (i == 0){
				if (p1score > p2score){
					winLose = "p1win";
					player2->crashed = true;
				}
				else if (p2score > p1score){
					winLose = "p2win";
					player1->crashed = true;
				}
				else if (p1score == p2score){
					winLose = "neither";
					gamestate = 4;
				}
			}
			else player1->crashed = true;
		}
	}

	for (size_t i = 0; i < player1->body.size(); i++){
		if (isColliding((player2->body[0]), player1->body[i]) && !player1->crashed && !player2->crashed){
			if (i == 0){
				if (p1score > p2score){
					winLose = "p1win";
					player2->crashed = true;
				}
				else if (p2score > p1score){
					winLose = "p2win";
					player1->crashed = true;
				}
				else if (p1score == p2score){
					winLose = "neither";
					gamestate = 4;
				}
			}
			else player2->crashed = true;
		}
	}
	
	//collecting gems
	for (size_t i = 0; i < gems.size(); i++){
		if (isColliding((player1->body[0]), gems[i]) && !player1->crashed && !player2->crashed){

			Mix_PlayChannel(5, collectSound, 0);
			p1score += random1To3();
			
			gems[currentGem]->xPos = 10.0f;
			gems[currentGem]->yPos = 10.0f;
			currentGem++;
			gems[currentGem]->xPos = 0.01f * random1To120() * random1();
			gems[currentGem]->yPos = 0.01f * random1To90() * random1();

			//creating a new entity after collecting gem
			player1->body[p1current]->next = true;
			Entity* entity = new Entity(0.0625f, 0.0625f);
			if (player1->body[p1current]->direction == "up"){
				entity->xPos = player1->body[p1current]->xPos;
				entity->yPos = player1->body[p1current]->yPos - 0.0825f;
			}
			else if (player1->body[p1current]->direction == "down"){
				entity->xPos = player1->body[p1current]->xPos;
				entity->yPos = player1->body[p1current]->yPos + 0.0825f;
			}
			else if (player1->body[p1current]->direction == "left"){
				entity->xPos = player1->body[p1current]->xPos + 0.0825f;
				entity->yPos = player1->body[p1current]->yPos;
			}
			else if (player1->body[p1current]->direction == "right"){
				entity->xPos = player1->body[p1current]->xPos - 0.0825f;
				entity->yPos = player1->body[p1current]->yPos;
			}
			entity->type = "p1tail";
			entity->direction = player1->body[p1current]->direction;
			entity->velocity_x = player1->body[0]->velocity_x;
			entity->velocity_y = player1->body[0]->velocity_y;
			entity->next = false;
			player1->body.push_back(entity);
			p1current++;
		}
	}

	for (size_t i = 0; i < gems.size(); i++){
		if (isColliding((player2->body[0]), gems[i]) && !player1->crashed && !player2->crashed){

			Mix_PlayChannel(5, collectSound, 0);
			p2score += random1To3();
			
			gems[currentGem]->xPos = 10.0f;
			gems[currentGem]->yPos = 10.0f;
			currentGem++;
			gems[currentGem]->xPos = 0.01f * random1To120() * random1();
			gems[currentGem]->yPos = 0.01f * random1To90() * random1();

			//creating a new entity after collecting gem
			player2->body[p2current]->next = true;
			Entity* entity = new Entity(0.0625f, 0.0625f);
			if (player2->body[p2current]->direction == "up"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			else if (player2->body[p2current]->direction == "down"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			else if (player2->body[p2current]->direction == "left"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			else if (player2->body[p2current]->direction == "right"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			entity->type = "p2tail";
			entity->direction = player2->body[p2current]->direction;
			entity->velocity_x = player2->body[0]->velocity_x;
			entity->velocity_y = player2->body[0]->velocity_y;
			entity->next = false;
			player2->body.push_back(entity);
			p2current++;
		}
	}

	//collision into walls
	if (player1->body[0]->xPos > 1.33f || player1->body[0]->xPos < -1.33f || player1->body[0]->yPos > 1.0f || player1->body[0]->yPos < -1.0f && !player1->crashed && !player2->crashed){
		glMatrixMode(GL_MODELVIEW);

		if (!screenShaking){
			shakeTimer = 10.0f;
			screenShakeSpeed = 100.0f;
			screenShakeIntensity = 0.03f;
		}
		screenShaking = true;
		if (player1->body[0]->xPos > 1.33f){
			player1->body[0]->xPos = 1.33f;
		}
		else if (player1->body[0]->xPos < -1.33f){
			player1->body[0]->xPos = -1.33f;
		}
		else if (player1->body[0]->yPos > 1.0f){
			player1->body[0]->yPos = 1.0f;
		}
		else if (player1->body[0]->yPos < -1.0f){
			player1->body[0]->yPos = -1.0f;
		}
		Mix_PlayChannel(6, beforecrashSound, 0);
		Mix_PlayChannel(4, crashSound, 0);
		player1->crashed = true;
	}
	else if (player2->body[0]->xPos > 1.33f || player2->body[0]->xPos < -1.33f || player2->body[0]->yPos > 1.0f || player2->body[0]->yPos < -1.0f && !player1->crashed && !player2->crashed){
		glMatrixMode(GL_MODELVIEW);

		if (!screenShaking){
			shakeTimer = 10.0f;
			screenShakeSpeed = 100.0f;
			screenShakeIntensity = 0.03f;
		}
		screenShaking = true;
		if (player2->body[0]->xPos > 1.33f){
			player2->body[0]->xPos = 1.33f;
		}
		else if (player2->body[0]->xPos < -1.33f){
			player2->body[0]->xPos = -1.33f;
		}
		else if (player2->body[0]->yPos > 1.0f){
			player2->body[0]->yPos = 1.0f;
		}
		else if (player2->body[0]->yPos < -1.0f){
			player2->body[0]->yPos = -1.0f;
		}
		Mix_PlayChannel(6, beforecrashSound, 0);
		Mix_PlayChannel(4, crashSound, 0);
		player2->crashed = true;	
	}
	if (player1->crashed && crashTimer < 0.0f){
		player1->body[0]->velocity_x = 0.0f;
		winLose = "p2win";
		gamestate = 4;
	}
	else if (player2->crashed && crashTimer < 0.0f){
		player2->body[0]->velocity_x = 0.0f;
		winLose = "p1win";
		gamestate = 4;
	}
	if (player1->crashed || player2->crashed){
		crashTimer = crashTimer - elapsed;
	}
}

void updateGameLevel2(float elapsed){
	SDL_Event event;

	//perlinValue += elapsed;
	if (shakeTimer > 0.0f && screenShaking){
		screenShakeValue += elapsed;
		shakeTimer = shakeTimer--;
	}
	else{
		screenShakeValue = 0.0f;
		screenShakeSpeed = 0.0f;
		screenShakeIntensity = 0.0f;
		screenShaking = false;
	}

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			Mix_FreeMusic(music);
			Mix_FreeChunk(randomSound);
			Mix_FreeChunk(pressSpaceSound);
			Mix_FreeChunk(bombSound);
			Mix_FreeChunk(crashSound);
			Mix_FreeChunk(collectSound);
			Mix_FreeChunk(beforecrashSound);
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
			winLose = "neither";
			gamestate = 4;
		}

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_UP] && player2->body[0]->type != "p2down") {
			player2->body[0]->type = "p2up";
			player2->body[0]->direction = "up";
		}
		else if (keys[SDL_SCANCODE_DOWN] && player2->body[0]->type != "p2up") {
			player2->body[0]->type = "p2down";
			player2->body[0]->direction = "down";
		}
		else if (keys[SDL_SCANCODE_LEFT] && player2->body[0]->type != "p2right") {
			player2->body[0]->type = "p2left";
			player2->body[0]->direction = "left";
		}
		else if (keys[SDL_SCANCODE_RIGHT] && player2->body[0]->type != "p2left") {
			player2->body[0]->type = "p2right";
			player2->body[0]->direction = "right";
		}
		else if (keys[SDL_SCANCODE_W] && player1->body[0]->type != "p1down") {
			player1->body[0]->type = "p1up";
			player1->body[0]->direction = "up";
		}
		else if (keys[SDL_SCANCODE_S] && player1->body[0]->type != "p1up") {
			player1->body[0]->type = "p1down";
			player1->body[0]->direction = "down";
		}
		else if (keys[SDL_SCANCODE_A] && player1->body[0]->type != "p1right") {
			player1->body[0]->type = "p1left";
			player1->body[0]->direction = "left";
		}
		else if (keys[SDL_SCANCODE_D] && player1->body[0]->type != "p1left") {
			player1->body[0]->type = "p1right";
			player1->body[0]->direction = "right";
		}
	}

	//collision into other player
	for (size_t i = 0; i < player2->body.size(); i++){
		if (isColliding((player1->body[0]), player2->body[i]) && !player1->crashed && !player2->crashed){
			if (i == 0){
				if (p1score > p2score){
					winLose = "p1win";
					player2->crashed = true;
				}
				else if (p2score > p1score){
					winLose = "p2win";
					player1->crashed = true;
				}
				else if (p1score == p2score){
					winLose = "neither";
					gamestate = 4;
				}
			}
			else player1->crashed = true;
		}
	}

	for (size_t i = 0; i < player1->body.size(); i++){
		if (isColliding((player2->body[0]), player1->body[i]) && !player1->crashed && !player2->crashed){
			if (i == 0){
				if (p1score > p2score){
					winLose = "p1win";
					player2->crashed = true;
				}
				else if (p2score > p1score){
					winLose = "p2win";
					player1->crashed = true;
				}
				else if (p1score == p2score){
					winLose = "neither";
					gamestate = 4;
				}
			}
			else player2->crashed = true;
		}
	}

	//collecting gems
	for (size_t i = 0; i < gems.size(); i++){
		if (isColliding((player1->body[0]), gems[i]) && !player1->crashed && !player2->crashed){
			Mix_PlayChannel(5, collectSound, 0);
			p1score += random1To3();

			enemies[currentEnemy]->xPos = 0.01f * random1To120() * random1();
			enemies[currentEnemy]->yPos = 0.01f * random1To90() * random1();
			currentEnemy++;

			gems[currentGem]->xPos = 10.0f;
			gems[currentGem]->yPos = 10.0f;
			currentGem++;
			gems[currentGem]->xPos = 0.01f * random1To120() * random1();
			gems[currentGem]->yPos = 0.01f * random1To90() * random1();


		}
	}

	for (size_t i = 0; i < gems.size(); i++){
		if (isColliding((player2->body[0]), gems[i]) && !player1->crashed && !player2->crashed){

			Mix_PlayChannel(5, collectSound, 0);
			p2score += random1To3();

			enemies[currentEnemy]->xPos = 0.01f * random1To120() * random1();
			enemies[currentEnemy]->yPos = 0.01f * random1To90() * random1();
			currentEnemy++;

			gems[currentGem]->xPos = 10.0f;
			gems[currentGem]->yPos = 10.0f;
			currentGem++;
			gems[currentGem]->xPos = 0.01f * random1To120() * random1();
			gems[currentGem]->yPos = 0.01f * random1To90() * random1();
		}
	}

	//collision into enemies
	for (size_t i = 0; i < enemies.size(); i++){
		if (isColliding((player2->body[0]), enemies[i]) && !player1->crashed && !player2->crashed){
			Mix_PlayChannel(3, bombSound, 0);
			player2->crashed = true;
		}
	}
	for (size_t i = 0; i < enemies.size(); i++){
		if (isColliding((player1->body[0]), enemies[i]) && !player1->crashed && !player2->crashed){
			Mix_PlayChannel(3, bombSound, 0);
			player1->crashed = true;
		}
	}

	//collision into walls
	if (player1->body[0]->xPos > 1.33f || player1->body[0]->xPos < -1.33f || player1->body[0]->yPos > 1.0f || player1->body[0]->yPos < -1.0f && !player1->crashed && !player2->crashed){
		glMatrixMode(GL_MODELVIEW);

		if (!screenShaking){
			shakeTimer = 10.0f;
			screenShakeSpeed = 100.0f;
			screenShakeIntensity = 0.03f;
		}
		screenShaking = true;
		if (player1->body[0]->xPos > 1.33f){
			player1->body[0]->xPos = 1.33f;
		}
		else if (player1->body[0]->xPos < -1.33f){
			player1->body[0]->xPos = -1.33f;
		}
		else if (player1->body[0]->yPos > 1.0f){
			player1->body[0]->yPos = 1.0f;
		}
		else if (player1->body[0]->yPos < -1.0f){
			player1->body[0]->yPos = -1.0f;
		}
		Mix_PlayChannel(6, beforecrashSound, 0);
		Mix_PlayChannel(4, crashSound, 0);
		player1->crashed = true;
	}
	else if (player2->body[0]->xPos > 1.33f || player2->body[0]->xPos < -1.33f || player2->body[0]->yPos > 1.0f || player2->body[0]->yPos < -1.0f && !player1->crashed && !player2->crashed){
		glMatrixMode(GL_MODELVIEW);

		if (!screenShaking){
			shakeTimer = 10.0f;
			screenShakeSpeed = 100.0f;
			screenShakeIntensity = 0.03f;
		}
		screenShaking = true;
		if (player2->body[0]->xPos > 1.33f){
			player2->body[0]->xPos = 1.33f;
		}
		else if (player2->body[0]->xPos < -1.33f){
			player2->body[0]->xPos = -1.33f;
		}
		else if (player2->body[0]->yPos > 1.0f){
			player2->body[0]->yPos = 1.0f;
		}
		else if (player2->body[0]->yPos < -1.0f){
			player2->body[0]->yPos = -1.0f;
		}
		Mix_PlayChannel(6, beforecrashSound, 0);
		Mix_PlayChannel(4, crashSound, 0);
		player2->crashed = true;
	}
	if (player1->crashed && crashTimer < 0.0f){
		player1->body[0]->velocity_x = 0.0f;
		winLose = "p2win";
		gamestate = 4;
	}
	else if (player2->crashed && crashTimer < 0.0f){
		player2->body[0]->velocity_x = 0.0f;
		winLose = "p1win";
		gamestate = 4;
	}
	if (player1->crashed || player2->crashed){
		crashTimer = crashTimer - elapsed;
	}
}

void updateGameLevel3(float elapsed){
	SDL_Event event;

	//perlinValue += elapsed;
	if (shakeTimer > 0.0f && screenShaking){
		screenShakeValue += elapsed;
		shakeTimer = shakeTimer--;
	}
	else{
		screenShakeValue = 0.0f;
		screenShakeSpeed = 0.0f;
		screenShakeIntensity = 0.0f;
		screenShaking = false;
	}

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			Mix_FreeMusic(music);
			Mix_FreeChunk(randomSound);
			Mix_FreeChunk(pressSpaceSound);
			Mix_FreeChunk(bombSound);
			Mix_FreeChunk(crashSound);
			Mix_FreeChunk(collectSound);
			Mix_FreeChunk(beforecrashSound);
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
			winLose = "neither";
			gamestate = 4;
		}

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_UP] && player2->body[0]->type != "p2down" && player2->body[0]->type != "p2up") {
			player2->body[0]->type = "p2up";
			player2->body[0]->direction = "up";
		}
		else if (keys[SDL_SCANCODE_DOWN] && player2->body[0]->type != "p2up" && player2->body[0]->type != "p2down") {
			player2->body[0]->type = "p2down";
			player2->body[0]->direction = "down";
		}
		else if (keys[SDL_SCANCODE_LEFT] && player2->body[0]->type != "p2right" && player2->body[0]->type != "p2left") {
			player2->body[0]->type = "p2left";
			player2->body[0]->direction = "left";
		}
		else if (keys[SDL_SCANCODE_RIGHT] && player2->body[0]->type != "p2left" && player2->body[0]->type != "p2right") {
			player2->body[0]->type = "p2right";
			player2->body[0]->direction = "right";
		}
		else if (keys[SDL_SCANCODE_W] && player1->body[0]->type != "p1down" && player1->body[0]->type != "p1up") {
			player1->body[0]->type = "p1up";
			player1->body[0]->direction = "up";
		}
		else if (keys[SDL_SCANCODE_S] && player1->body[0]->type != "p1up" && player1->body[0]->type != "p1down") {
			player1->body[0]->type = "p1down";
			player1->body[0]->direction = "down";
		}
		else if (keys[SDL_SCANCODE_A] && player1->body[0]->type != "p1right" && player1->body[0]->type != "p1left") {
			player1->body[0]->type = "p1left";
			player1->body[0]->direction = "left";
		}
		else if (keys[SDL_SCANCODE_D] && player1->body[0]->type != "p1left" && player1->body[0]->type != "p1right") {
			player1->body[0]->type = "p1right";
			player1->body[0]->direction = "right";
		}
	}

	//collision into other player
	for (size_t i = 0; i < player2->body.size(); i++){
		if (isColliding((player1->body[0]), player2->body[i]) && !player1->crashed && !player2->crashed){
			if (i == 0){
				if (p1score > p2score){
					winLose = "p1win";
					player2->crashed = true;
				}
				else if (p2score > p1score){
					winLose = "p2win";
					player1->crashed = true;
				}
				else if (p1score == p2score){
					winLose = "neither";
					gamestate = 4;
				}
			}
			else player1->crashed = true;
		}
	}

	for (size_t i = 0; i < player1->body.size(); i++){
		if (isColliding((player2->body[0]), player1->body[i]) && !player1->crashed && !player2->crashed){
			if (i == 0){
				if (p1score > p2score){
					winLose = "p1win";
					player2->crashed = true;
				}
				else if (p2score > p1score){
					winLose = "p2win";
					player1->crashed = true;
				}
				else if (p1score == p2score){
					winLose = "neither";
					gamestate = 4;
				}
			}
			else player2->crashed = true;
		}
	}

	//collecting gems
	for (size_t i = 0; i < gems.size(); i++){
		if (isColliding((player1->body[0]), gems[i]) && !player1->crashed && !player2->crashed){

			Mix_PlayChannel(5, collectSound, 0);
			p1score += random1To3();

			enemies[currentEnemy]->xPos = 0.01f * random1To120() * random1();
			enemies[currentEnemy]->yPos = 0.01f * random1To90() * random1();
			currentEnemy++;

			gems[currentGem]->xPos = 10.0f;
			gems[currentGem]->yPos = 10.0f;
			currentGem++;
			gems[currentGem]->xPos = 0.01f * random1To120() * random1();
			gems[currentGem]->yPos = 0.01f * random1To90() * random1();

			//creating a new entity after collecting gem
			player1->body[p1current]->next = true;
			Entity* entity = new Entity(0.0625f, 0.0625f);
			if (player1->body[p1current]->direction == "up"){
				entity->xPos = player1->body[p1current]->xPos;
				entity->yPos = player1->body[p1current]->yPos - 0.0825f;
			}
			else if (player1->body[p1current]->direction == "down"){
				entity->xPos = player1->body[p1current]->xPos;
				entity->yPos = player1->body[p1current]->yPos + 0.0825f;
			}
			else if (player1->body[p1current]->direction == "left"){
				entity->xPos = player1->body[p1current]->xPos + 0.0825f;
				entity->yPos = player1->body[p1current]->yPos;
			}
			else if (player1->body[p1current]->direction == "right"){
				entity->xPos = player1->body[p1current]->xPos - 0.0825f;
				entity->yPos = player1->body[p1current]->yPos;
			}
			entity->type = "p1tail";
			entity->direction = player1->body[p1current]->direction;
			entity->velocity_x = player1->body[0]->velocity_x;
			entity->velocity_y = player1->body[0]->velocity_y;
			entity->next = false;
			player1->body.push_back(entity);
			p1current++;
		}
	}

	for (size_t i = 0; i < gems.size(); i++){
		if (isColliding((player2->body[0]), gems[i]) && !player1->crashed && !player2->crashed){

			Mix_PlayChannel(5, collectSound, 0);
			p2score += random1To3();

			enemies[currentEnemy]->xPos = 0.01f * random1To120() * random1();
			enemies[currentEnemy]->yPos = 0.01f * random1To90() * random1();
			currentEnemy++;

			gems[currentGem]->xPos = 10.0f;
			gems[currentGem]->yPos = 10.0f;
			currentGem++;
			gems[currentGem]->xPos = 0.01f * random1To120() * random1();
			gems[currentGem]->yPos = 0.01f * random1To90() * random1();

			//creating a new entity after collecting gem
			player2->body[p2current]->next = true;
			Entity* entity = new Entity(0.0625f, 0.0625f);
			if (player2->body[p2current]->direction == "up"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			else if (player2->body[p2current]->direction == "down"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			else if (player2->body[p2current]->direction == "left"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			else if (player2->body[p2current]->direction == "right"){
				entity->xPos = player2->body[p2current]->xPos;
				entity->yPos = player2->body[p2current]->yPos;
			}
			entity->type = "p2tail";
			entity->direction = player2->body[p2current]->direction;
			entity->velocity_x = player2->body[0]->velocity_x;
			entity->velocity_y = player2->body[0]->velocity_y;
			entity->next = false;
			player2->body.push_back(entity);
			p2current++;
		}
	}

	// collision into enemies
	for (size_t i = 0; i < enemies.size(); i++){
		if (isColliding((player2->body[0]), enemies[i]) && !player1->crashed && !player2->crashed){
			Mix_PlayChannel(3, bombSound, 0);
			player2->crashed = true;
		}
	}
	for (size_t i = 0; i < enemies.size(); i++){
		if (isColliding((player1->body[0]), enemies[i]) && !player1->crashed && !player2->crashed){
			Mix_PlayChannel(3, bombSound, 0);
			player1->crashed = true;
		}
	}

	//collision into walls
	if (player1->body[0]->xPos > 1.33f || player1->body[0]->xPos < -1.33f || player1->body[0]->yPos > 1.0f || player1->body[0]->yPos < -1.0f && !player1->crashed && !player2->crashed){
		glMatrixMode(GL_MODELVIEW);

		if (!screenShaking){
			shakeTimer = 10.0f;
			screenShakeSpeed = 100.0f;
			screenShakeIntensity = 0.03f;
		}
		screenShaking = true;
		if (player1->body[0]->xPos > 1.33f){
			player1->body[0]->xPos = 1.33f;
		}
		else if (player1->body[0]->xPos < -1.33f){
			player1->body[0]->xPos = -1.33f;
		}
		else if (player1->body[0]->yPos > 1.0f){
			player1->body[0]->yPos = 1.0f;
		}
		else if (player1->body[0]->yPos < -1.0f){
			player1->body[0]->yPos = -1.0f;
		}
		Mix_PlayChannel(6, beforecrashSound, 0);
		Mix_PlayChannel(4, crashSound, 0);
		player1->crashed = true;
	}
	else if (player2->body[0]->xPos > 1.33f || player2->body[0]->xPos < -1.33f || player2->body[0]->yPos > 1.0f || player2->body[0]->yPos < -1.0f && !player1->crashed && !player2->crashed){
		glMatrixMode(GL_MODELVIEW);

		if (!screenShaking){
			shakeTimer = 10.0f;
			screenShakeSpeed = 100.0f;
			screenShakeIntensity = 0.03f;
		}
		screenShaking = true;
		if (player2->body[0]->xPos > 1.33f){
			player2->body[0]->xPos = 1.33f;
		}
		else if (player2->body[0]->xPos < -1.33f){
			player2->body[0]->xPos = -1.33f;
		}
		else if (player2->body[0]->yPos > 1.0f){
			player2->body[0]->yPos = 1.0f;
		}
		else if (player2->body[0]->yPos < -1.0f){
			player2->body[0]->yPos = -1.0f;
		}
		Mix_PlayChannel(6, beforecrashSound, 0);
		Mix_PlayChannel(4, crashSound, 0);
		player2->crashed = true;
	}
	if (player1->crashed && crashTimer < 0.0f){
		player1->body[0]->velocity_x = 0.0f;
		winLose = "p2win";
		gamestate = 4;
	}
	else if (player2->crashed && crashTimer < 0.0f){
		player2->body[0]->velocity_x = 0.0f;
		winLose = "p1win";
		gamestate = 4;
	}
	if (player1->crashed || player2->crashed){
		crashTimer = crashTimer - elapsed;
	}
}

void updateGameOver(float elapsed){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			Mix_FreeMusic(music);
			Mix_FreeChunk(randomSound);
			Mix_FreeChunk(pressSpaceSound);
			Mix_FreeChunk(bombSound);
			Mix_FreeChunk(crashSound);
			Mix_FreeChunk(collectSound);
			Mix_FreeChunk(beforecrashSound);
			done = true;
		}
	}
}

void fixedUpdate(){
	if (gamestate == 1 || gamestate == 2 || gamestate == 3){
		if (!player1->crashed && !player2->crashed){
			//comment out to test 1 player
			if (player1->body[0]->type == "p1up"){
			player1->body[0]->yPos += player1->body[0]->velocity_y * FIXED_TIMESTEP;
			}
			else if (player1->body[0]->type == "p1down"){
			player1->body[0]->yPos -= player1->body[0]->velocity_y * FIXED_TIMESTEP;
			}
			else if (player1->body[0]->type == "p1left"){
			player1->body[0]->xPos -= player1->body[0]->velocity_x * FIXED_TIMESTEP;
			}
			else if (player1->body[0]->type == "p1right"){
			player1->body[0]->xPos += player1->body[0]->velocity_x * FIXED_TIMESTEP;
			}
			
			//comment out to test 1 player
			/*
			if (player2->body[0]->type == "p2up"){
				player2->body[0]->yPos += player2->body[0]->velocity_y * FIXED_TIMESTEP;
			}
			else if (player2->body[0]->type == "p2down"){
				player2->body[0]->yPos -= player2->body[0]->velocity_y * FIXED_TIMESTEP;
			}
			else if (player2->body[0]->type == "p2left"){
				player2->body[0]->xPos -= player2->body[0]->velocity_x * FIXED_TIMESTEP;
			}
			else if (player2->body[0]->type == "p2right"){
				player2->body[0]->xPos += player2->body[0]->velocity_x * FIXED_TIMESTEP;
			}
			*/
			for (size_t i = 0; i < player1->body.size(); i++){
				if (player1->body[i]->next){
					if (player1->body[i + 1]->direction == "up"){
						player1->body[i + 1]->xPos = player1->body[i]->xPos;
						player1->body[i + 1]->yPos = player1->body[i]->yPos;
						player1->body[i + 1]->yPos -= 0.0625f;
						player1->body[i + 1]->turnTimer = player1->body[i + 1]->turnTimer - 0.3f;
						if (player1->body[i + 1]->turnTimer < 0.0f){
							player1->body[i + 1]->direction = player1->body[i]->direction;
							player1->body[i + 1]->turnTimer = 1.0f;
						}
					}
					else if (player1->body[i + 1]->direction == "down"){
						player1->body[i + 1]->xPos = player1->body[i]->xPos;
						player1->body[i + 1]->yPos = player1->body[i]->yPos;
						player1->body[i + 1]->yPos += 0.0625f;
						player1->body[i + 1]->turnTimer = player1->body[i + 1]->turnTimer - 0.3f;
						if (player1->body[i + 1]->turnTimer < 0.0f){
							player1->body[i + 1]->direction = player1->body[i]->direction;
							player1->body[i + 1]->turnTimer = 1.0f;
						}
					}
					else if (player1->body[i + 1]->direction == "left"){
						player1->body[i + 1]->xPos = player1->body[i]->xPos;
						player1->body[i + 1]->yPos = player1->body[i]->yPos;
						player1->body[i + 1]->xPos += 0.0625f;
						player1->body[i + 1]->turnTimer = player1->body[i + 1]->turnTimer - 0.3f;
						if (player1->body[i + 1]->turnTimer < 0.0f){
								player1->body[i + 1]->direction = player1->body[i]->direction;
								player1->body[i + 1]->turnTimer = 1.0f;
							}
						}
					else if (player1->body[i + 1]->direction == "right"){
						player1->body[i + 1]->xPos = player1->body[i]->xPos;
						player1->body[i + 1]->yPos = player1->body[i]->yPos;
						player1->body[i + 1]->xPos -= 0.0625f;
						player1->body[i + 1]->turnTimer = player1->body[i + 1]->turnTimer - 0.3f;
						if (player1->body[i + 1]->turnTimer < 0.0f){
							player1->body[i + 1]->direction = player1->body[i]->direction;
							player1->body[i + 1]->turnTimer = 1.0f;
						}
					}
				}
			}
			

			for (size_t i = 0; i < player2->body.size(); i++){
				if (player2->body[i]->next){
					if (player2->body[i + 1]->direction == "up"){
						player2->body[i + 1]->xPos = player2->body[i]->xPos;
						player2->body[i + 1]->yPos = player2->body[i]->yPos;
						player2->body[i + 1]->yPos -= 0.0625f;
						player2->body[i + 1]->turnTimer = player2->body[i + 1]->turnTimer - 0.3f;
						if (player2->body[i + 1]->turnTimer < 0.0f){
							player2->body[i + 1]->direction = player2->body[i]->direction;
							player2->body[i + 1]->turnTimer = 1.0f;
						}
					}
					else if (player2->body[i + 1]->direction == "down"){
						player2->body[i + 1]->xPos = player2->body[i]->xPos;
						player2->body[i + 1]->yPos = player2->body[i]->yPos;
						player2->body[i + 1]->yPos += 0.0625f;
						player2->body[i + 1]->turnTimer = player2->body[i + 1]->turnTimer - 0.3f;
						if (player2->body[i + 1]->turnTimer < 0.0f){
							player2->body[i + 1]->direction = player2->body[i]->direction;
							player2->body[i + 1]->turnTimer = 1.0f;
						}
					}
					else if (player2->body[i + 1]->direction == "left"){
						player2->body[i + 1]->xPos = player2->body[i]->xPos;
						player2->body[i + 1]->yPos = player2->body[i]->yPos;
						player2->body[i + 1]->xPos += 0.0625f;
						player2->body[i + 1]->turnTimer = player2->body[i + 1]->turnTimer - 0.3f;
						if (player2->body[i + 1]->turnTimer < 0.0f){
							player2->body[i + 1]->direction = player2->body[i]->direction;
							player2->body[i + 1]->turnTimer = 1.0f;
						}
					}
					else if (player2->body[i + 1]->direction == "right"){
						player2->body[i + 1]->xPos = player2->body[i]->xPos;
						player2->body[i + 1]->yPos = player2->body[i]->yPos;
						player2->body[i + 1]->xPos -= 0.0625f;
						player2->body[i + 1]->turnTimer = player2->body[i + 1]->turnTimer - 0.3f;
						if (player2->body[i + 1]->turnTimer < 0.0f){
							player2->body[i + 1]->direction = player2->body[i]->direction;
							player2->body[i + 1]->turnTimer = 1.0f;
						}
					}
				}
			}
		}
		
		if (player1->crashed){
		player1->body[0]->yPos -= player1->body[0]->velocity_y * FIXED_TIMESTEP;
		}
		else if (player2->crashed){
		player2->body[0]->yPos -= player2->body[0]->velocity_y * FIXED_TIMESTEP;
		}
	}
}		



void update(float elapsed) {
	switch (gamestate) {
	case 0:
		updateMenu(elapsed);
		break;
	case 1:
		updateGameLevel1(elapsed);
		break;
	case 2:
		updateGameLevel2(elapsed);
		break;
	case 3:
		updateGameLevel3(elapsed);
		break;
	case 4:
		updateGameOver(elapsed);
		break;

	}
}



void renderMenu() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.9f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, "S", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.85f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, "N", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.8f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, "A", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.75f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, "K", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.7f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, "E", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.65f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, "2", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.6f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, ".", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(easeIn(-1.2f, -3.5f, animationValue1), easeIn(0.55f, -5.5f, animationValue1), 0.0f);
	DrawText(textSheet, "0", 0.04f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	if (menuOption == 0){
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, -3.5f, animationValue1), easeIn(0.8f, -5.5f, animationValue1), 0.0f);
		DrawText(textSheet, "Timed 2P", 0.04f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, 3.5f, animationValue2), easeIn(0.6f, -4.5f, animationValue2), 0.0f);
		DrawText(textSheet, "1 v. 1 v. Enemy (Easy Mode)", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, -3.5f, animationValue3), easeIn(0.4f, -3.5f, animationValue3), 0.0f);
		DrawText(textSheet, "1 v. 1 v. Enemy (Normal Mode)", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(0.0f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Rules: ", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.1f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "1st Player: W/A/S/D || 2nd Player: UP/DOWN/LEFT/RIGHT", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.2f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Collect points. Each gem is 1 to 3 points.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.3f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Avoid walls. Beware the timer.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.4f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Don't hit the other player's tail.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.5f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Attack the player's head if you have higher score.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.7f, -3.5f, animationValue4), easeIn(-0.6f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Person with higher score wins.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.7f, -3.5f, animationValue4), easeIn(-0.8f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Press Esc to escape end game.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (menuOption == 1){
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, -3.5f, animationValue1), easeIn(0.8f, -5.5f, animationValue1), 0.0f);
		DrawText(textSheet, "Timed 2P", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, 3.5f, animationValue2), easeIn(0.6f, -4.5f, animationValue2), 0.0f);
		DrawText(textSheet, "1 v. 1 v. Enemy (Easy Mode)", 0.04f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, -3.5f, animationValue3), easeIn(0.4f, -3.5f, animationValue3), 0.0f);
		DrawText(textSheet, "1 v. 1 v. Enemy (Normal Mode)", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(0.0f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Rules: ", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.1f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "1st Player: W/A/S/D || 2nd Player: UP/DOWN/LEFT/RIGHT", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.2f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Collect points. Each gem is 1 to 3 points.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.3f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Avoid walls. Avoid enemies.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.4f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Attack the player's head if you have higher score.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.7f, -3.5f, animationValue4), easeIn(-0.6f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Person with higher score wins.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.7f, -3.5f, animationValue4), easeIn(-0.8f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Press Esc to escape end game.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (menuOption == 2){
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, -3.5f, animationValue1), easeIn(0.8f, -5.5f, animationValue1), 0.0f);
		DrawText(textSheet, "Timed 2P", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, 3.5f, animationValue2), easeIn(0.6f, -4.5f, animationValue2), 0.0f);
		DrawText(textSheet, "1 v. 1 v. Enemy (Easy Mode)", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.4f, -3.5f, animationValue3), easeIn(0.4f, -3.5f, animationValue3), 0.0f);
		DrawText(textSheet, "1 v. 1 v. Enemy (Normal Mode)", 0.04f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(0.0f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Rules: ", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.1f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "1st Player: W/A/S/D || 2nd Player: UP/DOWN/LEFT/RIGHT", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.2f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Collect points. Each gem is 1 to 3 points.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.3f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Avoid walls. Avoid enemies.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.4f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Don't hit the other player's tail.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-1.2f, -3.5f, animationValue4), easeIn(-0.5f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Attack the player's head if you have higher score.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.7f, -3.5f, animationValue4), easeIn(-0.6f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Person with higher score wins.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(easeIn(-0.7f, -3.5f, animationValue4), easeIn(-0.8f, -3.5f, animationValue4), 0.0f);
		DrawText(textSheet, "Press Esc to escape end game.", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}


void renderGameLevel1() {


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-TILE_SIZE* mapWidth / 2, TILE_SIZE* mapHeight / 2, 0.0f);

	glBindTexture(GL_TEXTURE_2D, mapSheet);
	glEnable(GL_TEXTURE_2D);
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_QUADS, 0, vertexData.size() / 2);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	glLoadIdentity();
	glTranslatef(-0.3f, 0.9f, 0.0f);
	DrawText(textSheet, "TIME: " + to_string(gameTimer), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	glLoadIdentity();

	if (player1->crashed || player2->crashed){
		glTranslatef(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	}

	player1->render(charSheet);
	player2->render(charSheet);

	for (size_t i = 0; i < gems.size(); i++) {
		gems[i]->render(etcSheet);
	}

	glLoadIdentity();
	glTranslatef(-1.3f, -0.95f, 0.0f);
	DrawText(textSheet, "P1: " + to_string(p1score), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(1.0f, -0.95f, 0.0f);
	DrawText(textSheet, "P2: " + to_string(p2score), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

}

void renderGameLevel2(){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-TILE_SIZE* mapWidth / 2, TILE_SIZE* mapHeight / 2, 0.0f);

	glBindTexture(GL_TEXTURE_2D, mapSheet);
	glEnable(GL_TEXTURE_2D);
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_QUADS, 0, vertexData.size() / 2);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glLoadIdentity();

	if (player1->crashed || player2->crashed){
		glTranslatef(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	}

	player1->render(charSheet);
	player2->render(charSheet);

	for (size_t i = 0; i < gems.size(); i++) {
		gems[i]->render(etcSheet);
	}

	for (size_t i = 0; i < enemies.size(); i++) {
		enemies[i]->render(etcSheet);
	}

	glLoadIdentity();
	glTranslatef(-1.3f, -0.95f, 0.0f);
	DrawText(textSheet, "P1: " + to_string(p1score), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(1.0f, -0.95f, 0.0f);
	DrawText(textSheet, "P2: " + to_string(p2score), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void renderGameLevel3(){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-TILE_SIZE* mapWidth / 2, TILE_SIZE* mapHeight / 2, 0.0f);

	glBindTexture(GL_TEXTURE_2D, mapSheet);
	glEnable(GL_TEXTURE_2D);
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_QUADS, 0, vertexData.size() / 2);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glLoadIdentity();

	if (player1->crashed || player2->crashed){
		glTranslatef(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	}


	player1->render(charSheet);
	player2->render(charSheet);

	for (size_t i = 0; i < gems.size(); i++) {
		gems[i]->render(etcSheet);
	}

	for (size_t i = 0; i < enemies.size(); i++) {
		enemies[i]->render(etcSheet);
	}

	glLoadIdentity();
	glTranslatef(-1.3f, -0.95f, 0.0f);
	DrawText(textSheet, "P1: " + to_string(p1score), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(1.0f, -0.95f, 0.0f);
	DrawText(textSheet, "P2: " + to_string(p2score), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

}

void renderGameOver() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (winLose == "p1win"){
		glTranslatef(-1.0f, 0.6f, 0.0f);
		DrawText(textSheet, "Winner: Player 1", 0.10f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		if (wasState1 == true){
			glLoadIdentity();
			glTranslatef(-1.0f, 0.3f, 0.0f);
			DrawText(textSheet, "TIME LEFT: " + to_string(gameTimer), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		glLoadIdentity();
		glTranslatef(-1.0f, 0.0f, 0.0f);
		DrawText(textSheet, "Press RETURN to Exit", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(-1.0f, -0.3f, 0.0f);
		DrawText(textSheet, "Player 1's Score: " + to_string(p1score), 0.08f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(-1.0f, -0.6f, 0.0f);
		DrawText(textSheet, "Player 2's Score: " + to_string(p2score), 0.08f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (winLose == "p2win"){
		glTranslatef(-1.0f, 0.6f, 0.0f);
		DrawText(textSheet, "Winner: Player 2", 0.10f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		if (wasState1 == true){
			glLoadIdentity();
			glTranslatef(-1.0f, 0.3f, 0.0f);
			DrawText(textSheet, "TIME LEFT: " + to_string(gameTimer), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		glLoadIdentity();
		glTranslatef(-1.0f, 0.0f, 0.0f);
		DrawText(textSheet, "Press RETURN to Exit", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(-1.0f, -0.3f, 0.0f);
		DrawText(textSheet, "Player 1's Score: " + to_string(p1score), 0.08f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(-1.0f, -0.6f, 0.0f);
		DrawText(textSheet, "Player 2's Score: " + to_string(p2score), 0.08f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (winLose == "neither"){
		glTranslatef(-1.0f, 0.6f, 0.0f);
		DrawText(textSheet, "Match ended in a draw.", 0.10f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		if (wasState1 == true){
			glLoadIdentity();
			glTranslatef(-1.0f, 0.3f, 0.0f);
			DrawText(textSheet, "TIME LEFT: " + to_string(gameTimer), 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		glLoadIdentity();
		glTranslatef(-1.0f, 0.0f, 0.0f);
		DrawText(textSheet, "Press RETURN to Exit", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(-1.0f, -0.3f, 0.0f);
		DrawText(textSheet, "Player 1's Score: " + to_string(p1score), 0.08f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(-1.0f, -0.6f, 0.0f);
		DrawText(textSheet, "Player 2's Score: " + to_string(p2score), 0.08f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}


void render() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (gamestate) {
	case 0:
		renderMenu();
		break;
	case 1:
		renderGameLevel1();
		break;
	case 2:
		renderGameLevel2();
		break;
	case 3:
		renderGameLevel3();
		break;
	case 4:
		renderGameOver();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);

}

int main(int argc, char *argv[]){
	srand(time(NULL));
	Setup();

	insertLevelData();

	float lastFrameTicks = 0.0f;
	
	while (!done) {
		
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
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