#pragma once

#include "Blit3D.h"

class GameSprites
{
public:
	std::vector<Sprite*> ballSprites;
	std::vector<Sprite*> levelOnePlatformSprites;
	std::vector<Sprite*> levelTwoPlatformSprites;
	std::vector<Sprite*> levelThreePlatformSprites;
	std::vector<Sprite*> levelFourPlatformSprites;
	std::vector<Sprite*> levelFivePlatformSprites;

	std::vector<Sprite*> movingPlatformSprites;

	std::vector<Sprite*> mossyHorizontalShortPlatformSprites;
	std::vector<Sprite*> mossyHorizontalMediumPlatformSprites;
	std::vector<Sprite*> mossyHorizontalLongPlatformSprites;

	std::vector<Sprite*> mossyVerticalLongPlatformSprites;

	GameSprites(Blit3D* blit3D);
};