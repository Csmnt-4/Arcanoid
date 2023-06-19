#include "GameSprites.h"

GameSprites::GameSprites(Blit3D* blit3D)
{
	int i;

	ballSprites.push_back(blit3D->MakeSprite(3, 39, 24, 24, "Media\\bricks.png"));

	levelOnePlatformSprites.push_back(blit3D->MakeSprite(3, 3, 72, 30, "Media\\bricks.png"));

	for (i = 0; i < 2; i++)
	{
		levelTwoPlatformSprites.push_back(blit3D->MakeSprite(81, 36 * i + 3, 72, 30, "Media\\bricks.png"));
	}

	for (i = 0; i < 3; i++)
	{
		levelThreePlatformSprites.push_back(blit3D->MakeSprite(159, 36 * i + 3, 72, 30, "Media\\bricks.png"));
	}

	for (i = 0; i < 4; i++)
	{
		levelFourPlatformSprites.push_back(blit3D->MakeSprite(237, 36 * i + 3, 72, 30, "Media\\bricks.png"));
	}

	for (i = 0; i < 5; i++)
	{
		levelFivePlatformSprites.push_back(blit3D->MakeSprite(315, 36 * i + 3, 72, 30, "Media\\bricks.png"));
	}

	movingPlatformSprites.push_back(blit3D->MakeSprite(393, 3, 108, 30, "Media\\bricks.png"));

	for (i = 0; i < 3; i++)
	{
		mossyHorizontalShortPlatformSprites.push_back(blit3D->MakeSprite(507, 36 * i + 3, 36, 30, "Media\\bricks.png"));
	}

	for (i = 0; i < 1; i++)
	{
		mossyHorizontalMediumPlatformSprites.push_back(blit3D->MakeSprite(393, 36 * i + 111, 108, 30, "Media\\bricks.png"));
	}

	for (i = 0; i < 1; i++)
	{
		mossyHorizontalLongPlatformSprites.push_back(blit3D->MakeSprite(393, 36 * i + 147, 144, 30, "Media\\bricks.png"));
	}

	for (i = 0; i < 8; i++)
	{
		collisionParticles.push_back(blit3D->MakeSprite(400 * i, 0 , 400, 400, "Media\\Explosion\\explosion.png"));
	}

	intro = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\intro.png");
	outro = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\outro.png");
}