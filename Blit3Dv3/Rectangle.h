#pragma once

#include "Blit3D.h"

#include <random>
#include "Physics.h"
#include "GameSprites.h"

class Brick
{
public:
    b2Body* body = NULL;

    std::vector<Sprite*> sprites;
    int spriteNumber = 0;
    boolean isDrawed = true;

    Brick(std::vector<Sprite*> spriteList, float width, float height,
        float xpos, float ypos,
        float density, float friction, float restitution);
    void Draw();
    void Destroy();
};

std::vector<Brick*> ReadBricksFromFile(std::string filename, GameSprites* sprites);
Brick* AddBrick(int x, int y, int brickType, GameSprites* sprites);