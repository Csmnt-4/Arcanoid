#include "Blit3D.h"

#include <random>
#include "Physics.h"

Blit3D* blit3D = NULL;

//memory leak detection
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Brick.h"
#include "Ball.h"

#include "GameSprites.h"
#include "Camera.h"
#include "CollisionMasks.h"
#include "Particle.h"
#include "ContactListener.h"

//GLOBAL DATA
std::mt19937 rng;
DiceRoller dice;

//Box2D does not use glm::vec2, but instead provides b2Vec2, a similiar
//2d vector object
b2Vec2 gravity; //defines our gravity vector
b2World* world = NULL; //our physics engine

b2Body* groundBody = NULL; //the physics body for the screen edges

//all game sprites
GameSprites* sprites;

//contact listener to handle collisions between important objects
ContactListener* contactListener;

//camera
Camera2D* camera = NULL;

//coursor
glm::vec2 cursor;

//font
AngelcodeFont* debugFont = NULL;

//objects
std::vector<Ball*> balls;
std::vector<Brick*> bricks;
std::vector<Brick*> platforms;
std::vector<Brick*> powerups;
std::vector<Particle*> particles;

// Prepare for simulation. Typically we use a time step of 1/60 of a
// second (60Hz) and ~10 iterations. This provides a high quality simulation
// in most game scenarios.
int32 velocityIterations = 8;
int32 positionIterations = 3;

float timeStep = 1.f / 60.f; //one 60th of a second
float elapsedTime = 0; //used for calculating time passed

//game state configuration
enum GameState { START = 0, PLAYING, GAMEOVER };
GameState gameState = START;

int playerLives = 3;
int playerScore = 0;
int levelNumber = 1;

//flags for operations
bool kickBall = false; //tracks whether we should apply a kick to the ball next update
bool stickToYourPlatform = true; //tracks whether we should move the ball with the platform
bool nextLevel = true; //tracks whether we should proceed to the next level
bool nextLevelAvailable = false; //tracks whether user can manually proceed
bool multipleBalls = false; //tracks whether we should apply "multiply balls" power-up
bool multiplePlatforms = false; //tracks whether we should put a second platform power-up
bool inclineLeft = false; //tracks the platform incline
bool inclineRight = false;

// erases existing instances of balls in the world and creates one
void EraseBalls()
{
	// cleaning
	for (int i = balls.size() - 1; i >= 0; --i)
	{
		world->DestroyBody(balls[i]->body);
		delete balls[i];
		balls.erase(balls.begin() + i);
	}
	balls.clear();

	// setting the ball "at the platform", as we will need to shoot it
	stickToYourPlatform = true;
	balls.push_back(MakeBall(sprites->ballSprites.at(0), 24.f, 50.f, 100.f,
		1.f, 0.0f, 1.2f, 0.f));
}

void ProceedToNextLevel()
{
	// remove the flags
	nextLevelAvailable = false;
	multiplePlatforms = false;
	platforms.at(1)->Destroy();

	std::string level = "Levels\\" + std::to_string(levelNumber);

	// there are only 3 levels and then the player moves to an "infinite" mode
	if (levelNumber <= 4)
	{
		level += ".lvl";
		levelNumber++;
	}
	else
	{
		level = "random";
	}

	for (int i = bricks.size() - 1; i >= 0; --i)
	{
		world->DestroyBody(bricks[i]->body);
		delete bricks[i];
		bricks.erase(bricks.begin() + i);
	}

	for (int i = powerups.size() - 1; i >= 0; --i)
	{
		world->DestroyBody(powerups[i]->body);
		delete powerups[i];
		powerups.erase(powerups.begin() + i);
	}

	EraseBalls();

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
	// from a pool and creates the ground box shape (also from a pool).adadada
	// The body is also added to the world.
	//TODO: Switch to a distinct class initialization to descreetly detect collision with the ground.

	b2Body* groundBody = world->CreateBody(&groundBodyDef);

	//an EdgeShape object, for the ground
	b2EdgeShape groundBox;

	// Define the ground as 4 edge shapes around the edge of the screen.
	b2FixtureDef boxShapeDef;

	boxShapeDef.filter.categoryBits = CMASK_EDGES;  //this is the edges/top
	boxShapeDef.filter.maskBits = CMASK_BALL;		//it collides wth balls

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
	blit3D->ShowCursor(false);

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

	contactListener = new ContactListener();
	world->SetContactListener(contactListener);

	sprites = new GameSprites(blit3D);

	Brick* thePlatform = MakePaddle(sprites->movingPlatformSprites);
	Brick* theDouble = MakePaddle(sprites->movingPlatformSprites);
	theDouble->Destroy();

	platforms.push_back(thePlatform);
	platforms.push_back(theDouble);

	for each (Brick * b in platforms) {
		b->typeID = ENTITY_PADDLE;
		b->body->SetBullet(true);
	}

	ProceedToNextLevel();
}

void DeInit(void)
{
	//Free all physics game data we allocated
	if (camera != NULL) delete camera;

	for each (Brick * b in platforms) delete b;
	for each (Brick * b in bricks) delete b;
	for each (Ball * b in balls) delete b;
	for each (Brick * p in powerups) delete p;

	delete world;
	//any sprites/fonts still allocated are freed automatcally by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	camera->Update((float)seconds);
	camera->PanTo(960, 540);

	switch (gameState)
	{
	case START:
	{
		camera->StopShaking();
	}
	break;

	case PLAYING:
	{
		if (seconds > 1.0 / 30) elapsedTime += 1.f / 30;
		else elapsedTime += (float)seconds;

		//waiting for the next level to happen
		nextLevel = true;
		for each (Brick * b in bricks)
		{
			if (b->body->IsEnabled() == true)
			{
				//postponing, if there are blocks left
				nextLevel = false;
			}
		}

		// if all balls were destroyed, the flag is raised and we move to the next level
		if (nextLevel)	ProceedToNextLevel();

		int entitiesLeft = 0;

		//updating the balls
		for (int i = balls.size() - 1; i >= 0; --i)
		{
			if (balls.at(i)->isDrawn)
			{
				// old way of determining whether we are below the platform/near ground;
				//TODO: Move to switch(typeID) tree and destroy on touching *the ground*
				if (balls.at(i)->body->GetPosition().y < 5)
				{
					balls.at(i)->Destroy();
				}
				else
				{
					// counting if there are any balls in the air
					// maybe should use: auto count = std::count_if(balls.begin(), balls.end(),[&](auto const& ball){ return ball->isDrawn == true; });
					entitiesLeft++;
				}

				if (multipleBalls) { // applying the power-up
					Ball* ball2 = MakeBall(sprites->ballSprites.at(0), 24.f, (balls.at(i)->body->GetPosition().x + 1) * PTM_RATIO, balls.at(i)->body->GetPosition().y * PTM_RATIO,
						1.f, 0.1f, 1.2f, 0.f);
					Ball* ball3 = MakeBall(sprites->ballSprites.at(0), 24.f, balls.at(i)->body->GetPosition().x * PTM_RATIO, (balls.at(i)->body->GetPosition().y + 1) * PTM_RATIO,
						1.f, 0.1f, 1.2f, 0.f);

					ball2->body->SetLinearVelocity(balls.at(i)->body->GetLinearVelocity());
					ball3->body->SetLinearVelocity(balls.at(i)->body->GetLinearVelocity());

					KickBall(ball2);
					KickBall(ball3);

					balls.push_back(ball2);
					balls.push_back(ball3);
				}

				//check if we are not moving enough vertically
				b2Vec2 dir = balls.at(i)->body->GetLinearVelocity();
				float minAmountY = 30.f / PTM_RATIO;

				// if less than minimum, set to minimum
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
				dir *= balls.at(i)->body->GetLinearVelocity().Length(); //scale up the velocity tp correct ball speed
				balls.at(i)->body->SetLinearVelocity(dir);
			}
		}

		// the power-up was applied
		multipleBalls = false;

		// if no balls are in the air, remove all unrendered balls + create one
		// also, remove one life, as it would mean the player dropped every ball that has been created until now
		if (entitiesLeft == 0)
		{
			playerLives--;
			EraseBalls();
		}

		// if ball has been respawned, move it with the platform
		if (stickToYourPlatform) balls.at(0)->body->SetTransform(b2Vec2(cursor.x / PTM_RATIO, 35.7), 0);

		// if no lives left, Game Over
		if (playerLives < 0) gameState = GAMEOVER;

		//don't apply physics unless at least a timestep worth of time has passed
		while (elapsedTime >= timeStep)
		{
			// update passed time counter
			elapsedTime -= timeStep;

			//update the physics world
			world->Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces.
			world->ClearForces();

			// Move the paddle to where it belongs
			platforms.at(0)->MatchCursor(cursor.x, inclineLeft, inclineRight);
			if (multiplePlatforms)
			{
				platforms.at(1)->MatchCursor(blit3D->screenWidth - cursor.x, inclineLeft, inclineRight, true);
			}
			//update the particle list and remove dead particles
			for (int i = particles.size() - 1; i >= 0; --i)
			{
				if (particles[i]->Update(timeStep))
				{
					//time to die!
					delete particles[i];
					particles.erase(particles.begin() + i);
				}
			}

			//loop over contacts
			for (int pos = 0; pos < contactListener->contacts.size(); ++pos)
			{
				Contact contact = contactListener->contacts[pos];

				//fetch the entities from the body userdata
				Entity* A = (Entity*)contact.fixtureB->GetBody()->GetUserData().pointer;
				Entity* B = (Entity*)contact.fixtureA->GetBody()->GetUserData().pointer;

				if (A != NULL && B != NULL) //if there is an entity for these objects...
				{
					if (A->typeID == ENTITY_BALL)
					{
						//swap A and B
						Entity* C = A;
						A = B;
						B = C;
					}

					if (B->typeID == ENTITY_BALL && A->typeID == ENTITY_BRICK)
					{
						Brick* b = (Brick*)A;

						//update the brick
						b->Destroy();

						//shake the screen
						camera->Shake(10.f * (b->spriteNumber + 1));

						//increase the score
						playerScore += (int)10 / (b->spriteNumber + 1);

						if (b->DropPowerUp())
						{
							int power;
							if (dice.Roll1DN(100) > dice.Roll1DN(100))
							{
								power = 0;
							}
							else if (dice.Roll1DN(100) > dice.Roll1DN(70)) {
								power = 1;
							}
							else {
								power = 2;
							}
							Brick* pUp = MakePowerUp(sprites->mossyHorizontalShortPlatformSprites, power, b, dice);
							powerups.push_back(pUp);
						};

						//add a particle effect
						Particle* p = new Particle();
						p->spriteList = sprites->collisionParticles;
						p->rotationSpeed = 0;
						p->angle = 0;

						p->totalTimeToLive = 0.2f;

						//get coords of contact
						p->coords = Physics2Pixels(contact.contactPoint);

						particles.push_back(p);
					}

					if (B->typeID == ENTITY_BALL && A->typeID == ENTITY_GROUND) {}

					//check and see if paddle or ground hit a powerup
					if (A->typeID == ENTITY_POWERUP)
					{
						//swap A and B
						Entity* C = A;
						A = B;
						B = C;
					}

					if (B->typeID == ENTITY_POWERUP)
					{
						if (A->typeID == ENTITY_PADDLE)
						{
							//shake the screen!
							camera->Shake(5);

							Brick* p = (Brick*)B;
							p->Destroy();
							switch (p->powerUpNumber)
							{
							case 0:
							{
								// that would mean that we have already fired our 'main' ball,
								// which would prevent us from having "you two go & you stay here" situations
								if (stickToYourPlatform == false) multipleBalls = true;
							}
							break;
							case 1:
							{
								multiplePlatforms = true;
								platforms.at(1)->Enable();
							}
							break;
							case 2:
							{
								// disable multiple platforms - first, because this powerup is quite OP
								// and second, platform doesn't make much difference anyway
								platforms.at(1)->Destroy();
								multiplePlatforms = false;

								//shake the screen a lot!!!!
								camera->Shake(250.f);
								for each (Brick * b in bricks)
								{
									b->Destroy();
									b->body->SetLinearVelocity(b2Vec2(0, 0));

									//increase the score
									playerScore += (int)9 / (b->spriteNumber + 1);
								}
							}
							default:
								break;
							}
						}
					}
				} //end of checking if they are both NULL userdata
			} //end of collison handling
		}
	}

	for (int i = balls.size() - 1; i >= 0; --i)
	{
		if (!balls[i]->isDrawn) {
			world->DestroyBody(balls[i]->body);
			delete balls[i];
			balls.erase(balls.begin() + i);
		}
	}

	for (int i = powerups.size() - 1; i >= 0; --i)
	{
		if (!powerups[i]->isDrawn || powerups[i]->body->GetPosition().y < 0) {
			world->DestroyBody(powerups[i]->body);
			delete powerups[i];
			powerups.erase(powerups.begin() + i);
		}
	}

	break; //end case PLAYING

	case GAMEOVER:
		// cleaning may be unnecessary?
		EraseBalls();
	default:
		break;
	}
	//end switch(gameState)
}

void Draw(void)
{
	glClearColor(0.8f, 0.9f, 0.9f, 0.0f);	//clear colour: r,g,b,a
	// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//transform the view by "drawing" the camera
	camera->Draw();

	switch (gameState)
	{
	case START:
	{
		std::string lives = "Control paddle with mouse";
		debugFont->BlitText(100, 100, lives);

		std::string score = "Tilt the paddle with \"A\" and \"D\"";
		debugFont->BlitText(100, 70, score);

		sprites->intro->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		std::string proceed = "Press \"Enter\" to proceed";
		debugFont->BlitText(1500, 70, proceed);
	}
	break;
	case PLAYING:
	{
		//loop over all entities and draw them
		for each (auto & b in balls) b->Draw();
		for each (auto & p in platforms) p->Draw();
		for each (auto & p in powerups) p->Draw();
		for each (auto & b in bricks) b->Draw();
		for each (auto & p in particles) p->Draw();

		if (nextLevelAvailable)
		{
			std::string proceed = "Press \"Enter\" to proceed to the next level";
			debugFont->BlitText(1400, 70, proceed);
		}

		std::string lives = "Lives: " + std::to_string(playerLives);
		debugFont->BlitText(100, 100, lives);

		std::string score = "Score: " + std::to_string(playerScore);
		debugFont->BlitText(100, 70, score);
	}
	break;
	case GAMEOVER:
	{
		sprites->outro->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		std::string score = "~ " + std::to_string(playerScore) + " ~";
		debugFont->BlitText(blit3D->screenWidth / 2 - 50, 650, score);
	}
	break;
	}

	camera->UnDraw(); //turns off the camera transformation of the view coords
}

void DoInput(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && gameState == START)
	{
		gameState = PLAYING;
	}

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && nextLevelAvailable)
	{
		ProceedToNextLevel();
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && stickToYourPlatform == true)
	{
		stickToYourPlatform = false;
		// magic numbers are bad, but the flag is only set
		// when I have just created a new ball
		KickBall(balls.at(0));
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		inclineLeft = true;
	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		inclineLeft = false;

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		inclineRight = true;
	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		inclineRight = false;
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