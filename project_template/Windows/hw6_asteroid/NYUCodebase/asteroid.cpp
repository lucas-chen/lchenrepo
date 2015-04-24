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
#include <stdlib.h>
#include <random>

#include <SDL_mixer.h>
using namespace std;

SDL_Window* displayWindow;

// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
// 30 FPS (1.0f/30.0f)
//#define FIXED_TIMESTEP 0.0333333f
#define MAX_TIMESTEPS 6
float timeLeftOver = 0.0f;


//enum GameState { 100, 101, 102 };
int state;
//float gravity_x = 0.0f;
//float gravity_y = -9.8f;


string winLose = "lose";
bool done = false;


class Vector{
public:
	Vector(){};
	Vector(float x, float y, float z) :
		x(x), y(y), z(z){}

	float length();
	void normalize();

	float x;
	float y;
	float z;
};

float Vector::length(){
	return sqrt(x*x + y*y + z*z);
}

void Vector::normalize() {
	float length = sqrt(x*x + y*y + z*z);
	x = x / length;
	y = y / length;
	z = z / length;
}

class Matrix {
public:
	Matrix();

	union {
		float m[4][4];
		float ml[16];
	};

	void identity();
	Matrix inverse();
	Matrix operator * (const Matrix& m2);
	Vector operator * (const Vector& v2);
};

void Matrix::identity() {
	m[0][0] = 1;	m[0][1] = 0;	m[0][2] = 0;	m[0][3] = 0;
	m[1][0] = 0;	m[1][1] = 1;	m[1][2] = 0;	m[1][3] = 0;
	m[2][0] = 0;	m[2][1] = 0;	m[2][2] = 1;	m[2][3] = 0;
	m[3][0] = 0;	m[3][1] = 0;	m[3][2] = 0;	m[3][3] = 1;
}

Matrix Matrix::inverse() {
	float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
	float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
	float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	float v0 = m20 * m31 - m21 * m30;
	float v1 = m20 * m32 - m22 * m30;
	float v2 = m20 * m33 - m23 * m30;
	float v3 = m21 * m32 - m22 * m31;
	float v4 = m21 * m33 - m23 * m31;
	float v5 = m22 * m33 - m23 * m32;

	float t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
	float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
	float t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
	float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

	float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	float d00 = t00 * invDet;
	float d10 = t10 * invDet;
	float d20 = t20 * invDet;
	float d30 = t30 * invDet;

	float d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	float d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	float d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	float d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	float d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	float d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	float d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	float d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	float d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	float d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	float d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	float d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	Matrix m2;

	m2.m[0][0] = d00;
	m2.m[0][1] = d01;
	m2.m[0][2] = d02;
	m2.m[0][3] = d03;
	m2.m[1][0] = d10;
	m2.m[1][1] = d11;
	m2.m[1][2] = d12;
	m2.m[1][3] = d13;
	m2.m[2][0] = d20;
	m2.m[2][1] = d21;
	m2.m[2][2] = d22;
	m2.m[2][3] = d23;
	m2.m[3][0] = d30;
	m2.m[3][1] = d31;
	m2.m[3][2] = d32;
	m2.m[3][3] = d33;

	return m2;
}

//matrix*matrix and matrix*vector multiplication code borrowed from Kong Huang's github

Matrix Matrix::operator * (const Matrix& m2) {
	Matrix res;

	res.m[0][0] = (m[0][0] * m2.m[0][0]) + (m[1][0] * m2.m[0][1]) + (m[2][0] * m2.m[0][2]) + (m[3][0] * m2.m[0][3]);
	res.m[0][1] = (m[0][1] * m2.m[0][0]) + (m[1][1] * m2.m[0][1]) + (m[2][1] * m2.m[0][2]) + (m[3][1] * m2.m[0][3]);
	res.m[0][2] = (m[0][2] * m2.m[0][0]) + (m[1][2] * m2.m[0][1]) + (m[2][2] * m2.m[0][2]) + (m[3][2] * m2.m[0][3]);
	res.m[0][3] = (m[0][3] * m2.m[0][0]) + (m[1][3] * m2.m[0][1]) + (m[2][3] * m2.m[0][2]) + (m[3][3] * m2.m[0][3]);

	res.m[1][0] = (m[0][0] * m2.m[1][0]) + (m[1][0] * m2.m[1][1]) + (m[2][0] * m2.m[1][2]) + (m[3][0] * m2.m[1][3]);
	res.m[1][1] = (m[0][1] * m2.m[1][0]) + (m[1][1] * m2.m[1][1]) + (m[2][1] * m2.m[1][2]) + (m[3][1] * m2.m[1][3]);
	res.m[1][2] = (m[0][2] * m2.m[1][0]) + (m[1][2] * m2.m[1][1]) + (m[2][2] * m2.m[1][2]) + (m[3][2] * m2.m[1][3]);
	res.m[1][3] = (m[0][3] * m2.m[1][0]) + (m[1][3] * m2.m[1][1]) + (m[2][3] * m2.m[1][2]) + (m[3][3] * m2.m[1][3]);

	res.m[2][0] = (m[0][0] * m2.m[2][0]) + (m[1][0] * m2.m[2][1]) + (m[2][0] * m2.m[2][2]) + (m[3][0] * m2.m[2][3]);
	res.m[2][1] = (m[0][1] * m2.m[2][0]) + (m[1][1] * m2.m[2][1]) + (m[2][1] * m2.m[2][2]) + (m[3][1] * m2.m[2][3]);
	res.m[2][2] = (m[0][2] * m2.m[2][0]) + (m[1][2] * m2.m[2][1]) + (m[2][2] * m2.m[2][2]) + (m[3][2] * m2.m[2][3]);
	res.m[2][3] = (m[0][3] * m2.m[2][0]) + (m[1][3] * m2.m[2][1]) + (m[2][3] * m2.m[2][2]) + (m[3][3] * m2.m[2][3]);

	res.m[3][0] = (m[0][0] * m2.m[3][0]) + (m[1][0] * m2.m[3][1]) + (m[2][0] * m2.m[3][2]) + (m[3][0] * m2.m[3][3]);
	res.m[3][1] = (m[0][1] * m2.m[3][0]) + (m[1][1] * m2.m[3][1]) + (m[2][1] * m2.m[3][2]) + (m[3][1] * m2.m[3][3]);
	res.m[3][2] = (m[0][2] * m2.m[3][0]) + (m[1][2] * m2.m[3][1]) + (m[2][2] * m2.m[3][2]) + (m[3][2] * m2.m[3][3]);
	res.m[3][3] = (m[0][3] * m2.m[3][0]) + (m[1][3] * m2.m[3][1]) + (m[2][3] * m2.m[3][2]) + (m[3][3] * m2.m[3][3]);

	return res;
}
Vector Matrix::operator * (const Vector& v2) {
	Vector res;

	res.x = (m[0][0] * v2.x) + (m[1][0] * v2.y) + (m[2][0] * v2.z) + (m[3][0] * 1);
	res.y = (m[0][1] * v2.x) + (m[1][1] * v2.y) + (m[2][1] * v2.z) + (m[3][1] * 1);
	res.z = (m[0][2] * v2.x) + (m[1][2] * v2.y) + (m[2][2] * v2.z) + (m[3][2] * 1);

	return res;
}

class Entity {
public:
	Entity(){};
	Entity(float u, float v, float width, float height) :
		u(u), v(v), width(width), height(height)
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

	float xPos, yPos;
	float u, v, width, height;
	float speed, velocity_x, velocity_y, acceleration_x, acceleration_y, friction_x, friction_y;

	bool collidedTop, collidedBottom, collidedLeft, collidedRight;

	string type;
	bool alive;

};

bool Entity::operator== (const Entity* other) const {
	return this == other;
}


//vectors
vector<Entity*> playerVect;
vector<Entity*> asteroidsVect;
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
GLuint spriteSheet;


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

/*
draw sprite with fixed index

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
*/

//draw sprite without fixed x by y spritesheet
void drawEntity(Entity* entity, float x, float y, GLuint spriteTexture, int index) {
	//width and height = tile_size
	float spriteWidth = 1.0f / (float)spriteCountX;
	float spriteHeight = 1.0f / (float)spriteCountY;
	GLfloat quad[] = { x + 0.5f * width, y - 0.5f * height, x + 0.5f * width, y + 0.5f * height, x - 0.5f * width, y + 0.5f * height, x - 0.5f * width, y - 0.5f * height };
	//GLfloat quad[] = { y - 0.5f * height, x - 0.5f * width, y + 0.5f * height, x - 0.5f * width, y + 0.5f * height, x + 0.5f * width, y - 0.5f * height, x + 0.5f * width };
	//GLfloat quadUVs[] = { u, v, u, v - spriteHeight, u + spriteWidth, v - spriteHeight, u + spriteWidth, v };
	//GLfloat quadUVs[] = { u, v, u, v + spriteHeight, u + spriteWidth, v + spriteHeight, u + spriteWidth, v };
	GLfloat quadUVs[] = { u + spriteWidth, v + spriteHeight, u + spriteWidth, v, u, v, u, v + spriteHeight };
	drawTexture(x, y, quad, quadUVs, spriteTexture);
}

/*
fixed indexes for spritesheet

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
*/

void createPlayer(){
	Entity* player = new Entity(211.0f / 1024.0f, 941.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f);
	player->xPos = 0.0f;
	player->yPos = 0.0f;
	player->type = "player";
	playerVect.push_back(player);
}

void createAsteroid(){
	std::random_device rd;
	std::default_random_engine generator(rd()); // rd() provides a random seed
	std::uniform_real_distribution<float> distribution(0.01f, 1.20f);

	float number = distribution(generator);

	Entity* asteroid = new Entity(224.0f / 1024.0f, 664.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f);
	float xPlacement = random
	asteroid->type = "enemy";
	asteroidsVect.push_back(asteroid);
}

//sprite sheet is not with fixed indexes
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

	state = 100;
	textSheet = LoadTexture("asteroid/font2.png");
	spriteSheet = LoadTexture("asteroid/sheet.png");
	
}

void updateMenu(float elapsed){
	SDL_Event event;


	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				state = 101;

			}
			if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
				state = 102;
				
			}
		}
	}
}

void updateGame(float elapsed){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {


		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
			state = 102;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			for (size_t i = 0; i < entities.size(); i++){
				if (entities[i]->type == "player"){
					if (entities[i]->collidedBottom){
						entities[i]->collidedBottom = false;
						glMatrixMode(GL_MODELVIEW);
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
					state = 102;
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
		//entities[i]->velocity_x += gravity_x * FIXED_TIMESTEP;
		//entities[i]->velocity_y += gravity_y * FIXED_TIMESTEP;
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
	if (state == 100){
		updateMenu(elapsed);
	}
	else if (state == 101){
		updateGame(elapsed);
	}
	else if (state == 102){
		updateGameOver(elapsed);
	}
}

void renderMenu() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.0f, 0.3f, 0.0f);
	DrawText(textSheet, "Avoid the asteroids", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-1.0f, 0.0f, 0.0f);
	DrawText(textSheet, "Use UP, DOWN, LEFT and RIGHT", 0.04f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
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
	glLoadIdentity();
	glPushMatrix();
	/*
	glTranslatef(-TILE_SIZE* mapWidth / 2, TILE_SIZE* mapHeight / 2, 0.0f);
	glDrawArrays(GL_QUADS, 0, vertexData.size()/2);
	glDisable(GL_TEXTURE_2D);
	*/

	int playerEntitiesIndex = 0;
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->type == "player") { playerEntitiesIndex = i; }
	}
	glLoadIdentity();
	
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i]->alive && !(entities[i]->type == "platform")) {
			entities[i]->render(spriteSheet);
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
	case 100:
		renderMenu();
		break;
	case 101:
		renderGame();
		break;
	case 102:
		renderGameOver();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);

}

int main(int argc, char *argv[]){

	Setup();

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