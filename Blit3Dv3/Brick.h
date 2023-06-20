#pragma once
#include "Blit3D.h"

#include <random>
#include "Physics.h"
#include "GameSprites.h"
#include "Entity.h"
#include "DieRoller.h"

class Brick : public Entity
{
public:
	b2Body* body = NULL; //the physics body for the ball
	std::vector<Sprite*> sprites;

	bool isDrawn = true;
	int powerUpNumber = -1;

	int spriteNumber = 0;

	Brick()
	{
		typeID = ENTITY_BRICK;
	};

	void Draw();
	void Enable();
	void Destroy();

	void MatchCursor(float x, bool inclineLeft, bool inclineRight);
	void MatchCursor(float x, bool inclineLeft, bool inclineRight, bool isOffset);
	bool DropPowerUp();
};

Brick* MakeBrick(std::vector<Sprite*> spriteList, float width, float height, float xpos, float ypos, float density, float friction, float restitution, short categoryBits, short maskBits);
std::vector<Brick*> ReadBricksFromFile(std::string filename, GameSprites* sprites);
Brick* MakePowerUp(std::vector<Sprite*> spriteList, int power, Brick* b, DiceRoller dice);
Brick* MakePaddle(std::vector<Sprite*> spriteList);


Brick* AddBrick(int x, int y, int brickType, GameSprites* sprites);
