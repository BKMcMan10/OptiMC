#include "gameplay/HeartManager.h"

namespace Minecraft
{
	namespace HeartManager
	{
		  
		static std::array<glm::vec2, Player::numHeartColums> heartPositions;

		static const Sprite* heartSpriteFull = nullptr;
		static const Sprite* heartSpriteHalf = nullptr;
		static const Sprite* heartSpriteNone = nullptr;

		float health = 10;

		static const glm::vec2 heartSize = glm::vec2(0.098f, 0.098f);
		const float heartOffset = 0.0975f;	

		void Init()
		{

			const robin_hood::unordered_map<std::string, Sprite>& menuSprites = Sprites::getSpritesheet("assets/images/hudSpritesheet.yaml");
			heartSpriteFull = &menuSprites.at(std::string("heartFull"));
			heartSpriteHalf = &menuSprites.at(std::string("heartHalf"));
			heartSpriteNone = &menuSprites.at(std::string("heartNone"));

		}

		void Update()
		{
			for (int i = 0; i < Player::numHeartColums; i++)
			{
				if (i < health)
				{
					Renderer::drawTexture2D(*heartSpriteFull, glm::vec2{ -0.9 + (float)heartOffset * (float)i, -1.26 }, heartSize, Styles::defaultStyle);
				}
				else
				{
					Renderer::drawTexture2D(*heartSpriteNone, glm::vec2{ -0.9 + (float)heartOffset * (float)i, -1.26 }, heartSize, Styles::defaultStyle);
				}
			}
			
			
		}

		void addHearts(float amount)
		{
			Player::numHeartColums + amount;
		}

		void removeHearts(float amount)
		{
			Player::numHeartColums - amount;
		}
	}
}