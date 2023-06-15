#include "Ball.h"
#include "DieRoller.h"

extern std::mt19937 rng;
std::uniform_int_distribution<> randomDegrees(30, 150);
extern Blit3D* blit3D;
extern b2World* world; //our physics engine

Ball::Ball(Sprite* theSprite, float diameter, float xpos, float ypos,
	float density, float friction, float restitution,
	float angularDamping)
{
	sprite = theSprite;
	// Define the dynamic Ball body.
	//We set its position and call the body factory.
	b2BodyDef BallBodyDef;
	BallBodyDef.type = b2_dynamicBody; //make it a dynamic body i.e. one moved by the physics engine
	BallBodyDef.position.Set(xpos / PTM_RATIO, ypos / PTM_RATIO); //set its position in the world

	//friction won't work on a rolling circle in Box2D, so apply angular damping to the body
	//to make it slow down as it rolls
	BallBodyDef.angularDamping = angularDamping;

	body = world->CreateBody(&BallBodyDef); //create the body and add it to the world

	// Define a ball shape for our dynamic body.
	//A circle shape for our ball
	b2CircleShape dynamicBall;
	dynamicBall.m_radius = diameter / (2 * PTM_RATIO); //ball has diameter ? pixels

	//create the fixture definition - we don't need to save this
	b2FixtureDef fixtureDef;

	// Define the dynamic body fixture.
	fixtureDef.shape = &dynamicBall;

	// Set the ball density to be non-zero, so it will be dynamic.
	//Remember, density * area determines the mass of the object
	fixtureDef.density = density;

	// Override the default friction.
	fixtureDef.friction = friction;

	//restitution makes us bounce; use 0 for no bounce, 1 for perfect bounce
	fixtureDef.restitution = restitution;

	// Add the shape to the body.
	body->CreateFixture(&fixtureDef);
}

void Ball::Draw()
{
	//update the sprite locations based on the physics objects
	b2Vec2 bposition = body->GetPosition();
	bposition = Physics2Pixels(bposition);

	//get the angle of the ball
	float angle = body->GetAngle();
	angle = glm::degrees(angle);

	// Draw the objects
	sprite->angle = angle;
	sprite->Blit(bposition.x, bposition.y);
}

void KickBall(Ball* ball)
{
	//kick the ball in a random direction

	b2Vec2 dir = deg2vec(ball->body->GetAngle() + randomDegrees(rng));

	//make the ball move
	ball->body->SetLinearVelocity(b2Vec2(0.f, 0.f)); //remove all current velocity
	dir *= 150.f * PTM_RATIO; //scale up the force
	ball->body->ApplyLinearImpulse(dir, ball->body->GetPosition(), true); //apply the "kick"
}

std::vector<Ball*> MultilplyToThree(Ball* ball, std::vector<Ball*> balls)
{

}


void KickBallUpAndToTheSide(Ball* ball)
{
	//kick the ball in a random direction
	b2Vec2 dir;
	if (ball->body->GetLinearVelocity().Length() > 10)
	{	
		if (vec2deg(ball->body->GetLinearVelocity()) > -150 && vec2deg(ball->body->GetLinearVelocity()) < -90) {
			dir = deg2vec(-90 - vec2deg(ball->body->GetLinearVelocity()) + randomDegrees(rng));
		}
		else if (vec2deg(ball->body->GetLinearVelocity()) > -90 && vec2deg(ball->body->GetLinearVelocity()) < -30)
		{
			dir = deg2vec(90 - vec2deg(ball->body->GetLinearVelocity()) - randomDegrees(rng));
		}
		else if (vec2deg(ball->body->GetLinearVelocity()) > 30 && vec2deg(ball->body->GetLinearVelocity()) < 90)
		{
			dir = deg2vec(-90 - vec2deg(ball->body->GetLinearVelocity()) + randomDegrees(rng));
		}
		else if (vec2deg(ball->body->GetLinearVelocity()) > 90 && vec2deg(ball->body->GetLinearVelocity()) < 150)
		{
			dir = deg2vec(90 - vec2deg(ball->body->GetLinearVelocity()) - randomDegrees(rng));
		}
		else
		{
			dir = deg2vec(-vec2deg(ball->body->GetLinearVelocity()) + randomDegrees(rng));
		}

		dir *= ball->body->GetLinearVelocity().Length() * 60.f * PTM_RATIO; //scale up the force
	}
	else
	{
		dir = deg2vec(110);
		dir *= 150.f * PTM_RATIO; //scale up the force
	}

	//make the ball move
	ball->body->SetLinearVelocity(b2Vec2(0.f, 0.f)); //remove all current velocity
	ball->body->ApplyLinearImpulse(dir, ball->body->GetPosition(), true); //apply the "kick"
}