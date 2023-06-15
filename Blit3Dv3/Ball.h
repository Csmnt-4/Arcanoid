#pragma once

#include "Blit3D.h"

#include <random>
#include "Physics.h"

class Ball
{
public:
	b2Body* body = NULL; //the physics body for the ball
	Sprite* sprite = NULL;

	Ball(Sprite* theSprite, float diameter, 
		float xpos, float ypos,
		float density = 1.f, float friction = 0.5f, float restitution = 0.5f,
		float angularDamping = 1.8f);
	void Draw();
};

void KickBall(Ball* ball);
void KickBallUpAndToTheSide(Ball* ball);
