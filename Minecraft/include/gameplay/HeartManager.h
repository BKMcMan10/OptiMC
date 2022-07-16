#ifndef MINECRAFT_HEART_MANAGER
#define MINECRAFT_HEART_MANAGER

#include "gui/MainHud.h"
#include "gui/Gui.h"
#include "gui/GuiElements.h"
#include "renderer/Sprites.h"
#include "renderer/Renderer.h"
#include "core/Ecs.h"
#include "renderer/Styles.h"
#include "utils/Constants.h"

namespace Minecraft
{

	struct Heart;

	namespace HeartManager
	{
		void Init();

		void Update();

		//void addHearts(float amount);
		//void removeHearts(float amount);

		extern bool isAtFullHealth;

		extern float health;
	}
}

#endif