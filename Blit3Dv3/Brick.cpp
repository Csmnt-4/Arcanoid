#include "Brick.h"
#include "CollisionMasks.h"

extern Blit3D* blit3D;
extern b2World* world; //physics engine

void Brick::Destroy()
{
	body->SetLinearVelocity(b2Vec2(0.f, 0.f));
	if (spriteNumber + 1 >= sprites.size())
	{
		body->SetEnabled(false);
		isDrawn = false;
	}
	else
	{
		spriteNumber++;
	}
}

void Brick::Enable()
{
	body->SetEnabled(true);
	isDrawn = true;
	spriteNumber = 0;
}

//TODO: Create an Update for Bricks to simplify setting the angle;

Brick* MakeBrick(std::vector<Sprite*> spriteList, float width, float height, float xpos, float ypos, float density, float friction, float restitution, short categoryBits, short maskBits)
{
	Brick* brick = new Brick();
	brick->typeID = ENTITY_BRICK;
	brick->sprites = spriteList;

	// Define the dynamic Rectangle body.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(xpos / PTM_RATIO, ypos / PTM_RATIO);
	bodyDef.angularDamping = 3.f;

	//make the userdata point back to this entity
	bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(brick);

	brick->body = world->CreateBody(&bodyDef);

	// Define a rectangle shape for the body.
	b2PolygonShape dynamicRectangle;
	dynamicRectangle.SetAsBox(width / (2 * PTM_RATIO), height / (2 * PTM_RATIO));

	// Create the fixture definition.
	b2FixtureDef fixtureDef;

	fixtureDef.filter.categoryBits = categoryBits;
	fixtureDef.filter.maskBits = maskBits;

	fixtureDef.shape = &dynamicRectangle;
	fixtureDef.density = density;
	fixtureDef.friction = friction;
	fixtureDef.restitution = restitution;
	// Add the shape to the body.
	brick->body->CreateFixture(&fixtureDef);

	return brick;
}

void Brick::Draw()
{
	if (isDrawn) {
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

void Brick::MatchCursor(float x, bool inclineLeft, bool inclineRight)
{
	if (inclineLeft)
	{
		body->SetTransform(b2Vec2(x / PTM_RATIO, 30), 0.5);
	}
	else if (inclineRight)
	{
		body->SetTransform(b2Vec2(x / PTM_RATIO, 30), -0.5);
	}
	else
	{
		body->SetTransform(b2Vec2(x / PTM_RATIO, 30), 0);
	}
}

void Brick::MatchCursor(float x, bool inclineLeft, bool inclineRight, bool isOffset)
{
	if (isOffset) {
		if (inclineLeft)
		{
			body->SetTransform(b2Vec2(x / PTM_RATIO, 30), -0.5);
		}
		else if (inclineRight)
		{
			body->SetTransform(b2Vec2(x / PTM_RATIO, 30), 0.5);
		}
		else
		{
			body->SetTransform(b2Vec2(x / PTM_RATIO, 30), 0);
		}
	}

	else

	{
	}
}

bool Brick::DropPowerUp()
{
	DiceRoller dice;
	switch (spriteNumber)
	{
	case 0:
		return dice.Roll1DN(20) > dice.Roll1DN(100);
	case 1:
		return dice.Roll1DN(25) > dice.Roll1DN(100);
	case 2:
	case 3:
		return dice.Roll1DN(30) > dice.Roll1DN(90);
	case 4:
		return dice.Roll1DN(50) > dice.Roll1DN(90);
	default:
		break;
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
		brick = MakeBrick(sprites->levelOnePlatformSprites, 72, 30, x, y, 300.f, 0.1f, 1.5f, CMASK_BRICK, CMASK_BALL);
		break;
	}
	case 2:
	{
		brick = MakeBrick(sprites->levelTwoPlatformSprites, 72, 30, x, y, 300.f, 0.1f, 1.f, CMASK_BRICK, CMASK_BALL);
		break;
	}
	case 3:
	{
		brick = MakeBrick(sprites->levelThreePlatformSprites, 72, 30, x, y, 400.f, 0.1f, 1.f, CMASK_BRICK, CMASK_BALL);
		break;
	}
	case 4:
	{
		brick = MakeBrick(sprites->levelFourPlatformSprites, 72, 30, x, y, 500.f, 0.1f, 1.f, CMASK_BRICK, CMASK_BALL);
		break;
	}
	case 5:
	{
		brick = MakeBrick(sprites->levelFivePlatformSprites, 72, 30, x, y, 600.f, 0.1f, 1.f, CMASK_BRICK, CMASK_BALL);
		break;
	}
	}
	return brick;
}

Brick* MakePowerUp(std::vector<Sprite*> spriteList, int power, Brick* b, DiceRoller dice)
{
	Brick* brick = new Brick();
	brick->typeID = ENTITY_POWERUP;

	std::vector<Sprite*> oneSpriteList;
	oneSpriteList.push_back(spriteList.at(power));
	brick->sprites = oneSpriteList;

	// Define the dynamic Rectangle body.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(b->body->GetPosition().x, b->body->GetPosition().y);
	bodyDef.angularDamping = 0.f;

	//make the userdata point back to this entity
	bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(brick);

	brick->body = world->CreateBody(&bodyDef);

	// Define a rectangle shape for the body.
	b2PolygonShape dynamicRectangle;
	dynamicRectangle.SetAsBox(36 / (2 * PTM_RATIO), 30 / (2 * PTM_RATIO));

	// Create the fixture definition.
	b2FixtureDef fixtureDef;

	fixtureDef.filter.categoryBits = CMASK_POWERUP;
	fixtureDef.filter.maskBits = CMASK_PADDLE;

	fixtureDef.shape = &dynamicRectangle;
	fixtureDef.density = 10.f;
	fixtureDef.friction = 1.f;
	fixtureDef.restitution = 1.f;
	// Add the shape to the body.

	brick->powerUpNumber = power;

	brick->body->CreateFixture(&fixtureDef);
	brick->body->SetAngularVelocity((float)(dice.Roll1DN(80) - 40) / 100);
	brick->body->SetLinearVelocity(b2Vec2(0, -20));

	return brick;
}

Brick* MakePaddle(std::vector<Sprite*> spriteList)
{
	Brick* brick = new Brick();
	brick->typeID = ENTITY_PADDLE;
	brick->sprites = spriteList;

	// Define the dynamic Rectangle body.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(960, 300);
	bodyDef.angularDamping = 3.f;

	//make the userdata point back to this entity
	bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(brick);

	brick->body = world->CreateBody(&bodyDef);

	// Define a rectangle shape for the body.
	b2PolygonShape dynamicRectangle;
	dynamicRectangle.SetAsBox(108 / (2 * PTM_RATIO), 30 / (2 * PTM_RATIO));

	// Create the fixture definition.
	b2FixtureDef fixtureDef;

	fixtureDef.filter.categoryBits = CMASK_PADDLE;
	fixtureDef.filter.maskBits = CMASK_BALL | CMASK_POWERUP;

	fixtureDef.shape = &dynamicRectangle;
	fixtureDef.density = 200.f;
	fixtureDef.friction = 0.f;
	fixtureDef.restitution = 1.2f;
	// Add the shape to the body.

	brick->body->CreateFixture(&fixtureDef);

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