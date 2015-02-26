//press space to start

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include <vector>
using namespace std;

SDL_Window* displayWindow;

bool done = false;

enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };
int state;

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

void DrawSprite(GLint texture, float width, float height, float x, float y, float rotation) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 0.0f);
	glRotatef(rotation, 0.0f, 0.0f, 1.0f);
	//glScalef(width, height, 1.0f);
	GLfloat quad[] = { 0.0f, 0.0f + height, 0.0f, 0.0f, 0.0f + width, 0.0f, 0.0f + width, 0.0f + height};
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

class Entity {
public:
	Entity(float x, float y, float rotation, GLuint textureID, float width, float height, float speed, float direction_x, float direction_y) :
		x(x), y(y), rotation(rotation), textureID(textureID), width(width), height(height), speed(speed), direction_x(direction_x), direction_y(direction_y){}

	bool operator==(const Entity& other) const;

	void Draw();
	float x;
	float y;
	float rotation;
	GLuint textureID;
	float width;
	float height;
	float speed;
	float direction_x;
	float direction_y;
};

class Alien{
	Alien(float x, float y, float rotation, GLuint textureID, float width, float height, float speed, float direction_x, float direction_y) :
		x(x), y(y), rotation(rotation), textureID(textureID), width(width), height(height), speed(speed), direction_x(direction_x), direction_y(direction_y){}

	void Draw();
	float x;
	float y;
	float rotation;
	GLuint textureID;
	float width;
	float height;
	float speed;
	float direction_x;
	float direction_y;
	bool alive;
};

class Ship{
	Ship(float x, float y, float rotation, GLuint textureID, float width, float height, float speed, float direction_x, float direction_y) :
		x(x), y(y), rotation(rotation), textureID(textureID), width(width), height(height), speed(speed), direction_x(direction_x), direction_y(direction_y){}

	void Draw();
	float x;
	float y;
	float rotation;
	GLuint textureID;
	float width;
	float height;
	float speed;
	float direction_x;
	float direction_y;
	bool alive;
};

class Bullet{
	Bullet(float x, float y, float rotation, GLuint textureID, float width, float height, float speed, float direction_x, float direction_y) :
		x(x), y(y), rotation(rotation), textureID(textureID), width(width), height(height), speed(speed), direction_x(direction_x), direction_y(direction_y){}

	void Draw();
	float x;
	float y;
	float rotation;
	GLuint textureID;
	float width;
	float height;
	float speed;
	float direction_x;
	float direction_y;

};

void Entity::Draw(){
	DrawSprite(textureID, width, height, x, y, rotation);
}

void Alien::Draw(){
	DrawSprite(textureID, width, height, x, y, rotation);
}

void Ship::Draw(){
	DrawSprite(textureID, width, height, x, y, rotation);
}


bool Entity::operator== (const Entity& other) const {
	return &textureID == &(other.textureID);
}


class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID, float u, float v, float width, float height);
	void Draw(float scale, float x, float y, float rotation);
	float scale;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

void SheetSprite::Draw(float scale, float x, float y, float rotation) {

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 0.0f);
	glRotatef(rotation, 0.0f, 0.0f, 1.0f);
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
}

/*
spriteSheetTexture = LoadTexture("sheet.png");
mySprite = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/
1024.0f, 84.0f/1024.0f);

int index = 10;
int spriteCountX = 30;
int spriteCountY = 16;
float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
float spriteWidth = 1.0/(float)spriteCountX;
float spriteHeight = 1.0/(float)spriteCountY;
GLfloat quadUVs[] = { u, v,
u, v+spriteHeight,
u+spriteWidth, v+spriteHeight,
u+spriteWidth, v
};
u,v
spriteWidth


void DrawSpriteSheetSprite(int spriteTexture, int index, int spriteCountX, int
	spriteCountY) {
	// our regular sprite drawing
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	GLfloat quadUVs[] = { u, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight,
		u + spriteWidth, v
	};
	// our regular sprite drawing
}
*/

void DrawText(GLuint fontTexture, string text, float r, float g, float b, float a, float width, float height, float x, float y, float scale) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 0.0f);
	GLfloat quadColors[] = { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a };
	glColorPointer(4, GL_FLOAT, 0, quadColors);
	glEnableClientState(GL_COLOR_ARRAY);
	GLfloat quad[] = { -width * scale, height * scale, -width * scale, -height * scale,
		width * scale, -height * scale, width * scale, height * scale };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, text.size() * 4);
	glDisableClientState(GL_COLOR_ARRAY);
}

vector<Ship> ships;
vector<Alien> aliens;

void loadBullets(vector<Entity*>& bulletPack){
	for (size_t i = 0; i < 5000; i++){
		bulletPack.push_back(newBullet);
	}
}

bool entityInVector(Entity& entityToCheck, vector<Entity>& vectorToCheck){
	for (size_t i = 0; i < vectorToCheck.size(); i++){
		if (vectorToCheck[i] == entityToCheck){
			return true;
		}
	}
	return false;
}

bool areEntitiesColliding(Entity& entity1, Entity& entity2){
	if (entity1.y - entity1.height * 0.5 > entity2.y + entity2.height * 0.5){
		return false;
	}
	if (entity1.y + entity1.height * 0.5 < entity2.y - entity2.height * 0.5){
		return false;
	}
	if (entity1.x + entity1.width * 0.5 < entity2.x - entity2.width * 0.5){
		return false;
	}
	if (entity1.x - entity1.width * 0.5 > entity2.x + entity2.width * 0.5){
		return false;
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

void ProcessEventsStart(){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}

		if (event.type == SDL_KEYDOWN){
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				break;
			}
		}
	}
}

void ProcessEventsGame(float& elapsed){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
				
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
				
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				
			}
		}
	}
}

void update(float elapsed) {
	switch (state) {
	case STATE_MAIN_MENU:
		updateMainMenu(elapsed);
		break;
	case STATE_GAME_LEVEL:
		updateGameLevel(elapsed);
		break;
	case STATE_GAME_OVER:
		updateGameOver(elapsed);
		break;
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT);

	switch (state) {
	case STATE_MAIN_MENU:
		renderMainMenu();
		break;
	case STATE_GAME_LEVEL:
		renderGameLevel();
		break;
	case STATE_GAME_OVER:
		renderGameOver();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);
}

void renderMainMenu(int textTexture) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-0.8f, 0.7f, 0.0f);
	glTranslatef(-0.8f, 0.2f, 0.0f);
	glLoadIdentity();
	glTranslatef(-0.8f, 0.0f, 0.0f);
	DrawText(textTexture, "Press SPACE to Shoot, Arrow Keys to Move", 1.0, 1.0, 1.0, 1.0, 5.0, 10.0, -1.0, 0.3, 0.0);
	glLoadIdentity();
	glTranslatef(-0.8f, -0.7f, 0.0f);
	DrawText(textTexture, "Press SPACE to start the game", 1.0, 1.0, 0.0, 1.0, 5.0, 10.0, -1.0, -0.3, 0.0);
}
void renderGameLevel(int textTexture) {
	for (size_t i = 0; i < entities.size(); i++) {
		entities[i]->Render();
	}

	playerBullet.Render();

	for (size_t i = 0; i < 5; i++) {
		enemyBullets[i].Render();
	}

	for (size_t i = 0; i < defences.size(); i++){
		defences[i]->Render();
	}

	glLoadIdentity();
	glTranslatef(0.8f, -0.9f, 0.0f);
	DrawText(fontSheetTexture, "Score: " + to_string(score), 0.05, 0.0, 0.0, 1.0, 0.0, 1.0);
}
void renderGameOver(int textTexture) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-0.8f, 0.7f, 0.0f);
	DrawText(fontSheetTexture, "GAME OVER", 0.1, 0.0, 0.0, 1.0, 0.0, 1.0);
	glLoadIdentity();
	glTranslatef(-0.8f, 0.2f, 0.0f);
	DrawText(fontSheetTexture, "High Score: " + to_string(highScore), 0.05, 0.0, 1.0, 1.0, 1.0, 1.0);
	glLoadIdentity();
	glTranslatef(-0.8f, 0.0f, 0.0f);
	DrawText(fontSheetTexture, "Round Score: " + to_string(roundScore), 0.05, 0.0, 1.0, 1.0, 1.0, 1.0);
	glLoadIdentity();
	glTranslatef(-0.8f, -0.7f, 0.0f);
	DrawText(fontSheetTexture, "Press Z to go back to main menu", 0.05, 0.0, 0.0, 1.0, 0.0, 1.0);
}


void Render(){
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint alienSprites = LoadTexture("spaceinvaders/spritesheet.png");
	GLuint ship = LoadTexture("spaceinvaders/spritesheet.png");
	GLuint bullet = LoadTexture("spaceinvaders/spritesheet.png");
	GLuint text = LoadTexture("spaceinvaders/pixel_font.png");



	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[]){
	glClear(GL_COLOR_BUFFER_BIT);

	Setup();
	ProcessEventsStart();

	float lastFrameTicks = 0.0f;

	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		ProcessEventsGame(elapsed);
		Update(elapsed);
		Render();

	}

	SDL_Quit();
	return 0;
}