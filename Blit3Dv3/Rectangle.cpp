#include "Rectangle.h"
#include "DieRoller.h"

extern Blit3D* blit3D;
extern b2World* world; //our physics engine

Brick::Brick(std::vector<Sprite*> spriteList, float width, float height, float xpos, float ypos, float density, float friction, float restitution)
{
	sprites = spriteList;

	// Define the dynamic Rectangle body.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(xpos / PTM_RATIO, ypos / PTM_RATIO);
	bodyDef.angularDamping = 3.f;

	body = world->CreateBody(&bodyDef);

	// Define a rectangle shape for the body.
	b2PolygonShape dynamicRectangle;
	dynamicRectangle.SetAsBox(width / (2 * PTM_RATIO), height / (2 * PTM_RATIO));

	// Create the fixture definition.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicRectangle;
	fixtureDef.density = density;
	fixtureDef.friction = friction;
	fixtureDef.restitution = restitution;
	// Add the shape to the body.
	body->CreateFixture(&fixtureDef);
}

void Brick::Draw()
{
	if (isDrawed) {
		Sprite* sprite = sprites.at(spriteNumber);

		b2Vec2 position = body->GetPosition();
		position = Physics2Pixels(position);

		float angle = body->GetAngle();
		angle = glm::degrees(angle);

		// Draw the object
		sprite->angle = angle;
		sprite->Blit(position.x, position.y);
	}
}

void Brick::Destroy() {
	if (spriteNumber + 1 >= sprites.size()) 
	{
		body->SetEnabled(false);
		isDrawed = false;
	}
	else 
	{
		spriteNumber++;
	}
}

Brick* AddBrick(int x, int y, int brickType, GameSprites* sprites)
{
	const int gridWidth = 20;
	const int gridHeight = 25;

	x = 96 + 77 * x;
	y = 1080 - 60 - 35 * y;

	Brick* brick = NULL;
	switch (brickType) {
	case 0:
	default:
		break;
	case 1:
	{
		brick = new Brick(sprites->levelOnePlatformSprites, 72, 30, x, y, 300.f, 0.1f, 1.5f);
		break;
	}
	case 2:
	{
		brick = new Brick(sprites->levelTwoPlatformSprites, 72, 30, x, y, 300.f, 0.1f, 1.f);
		break;
	}
	case 3:
	{
		brick = new Brick(sprites->levelThreePlatformSprites, 72, 30, x, y, 400.f, 0.1f, 1.f);
		break;
	}
	case 4:
	{
		brick = new Brick(sprites->levelFourPlatformSprites, 72, 30, x, y, 500.f, 0.1f, 1.f);
		break;
	}
	case 5:
	{
		brick = new Brick(sprites->levelFivePlatformSprites, 72, 30, x, y, 600.f, 0.1f, 1.f);
		break;
	}
	}

	return brick;
}

std::vector<Brick*> ReadBricksFromFile(std::string filename, GameSprites* sprites)
{
	std::vector<Brick*> bricks;
	Brick* brick;
	int width = 23;
	int height = 10;

	if (filename == "random")
	{
		//distribution for creating a map
		DiceRoller dice;

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				//AddBrick(x + y * x, dice.Roll1DN(6) - 1, bricks, sprites);
				brick = AddBrick(x, y, dice.Roll1DN(6) - 1, sprites);
				if (brick != NULL)
				{
					bricks.push_back(brick);
				}
			}
		}

		return bricks;
	}
	else
	{
		std::ifstream mapfile(filename);

		if (!mapfile.is_open())
		{
			std::cout << "Can't open map file: " << filename << std::endl;
			return bricks;
		}

		int brickType;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				mapfile >> brickType;
				brick = AddBrick(x, y, brickType, sprites);
				if (brick != NULL)
				{
					bricks.push_back(brick);
				}
			}
		}

		mapfile.close();
	}
	return bricks;
}