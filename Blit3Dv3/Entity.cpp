#include "Entity.h"

Entity::Entity()
{
	sprite = NULL;
	body = NULL;
	typeID = ENTITY_NONE; //default entity type
}

Entity::~Entity()
{

};

void Entity::Draw()
{
	position = body->GetPosition();
	position = Physics2Pixels(position);

	sprite->angle = rad2deg(body->GetAngle());

	// Draw the objects
	sprite->Blit(position.x, position.y);
}

void Entity::Update(float seconds)
{
	//empty, provide one if you have something to update, like animation or game logic
}