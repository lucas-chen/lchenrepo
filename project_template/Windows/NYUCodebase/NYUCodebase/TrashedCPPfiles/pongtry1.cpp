//-mouse click event does not work - wanted to initiate ball movement with mouse click
//-could not use the system clock (using variables lastFrameTicks and amountToChangeToTime, etc.)
//-the game crashes after ~15 seconds
//-collision fails

//a vector was made: entities[0] = paddle (left), entities[1] = paddle (right), entities[2] = ball
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include <vector>
using namespace std;

bool done = false;

SDL_Window* displayWindow;

float paddle1XMover = 0.0f;
float paddle1YMover = 0.0f;
float paddle2XMover = 0.0f;
float paddle2YMover = 0.0f;
float ballXMover = 0.02f;
float ballYMover = 0.02f;

const float PI = 3.14159265358979f;


float lastFrameTicks = 0.0f;
float amountToChangeToTime = 0.0f;


GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
	glScalef(width, height, 1.0f);
	GLfloat quad[] = { -(width/2.0f), (height/2.0f), (-width/2.0f), (-height/2.0f), (width/2.0f), -(height/2.0f), (width/2.0f), (height/2.0f)};
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0};
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


	GLuint getTextureID(){
		return textureID;
	}

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


bool Entity::operator== (const Entity& other) const {
	return &textureID == &(other.textureID);
}

vector<Entity> entities;

bool entityInVector(Entity& entityToCheck){
	for (size_t i = 0; i < entities.size(); i++){
		if (entities[i] == entityToCheck){
			return true;
		}
	}
	return false;
}

void Entity::Draw(){
	DrawSprite(textureID, width, height, x + (direction_x * speed), y + (direction_y * speed), rotation);
}


void Setup(){
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, 800, 600);
	glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);

}

void ProcessEvents(){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_W) {
				paddle1YMover += 0.05f;
				entities[0].y += paddle1YMover;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_S) {
				paddle1YMover -= 0.05f;
				entities[0].y -= paddle1YMover;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
				paddle2YMover += 0.05f;
				entities[1].y += paddle2YMover;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
				paddle2YMover -= 0.05f;
				entities[1].y -= paddle2YMover;

			}
			/////////////////////////////////////
			switch (avg) {
			case 1: { /* code block 1 */ } break;
			case 2: { /* code block 2 */ } break;
			default: { /* code block default */ } break;
			}
			can be read as

				if (1 == avg) { /* code block 1 */ }
				else if (2 == avg) { /* code block 2 */ }
				else { /*code block default */ }
				///////////////////////////////////
		}
			else if /*(event.key.keysym.scancode == SDL_SCANCODE_SPACE)*/ (event.type == SDL_MOUSEBUTTONDOWN){
				if (event.button.button == 1){
					ballXMover += 10.03f;
					ballYMover += 10.03f;
				}
			}
		}
}

void Update(){

	float ticks = (float)SDL_GetTicks() / 1000.0f; //1000 = miliseconds
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	amountToChangeToTime += elapsed * 0.0001f;

	//original movement of the ball; used in place of ticks because ticks kept crashing with an error message before game window even displays textures
	// though it now works, it's still slightly wonky and I can't get the timing right
	//ballXMover += amountToChangeToTime;
	//ballYMover += amountToChangeToTime;
	//ballXMover += 0.001f;
	//ballYMover += 0.0001f;


	if (!entities.empty()){
		//ball hits upper part of right paddle
		if (((entities[2].x + entities[2].direction_x + entities[2].width / 2) > ((entities[1].x - entities[1].width / 2)))){
			//ballXMover *= amountToChangeToTime;
			ballXMover *= -1.0f;

			if (((entities[2].y + entities[2].direction_y + entities[2].height / 2) > ((entities[1].y)))){
				ballYMover *= -1.0f;
			}
			if (((entities[2].y + entities[2].height / 2) < ((entities[1].y)))){
				ballYMover *= -1.0f;
			}

		}
		if (((entities[2].x - entities[2].direction_x - entities[2].width / 2) < ((entities[0].x + entities[0].width / 2)))){
			//ballXMover *= amountToChangeToTime;
			ballXMover *= -1.0f;

			if (((entities[2].y + entities[2].height / 2) > ((entities[1].y)))){
				ballYMover *= -1.0f;
			}
			if (((entities[2].y + entities[2].height / 2) < ((entities[1].y)))){
				ballYMover *= -1.0f;
			}

		}



		//if ball hits top or bottom of the screen, it SHOULD bounce off
		if ((entities[2].y + entities[2].direction_y) == 1.0f){
			ballYMover *= -1.0f;
		}
		if ((entities[2].y + entities[2].direction_y) == -1.0f){
			ballYMover *= -1.0f;
		}

	}
}

void Render(){
	glMatrixMode(GL_MODELVIEW);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint texturePaddle1 = LoadTexture("pong/cardBack_blue1.png");
	GLuint texturePaddle2 = LoadTexture("pong/cardBack_red1.png");
	GLuint textureCry = LoadTexture("pong/cry.png");

	Entity paddle1(-1.3f, 0.0f, 0.0f, texturePaddle1, 0.25f, 0.7f, 1.0f, 0.0f, 0.0f);
	paddle1.direction_x = paddle1XMover;
	paddle1.direction_y = paddle1YMover;
	paddle1.Draw();
	if (!(entityInVector(paddle1))){
		entities.push_back(paddle1);
	}
	

	Entity paddle2(1.3f, 0.0f, 0.0f, texturePaddle2, 0.25f, 0.7f, 1.0f, 0.0f, 0.0f);
	paddle2.direction_x = paddle2XMover;
	paddle2.direction_y = paddle2YMover;
	paddle2.Draw();
	if (!(entityInVector(paddle2))){
		entities.push_back(paddle2);
	}

	Entity ball(0.0f, 0.0f, 0.0f, textureCry, 0.2f, 0.2f, 1.0f, 0.0f, 0.0f);
	ball.direction_x = ballXMover;
	ball.direction_y = ballYMover;
	ball.Draw();
	/*  Adding this crashes for some reason -meant to reset the ball to center after the ball reaches the boundary
	if (!entities.empty()){
		if ((entities[2].x + entities[2].direction_x) > 1.33){
			entities[2].direction_x = 0;
			ball.Draw(); 
		}
		if ((entities[2].x - entities[2].direction_x) < -1.33){
			entities[2].direction_x = 0;
			ball.Draw(); 
		}
	}
	*/
	if (!(entityInVector(ball))){
		entities.push_back(ball);
	}

	glLoadIdentity();

	SDL_GL_SwapWindow(displayWindow);
}



int main(int argc, char *argv[]){
	Setup();

	while (!done) {

		ProcessEvents();
		Update();
		Render();

	}

	SDL_Quit();
	return 0;
}