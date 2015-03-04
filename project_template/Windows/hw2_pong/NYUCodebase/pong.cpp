//click to start
//click more to speed up the ball

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include <vector>
using namespace std;

SDL_Window* displayWindow;

bool done = false;

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
	GLfloat quad[] = { 0.0f, 0.0f + height / 600.0f, 0.0f, 0.0f, 0.0f + width / 800.0f, 0.0f, 0.0f + width / 800.0f, 0.0f + height / 600.0f };
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

void Entity::Draw(){
	DrawSprite(textureID, width, height, x, y, rotation);
}

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

}

void ProcessEvents(float& elapsed){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_W) {
				entities[0].y += 0.05f; 
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_S) {
				entities[0].y += -0.05f; 
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
				entities[1].y += 0.05f;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
				entities[1].y += -0.05f;
			}
		}
		
		if (event.type == SDL_MOUSEBUTTONDOWN){
			if (event.button.button == 1){
				entities[2].direction_x += elapsed * 500;
				entities[2].direction_y += elapsed * 300;
			}
		}
		

	}
}

void Update(float& time){
	entities[2].x +=  entities[2].direction_x * (time / 3.0f);
	entities[2].y += entities[2].direction_y * (time / 10.0f);	

	if (entities[2].y /*+ entities[2].height / 600.0f*/ > 1.0f || entities[2].y /*+ entities[2].height / 600.0f*/ < -1.0f) {
		entities[2].direction_y = -entities[2].direction_y;
	}
	if (
		(
		(entities[2].x < entities[0].x + entities[0].width/800.0f) &&
		(entities[2].x > entities[0].x - entities[0].width/800.0f) &&
		(entities[2].y < entities[0].y + entities[0].height/50.0f) &&
		(entities[2].y > entities[0].y - entities[0].height/50.0f)
		)
		||
		(
		(entities[2].x > entities[1].x - entities[1].width/800.0f) &&
		(entities[2].x < entities[1].x + entities[1].width/800.0f) &&
		(entities[2].y < entities[1].y + entities[1].height/50.0f) &&
		(entities[2].y > entities[1].y - entities[1].height/50.0f)
		)
		)
	{	
		entities[2].direction_x = -entities[2].direction_x;
	}

	if (entities[2].x > 1.33f){
		entities[0].y = 0.0f;
		entities[1].y = 0.0f;
		entities[2].x = 0.0f;
		entities[2].y = 0.0f;
		entities[2].direction_x =0.0f;
		entities[2].direction_y =0.0f;
	}
	if (entities[2].x < -1.33f){
		entities[0].y = 0.0f;
		entities[1].y = 0.0f;
		entities[2].x = 0.0f;
		entities[2].y = 0.0f;
		entities[2].direction_x = 0.0f;
		entities[2].direction_y = 0.0f;
	}
}


void Render(GLuint& texturePaddle1, GLuint& texturePaddle2, GLuint& textureCry){
	glClear(GL_COLOR_BUFFER_BIT);


	if (!entities.empty()){
		for (size_t i = 0; i < entities.size(); i++){
			entities[i].Draw();
		}
	}
	else{
		Entity paddle1(-1.3f, 0.0f, 0.0f, texturePaddle1, 5.0f, 10.0f, 1.0f, 0.0f, 0.0f);
		Entity paddle2(1.27f, 0.0f, 0.0f, texturePaddle2, 5.0f, 10.0f, 1.0f, 0.0f, 0.0f);
		Entity ball(0.0f, 0.0f, 0.0f, textureCry, 3.0f, 3.0f, 1.0f, 0.0f, 0.0f);

		entities.push_back(paddle1);
		entities.push_back(paddle2);
		entities.push_back(ball);
	}

	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[]){
	glClear(GL_COLOR_BUFFER_BIT);

	Setup();

	GLuint texturePaddle1 = LoadTexture("pong/cardBack_blue1.png");
	GLuint texturePaddle2 = LoadTexture("pong/cardBack_red1.png");
	GLuint textureCry = LoadTexture("pong/cry.png");

	float lastFrameTicks = 0.0f;

	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		ProcessEvents(elapsed);
		if (!entities.empty()){
			Update(elapsed);
		}
		Render(texturePaddle1, texturePaddle2, textureCry);

	}

	SDL_Quit();
	return 0;
}