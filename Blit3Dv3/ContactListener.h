#pragma once
/*
A contact listener class for handling collisions between fixtures in Box2D.
Credit: Darren Reid
*/
#include <Box2D/Box2D.h>
#include <vector>
#include <algorithm>

struct Contact
{
    b2Fixture* fixtureA; //the two fixtures that collided
    b2Fixture* fixtureB;
    float maxImpulseAB; //strength (force) of the contact, determined by the maximum impulse
    b2Vec2 contactPoint; //where the contact occured in the world
    b2Vec2 normal; //normal pointing from A to B for the collision

    bool operator==(const Contact& other) const
    {
        if (((fixtureA == other.fixtureA) && (fixtureB == other.fixtureB)) ||
            ((fixtureA == other.fixtureB) && (fixtureB == other.fixtureA)))
            return true;
        else return false;
    }
};

class ContactListener : public b2ContactListener
{

public:
    std::vector<Contact>contacts;
    std::vector<Contact>::iterator pos;

    ContactListener();
    ~ContactListener();

    virtual void BeginContact(b2Contact* contact);
    virtual void EndContact(b2Contact* contact);
    virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
    virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);

};
