#pragma once

#include "Blit3D.h"

#include <random>

#include "Physics.h"
#include "DieRoller.h"
#include "GameSprites.h"
#include "Entity.h"

class Ball : public Entity
{
public:
	b2Body* body = NULL; //the physics body for the ball
	Sprite* sprite = NULL;

	boolean isDrawn = true;

	Ball()
	{
		typeID = ENTITY_BALL;
	};
	void Draw();
	void Destroy();
};

Ball* MakeBall(Sprite* theSprite, float diameter,
	float xpos, float ypos,
	float density, float friction, float restitution,
	float angularDamping);
void KickBall(Ball* ball);
void KickBallUpAndToTheSide(Ball* ball);
std::vector<Ball*> MultilplyToThree(std::vector<Ball*> balls, GameSprites* sprites);