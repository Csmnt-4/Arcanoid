/*
	Example program that demonstrates simple physics using Box2D
*/
#include "Blit3D.h"

#include <random>
#include "Physics.h"

Blit3D* blit3D = NULL;

//memory leak detection
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Rectangle.h"
#include "Ball.h"

#include "GameSprites.h"
#include "Camera.h"

//GLOBAL DATA
std::mt19937 rng;

//Box2D does not use glm::vec2, but instead provides b2Vec2, a similiar
//2d vector object
b2Vec2 gravity; //defines our gravity vector
b2World* world = NULL; //our physics engine

b2Body* groundBody = NULL; //the physics body for the screen edges
std::vector<Ball*> balls;
std::vector<Brick*> bricks;

// Prepare for simulation. Typically we use a time step of 1/60 of a
// second (60Hz) and ~10 iterations. This provides a high quality simulation
// in most game scenarios.
int32 velocityIterations = 8;
int32 positionIterations = 3;

float timeStep = 1.f / 60.f; //one 60th of a second
float elapsedTime = 0; //used for calculating time passed

bool kickBall = false; //tracks whether we should apply a kick to the ball next update

//game state configuration
enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;
//camera
Camera2D* camera = NULL;

//coursor
glm::vec2 cursor;

//font
AngelcodeFont* debugFont = NULL;

//ball and platform
Ball* theBall = NULL;
Brick* theBrick = NULL;

int lives = 3;
int playerScore = 0;
int levelNumber = 1;
boolean stickToYourPlatform = true;
boolean nextLevel = true;
boolean nextLevelAvailable = true;

//Sprites
GameSprites* sprites;
void ProceedToNextLevel()
{
	levelNumber++;
	std::string level = "Levels\\" + std::to_string(levelNumber);
	level += ".lvl";
	bricks.clear();
	bricks = ReadBricksFromFile(level, sprites);
}

b2Body* CreateWindowEdges(int windowWidth, int windowHeight, b2World* world)
{
	//A bodyDef for the ground
	b2BodyDef groundBodyDef;
	// Define the ground body.
	groundBodyDef.position.Set(0, 0);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	b2Body* groundBody = world->CreateBody(&groundBodyDef);

	//an EdgeShape object, for the ground
	b2EdgeShape groundBox;

	// Define the ground as 4 edge shapes around the edge of the screen.
	b2FixtureDef boxShapeDef;

	boxShapeDef.shape = &groundBox;

	//bottom
	groundBox.SetTwoSided(b2Vec2(0, 0), b2Vec2(windowWidth / PTM_RATIO, 0));
	//Create the fixture
	groundBody->CreateFixture(&boxShapeDef);

	//left
	groundBox.SetTwoSided(b2Vec2(0, windowHeight / PTM_RATIO), b2Vec2(0, 0));
	groundBody->CreateFixture(&boxShapeDef);

	//top
	groundBox.SetTwoSided(b2Vec2(0, windowHeight / PTM_RATIO),
		b2Vec2(windowWidth / PTM_RATIO, windowHeight / PTM_RATIO));
	groundBody->CreateFixture(&boxShapeDef);

	//right
	groundBox.SetTwoSided(b2Vec2(windowWidth / PTM_RATIO,
		0), b2Vec2(windowWidth / PTM_RATIO, windowHeight / PTM_RATIO));
	groundBody->CreateFixture(&boxShapeDef);

	return groundBody;
}

void Init()
{
	//make a camera
	camera = new Camera2D();

	std::random_device rd;
	rng.seed(rd());

	debugFont = blit3D->MakeAngelcodeFontFromBinary32("Media\\DebugFont_24.bin");
	//from here on, we are setting up the Box2D physics world model

	// Define the gravity vector.
	gravity.x = 0.0f;
	gravity.y = 0.f;

	// Construct a world object, which will hold and simulate the rigid bodies.
	world = new b2World(gravity);
	//world->SetGravity(gravity); <-can call this to change gravity at any time
	world->SetAllowSleeping(true); //set true to allow the physics engine to 'sleep" objects that stop moving

	groundBody = CreateWindowEdges(blit3D->screenWidth, blit3D->screenHeight, world);

	blit3D->ShowCursor(false);
	sprites = new GameSprites(blit3D);

	std::string level = "Levels\\" + std::to_string(levelNumber);
	level += ".lvl";
	bricks = ReadBricksFromFile(level, sprites);
	theBrick = new Brick(sprites->movingPlatformSprites, 108, 30, 960, 300, 500.f, 0.1f, 1.2f);
	theBrick->body->SetBullet(true);
	theBall = new Ball(sprites->ballSprites.at(0), 24.f, 50.f, 100.f,
		1.f, 0.1f, 1.2f, 0.f);
	balls.push_back(theBall);
}

void DeInit(void)
{
	//Free all physics game data we allocated

	if (camera != NULL) delete camera;

	delete world;
	delete theBall;
	delete theBrick;
	//any sprites/fonts still allocated are freed automatcally by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	//stop it from lagging hard if more than a small amount of time has passed
	if (seconds > 1.0 / 30) elapsedTime += 1.f / 30;
	else elapsedTime += (float)seconds;

	if (kickBall)
	{
		kickBall = false;
		KickBall(theBall);
	}

	//don't apply physics unless at least a timestep worth of time has passed
	while (elapsedTime >= timeStep)
	{
		camera->Update((float)seconds);
		camera->PanTo(960, 540);
		//update the physics world
		world->Step(timeStep, velocityIterations, positionIterations);

		// Clear applied body forces.
		world->ClearForces();
		//waiting for the next level
		nextLevel = true;
		int bricksLeft = 0;

		//updating bricks and applying shake
		for each (Brick * b in bricks)
		{
			if (b->body->IsEnabled() == true)
			{
				//postponing, if there are blocks left
				nextLevel = false;
				bricksLeft++;
			}

			if (b->body->GetLinearVelocity().Length() > 0)
			{
				camera->Shake(10.f * (b->spriteNumber + 1));
				playerScore += (int)10 / (b->spriteNumber + 1);
				b->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
				b->Destroy();
			}
		}

		if (nextLevel)
		{
			ProceedToNextLevel();
		}

		if (bricksLeft <= 5)
		{
			nextLevelAvailable = true;
		}

		//updating the platform, following the cursor
		theBrick->body->SetTransform(b2Vec2(cursor.x / PTM_RATIO, 30), theBrick->body->GetAngle());
		if (theBrick->body->GetAngle() != 0)
		{
			theBrick->body->SetAngularVelocity(-theBrick->body->GetAngle());
		}
		else
		{
			theBrick->body->SetAngularVelocity(0);
		}

		//updating the ball

		if (theBall->body->GetPosition().y < 5)
		{
			lives--;
			stickToYourPlatform = true;
		}

		if (stickToYourPlatform) {
			theBall->body->SetTransform(b2Vec2(cursor.x / PTM_RATIO, 36), 0);
		}

		//check if we are not moving enough vertically
		b2Vec2 dir = theBall->body->GetLinearVelocity();
		float minAmountY = 30.f / PTM_RATIO;
		if (dir.y < 0)
		{
			if (dir.y > -minAmountY)
			{
				dir.y = -minAmountY;
			}
		}
		else
		{
			if (dir.y < minAmountY)
			{
				dir.y = minAmountY;
			}
		}
		dir.Normalize();
		dir *= theBall->body->GetLinearVelocity().Length(); //scale up the velocity tp correct ball speed
		theBall->body->SetLinearVelocity(dir);
		//std::cout << theBall->body->GetLinearVelocity().Length() << " - " << vec2deg(theBall->body->GetLinearVelocity()) << std::endl;

		elapsedTime -= timeStep;
	}
}

void Draw(void)
{
	glClearColor(0.8f, 0.6f, 0.7f, 0.0f);	//clear colour: r,g,b,a
	// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//transform the view by "drawing" the camera
	camera->Draw();
	//switch (gameState)
	//{
	//case START:
		//logo->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		//break;

	//case PLAYING:
		//loop over all entities and draw them
		//for (auto& e : entityList) e->Draw();
		//for (auto& b : brickEntityList) b->Draw();
		//for (auto& b : ballEntityList) b->Draw();
		//for (auto& p : particleList) p->Draw();

		//std::string lifeString = "Lives: " + std::to_string(lives);
		//debugFont->BlitText(20, 30, lifeString);
		//break;
	theBall->Draw();
	theBrick->Draw();

	for each (Brick * b in bricks) b->Draw();

	if (nextLevelAvailable)
	{
		std::string proceed = "Press \"Enter\" to proceed to the next level";
		debugFont->BlitText(1400, 70, proceed);
	}

	std::string score = "Lives: " + std::to_string(lives);
	debugFont->BlitText(50, 80, score);

	std::string score = "Score: " + std::to_string(playerScore);
	debugFont->BlitText(50, 70, score);
	//case GAMEOVER:
	//}

	camera->UnDraw(); //turns off the camera transformation of the view coords
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && stickToYourPlatform == true)
	{
		stickToYourPlatform = false;
		kickBall = true;
	}

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && nextLevelAvailable)
		ProceedToNextLevel();
}

void DoCursor(double x, double y)
{
	//transform cursor coordinates from Windows-space to Blit3D-space
	cursor.x = x;
	cursor.y = (blit3D->trueScreenHeight - y) * (blit3D->screenHeight / blit3D->trueScreenHeight);
}

int main(int argc, char* argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	blit3D = new Blit3D(Blit3DWindowModel::FULLSCREEN, 1920, 1080);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);
	blit3D->SetDoCursor(DoCursor);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}