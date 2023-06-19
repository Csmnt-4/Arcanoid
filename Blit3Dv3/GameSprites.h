#pragma once

#include "Blit3D.h"

// Credits:
// Original blocks sprites - Pavel Kutejnikov
// Original explosion VFX - CodeManu

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

	std::vector<Sprite*> collisionParticles;

	Sprite* intro;
	Sprite* outro;

	GameSprites(Blit3D* blit3D);
};