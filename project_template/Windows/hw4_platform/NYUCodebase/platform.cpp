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
int score; 

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

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID, float u, float v, float width, float height);
	void Draw(float scale, float x, float y);
	bool operator==(const SheetSprite& other) const;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

bool SheetSprite::operator== (const SheetSprite& other) const {
	return &textureID == &(other.textureID);
}

void SheetSprite::Draw(float scale, float x, float y) {
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

bool areBulletsCollidingWithShip(Bullet* bullet, Ship* ship){
	if (bullet->yPos - bullet->height * 0.5 > ship->yPos + ship->height * 0.5){
		return false;
	}
	if (bullet->yPos + bullet->height * 0.5 < ship->yPos - ship->height * 0.5){
		return false;
	}
	if (bullet->xPos + bullet->width * 0.5 < ship->xPos - ship->width * 0.5){
		return false;
	}
	if (bullet->xPos - bullet->width * 0.5 > ship->xPos + ship->width * 0.5){
		return false;
	}
	return true;
}

bool areBulletsCollidingWithAliens(Bullet* bullet, Alien* alien){
	if (bullet->yPos - bullet->height * 0.5 > alien->yPos + alien->height * 0.5){
		return false;
	}
	if (bullet->yPos + bullet->height * 0.5 < alien->yPos - alien->height * 0.5){
		return false;
	}
	if (bullet->xPos + bullet->width * 0.5 < alien->xPos - alien->width * 0.5){
		return false;
	}
	if (bullet->xPos - bullet->width * 0.5 > alien->xPos + alien->width * 0.5){
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

	score = 0; 
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

void updateGame(float elapsed, int bulletTexture){
	SDL_Event event;

	if (ships[0]->lives > 0){

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
					state = STATE_GAME_OVER;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					//Bullet* bullet = new Bullet(bulletTexture, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f);
					//bullet->xPos = ships[0]->xPos;
					//bullet->yPos = ships[0]->yPos;
					//myBullets.push_back(bullet);
				}
			}
		}
		/*
		for (int i = 0; i < theirBullets.size(); i++){
			if (areBulletsCollidingWithShip(theirBullets[i], ship[0])){
				ships[0]->lives = ships[0]lives - 1;
			}
		}

		for (int j = 0; j < myBullets.size(); j++){
			for (int k = 0; k < aliens.size(); k++){
				if (areBulletsCollidingWithAliens(myBullets[j], aliens[k]){
					delete aliens[k];
					myBullets[j].xPos = 2.0;
					myBullets[j].yPos = 2.0; 
				}
			}
		}
		*/

	}

	else if (ships[0]->lives == 0){
		state = STATE_GAME_OVER;
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
	DrawText(textTexture, "Press SPACE to Shoot, Arrow Keys to Move", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-1.2f, -0.3f, 0.0f);
	DrawText(textTexture, "Press SPACE to start the game, BACKSPACE to exit", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	//USE TO cout VALUES
	/*
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);
	DrawText(textTexture, to_string(ships[0]->v), 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	*/
	
}
void renderGame(int textTexture, int spriteSheet, int spriteSheet2) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.3f, 0.95f, 1.0f);
	string StringScore = to_string(score);
	DrawText(textTexture, "Score: " + StringScore, 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();	

	for (size_t i = 0; i < ships.size(); i++){
		ships[i]->Draw(2.0, ships[i]->xPos, ships[i]->yPos);
	}
	for (size_t j = 0; j < aliens.size(); j++){
		if (aliens[j]->alive){
			aliens[j]->Draw(2.0, aliens[j]->xPos, aliens[j]->yPos);
		}
	}
	for (size_t k = 0; k < myBullets.size(); k++){
		myBullets[k]->Draw(2.0, myBullets[k]->xPos, myBullets[k]->yPos);
	}
	for (size_t l = 0; l < theirBullets.size(); l++){
		theirBullets[l]->Draw(2.0, theirBullets[l]->xPos, theirBullets[l]->yPos);
	}
	glLoadIdentity();
}
void renderGameOver(int textTexture) {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-0.9f, 0.0f, 0.0f);
	DrawText(textTexture, "GAME OVER", 0.2f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void render(int textTexture, int spriteSheet, int spriteSheet2) {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (state) {
	case STATE_MAIN_MENU:
		renderMenu(textTexture);
		break;
	case STATE_GAME_LEVEL:
		renderGame(textTexture, spriteSheet, spriteSheet2);
		break;
	case STATE_GAME_OVER:
		renderGameOver(textTexture);
		break;
	}
	SDL_GL_SwapWindow(displayWindow);

}

void loadAliens(int spriteSheetTexture){
	
	float alienU = getUniformU(89, 30, 16);
	float alienV = getUniformV(89, 30, 16);
	float alienWidth = getUniformWidth(89, 30, 16);
	float alienHeight = getUniformHeight(89, 30, 16);

	// shoves aliens into a vector
	for (float j = 0.8f; j > 0.6f; j = j - 0.1f){
		for (float i = -0.5f; i < 0.5f; i = i + 0.1f) {
			Alien* alien = new Alien(spriteSheetTexture, alienU, alienV, alienWidth, alienHeight);
			alien->xPos = i;
			alien->yPos = j;
			aliens.push_back(alien);
		}
	}
}
/*
void loadBullets(int spriteSheetTexture){
	
	for (size_t i = 0; i < 5000; i++){
		Bullet bullet(spriteSheetTexture, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f);
		myBullets.push_back(bullet);
		theirBullets.push_back(bullet);
	}
	
}
*/

int main(int argc, char *argv[]){
	glClear(GL_COLOR_BUFFER_BIT);
	Setup();
	
	GLuint spriteSheet = LoadTexture("spaceinvaders/spritesheet.png"); // 30 x 30
	GLuint spriteSheet2 = LoadTexture("spaceinvaders/sheet.png");
	GLuint text = LoadTexture("spaceinvaders/font1.png");

	loadAliens(spriteSheet);
	//loadBullets(spriteSheet2);

	// setup for creating the textures
	float shipU = getUniformU(251, 30, 16);
	float shipV = getUniformV(251, 30, 16);
	float shipWidth = getUniformWidth(251, 30, 16);
	float shipHeight = getUniformHeight(251, 30, 16);

	Ship* player = new Ship(spriteSheet, shipU, shipV, shipWidth, shipHeight);
	ships.push_back(player);


	float lastFrameTicks = 0.0f;

	while (!done) {
		
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		

		//player->Draw(0.3, player->xPos, player->yPos);
		update(elapsed, spriteSheet2);
		render(text, spriteSheet, spriteSheet2);
		
	}



	SDL_Quit();
	return 0;
}