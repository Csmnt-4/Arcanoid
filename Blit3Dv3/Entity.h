#pragma once
#pragma once

#include "Blit3D.h"

#include "Physics.h"
#include "Types.h"

//a simple entity class that connects the physics object and the render object
class Entity
{
public:
	Sprite* sprite;
	b2Body* body;
	b2Vec2 position; //used to calculate the position of the object
	EntityType typeID;

	Entity();
	virtual ~Entity(); //always make base class destructor virtual
	virtual void Draw();
	virtual void Update(float seconds);
};