#include "gameplay/PlayerController.h"
#include "input/Input.h"
#include "input/KeyBindings.h"
#include "core/Ecs.h"
#include "core/Components.h"
#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Styles.h"
#include "renderer/Sprites.h"
#include "utils/DebugStats.h"
#include "utils/CMath.h"
#include "physics/PhysicsComponents.h"
#include "physics/Physics.h"
#include "gameplay/CharacterController.h"
#include "gameplay/Inventory.h"
#include "gameplay/CommandLine.h"
#include "world/ChunkManager.h"
#include "world/BlockMap.h"
#include "gui/MainHud.h"
#include "core/Application.h"
#include "core/Scene.h"
#include "core/Window.h"
#include "network/Network.h"
#include "gameplay/HeartManager.h"
#include <math.h>
#include "Sounds/SoundDevice.h"
#include "Sounds/SoundBuffer.h"
#include "Sounds/SoundSource.h"
#include "utils/Constants.h"

namespace Minecraft
{
	enum class GameMode : uint8
	{
		None,
		Adventure,
		Survival,
		Creative,
		Spectator
	};

	enum class CubemapSide : uint8
	{
		Left = 0,
		Right = 1,
		Front = 2,
		Back = 3,
		Top = 4,
		Bottom = 5,
		Length
	};

	namespace PlayerController
	{
		bool generateCubemap = false;
		static CubemapSide sideGenerating = CubemapSide::Left;

		// Internal members
		static Ecs::EntityId playerId;
		static Style blockHighlight;
		static GameMode gameMode;
		static float blockPlaceDebounceTime = 0.2f;
		static float blockPlaceDebounce = 0.0f;

		static const TextureFormat* sideSprite;
		static const TextureFormat* topSprite;
		static const TextureFormat* bottomSprite;
		static const TextureFormat* side;
		//SoundSource mySpeaker;
		//uint32_t testSFX;
		// Internal functions
		static void updateSurvival(Transform& transform, CharacterController& controller, Rigidbody& rb, Inventory& inventory);
		static void updateCreative(Transform& transform, CharacterController& controller, Rigidbody& rb, Inventory& inventory);
		static void updateSpectator(Transform& transform, CharacterController& controller, Rigidbody& rb);
		static void updateInventory(Inventory& inventory);

		static Model stick;

		void init()
		{
			//testSFX = SoundBuffer::get()->addSoundEffect("assets/sounds/blocks/spell.ogg");
			blockHighlight = Styles::defaultStyle;
			blockHighlight.color = "#00000067"_hex;
			blockHighlight.strokeWidth = 0.01f;
			gameMode = GameMode::Survival;
			playerId = Ecs::nullEntity;

			sideSprite = &BlockMap::getTextureFormat("oak_log");
			topSprite = &BlockMap::getTextureFormat("oak_log_top");
			bottomSprite = &BlockMap::getTextureFormat("oak_log_top");
			side = &BlockMap::getTextureFormat("destroy_stage_1");

			stick = Vertices::getItemModel("stick");
		}

		void update(Ecs::Registry& registry)
		{
			setPlayerIfNeeded();

			/*if (Input::isKeyPressed(GLFW_KEY_W))
			{
				mySpeaker.Play(testSFX);
			}*/

			if (playerId != Ecs::nullEntity && registry.hasComponent<Transform>(playerId) && registry.hasComponent<CharacterController>(playerId)
				&& registry.hasComponent<Rigidbody>(playerId) && registry.hasComponent<Inventory>(playerId))
			{
				if (registry.hasComponent<PlayerComponent>(playerId) && !registry.getComponent<PlayerComponent>(playerId).isOnline)
				{
					return;
				}

				Transform& transform = registry.getComponent<Transform>(playerId);
				CharacterController& controller = registry.getComponent<CharacterController>(playerId);
				Rigidbody& rb = registry.getComponent<Rigidbody>(playerId);
				Inventory& inventory = registry.getComponent<Inventory>(playerId);

				if (generateCubemap)
				{
					controller.movementAxis.x = 0;
					controller.movementAxis.y = 0;
					controller.movementAxis.z = 0;
					controller.applyJumpForce = false;
					controller.viewAxis.x = 0;
					controller.viewAxis.y = 0;
					Scene::getCamera().fov = 90.0f;

					switch (sideGenerating)
					{
					case CubemapSide::Back:
						transform.orientation.x = 0.0f;
						transform.orientation.y = 180.0f;
						break;
					case CubemapSide::Left:
						transform.orientation.x = 0.0f;
						transform.orientation.y = 90.0f;
						break;
					case CubemapSide::Front:
						transform.orientation.x = 0.0f;
						transform.orientation.y = 0.0f;
						break;
					case CubemapSide::Right:
						transform.orientation.x = 0.0f;
						transform.orientation.y = 270.0f;
						break;
					case CubemapSide::Top:
						transform.orientation.x = 89.9f;
						transform.orientation.y = 0.0f;
						break;
					case CubemapSide::Bottom:
						transform.orientation.x = -89.9f;
						transform.orientation.y = 0.0f;
						break;
					}

					Application::takeScreenshot(magic_enum::enum_name(sideGenerating).data(), true);
					sideGenerating = (CubemapSide)((int)sideGenerating + 1);
					if (sideGenerating == CubemapSide::Length)
					{
						sideGenerating = (CubemapSide)0;
						generateCubemap = false;
						Scene::getCamera().fov = 45.0f;
					}
					// Don't update the HUD or anything
					return;
				}

				switch (gameMode)
				{
				case GameMode::Survival:
					updateSurvival(transform, controller, rb, inventory);
					break;
				case GameMode::Creative:
					updateCreative(transform, controller, rb, inventory);
					break;
				case GameMode::Spectator:
					updateSpectator(transform, controller, rb);
					break;
				default:
					break;
				}

				// Common key bindings across all game modes
				if (KeyBindings::keyBeginPress(KeyBind::Escape))
				{
					if (CommandLine::isActive)
					{
						CommandLine::isActive = false;
					}
					else if (MainHud::viewingCraftScreen)
					{
						MainHud::viewingCraftScreen = false;
						Application::getWindow().setCursorMode(CursorMode::Locked);
					}
					else if (!MainHud::isPaused)
					{
						Application::getWindow().setCursorMode(CursorMode::Normal);
						MainHud::isPaused = true;
					}
					else if (MainHud::isPaused)
					{
						Application::getWindow().setCursorMode(CursorMode::Locked);
						MainHud::isPaused = false;
					}
				}

				DebugStats::playerPos = transform.position;
				DebugStats::playerOrientation = transform.orientation;

				MainHud::update(inventory);

				SizedMemory transformData = pack<glm::vec3, glm::vec3, Ecs::EntityId>(transform.position, transform.orientation, playerId);
				Network::sendUserCommand(UserCommandType::UpdateTransform, transformData);
				g_memory_free(transformData.memory);
			}
		}

		void setPlayerIfNeeded(bool forceOverride)
		{
			Ecs::Registry* registry = Scene::getRegistry();
			if (forceOverride || playerId == Ecs::nullEntity || !registry->hasComponent<Tag>(playerId) ||
				registry->getComponent<Tag>(playerId).type != TagType::Player)
			{
				playerId = World::getLocalPlayer();
				if (registry->hasComponent<CharacterController>(playerId) && registry->hasComponent<Rigidbody>(playerId))
				{
					CharacterController& controller = registry->getComponent<CharacterController>(playerId);
					Rigidbody& rb = registry->getComponent<Rigidbody>(playerId);
					controller.controllerBaseSpeed = 4.4f;
					controller.controllerRunSpeed = 6.2f;
					rb.useGravity = true;
					controller.lockedToCamera = true;
					PlayerComponent& playerComp = registry->getComponent<PlayerComponent>(playerId);
					gameMode = GameMode::Survival;
					g_logger_info("Player controller found player: '%s'", playerComp.name);
				}
			}
		}
		glm::vec3 centerNow;
		glm::vec3 centerLast;
		float hardness = 0.65f;
		bool viewNotChanged = false;
		float time = hardness / 8;
		bool in1 = true;
		bool in2 = false;
		bool in3 = false;
		bool in4 = false;
		bool in5 = false;
		bool in6 = false;
		bool in7 = false;
		bool in8 = false;
		float acceleration;
		float fallTimer;
		float damagefloat;

		/*const glm::vec4 armVerts = {
			{0, 0, 0},
			{1, 0, 0},
			{1, 0, 1},
			{-1, 0, 1},

			{0, -1.5, 0},
			{1, -1.5, 0},
			{1, -1.5, 1},
			{-1, -1.5, 1}
		};*/

		/*const glm::vec4 armVerts2[2][4] = {
			{{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 1, 0}, {1, 0, -1, 0}},
			{{0, -2, 0, 0}, {1, -2, 0, 0}, {1, -2, 1, 0}, {1, -2, -1, 0}}
		};*/

		/*struct VoxelVertex
		{
			glm::vec3 position;
			glm::u8vec4 color;
		};*/

		static void updateSurvival(Transform& transform, CharacterController& controller, Rigidbody& rb, Inventory& inventory)
		{
			HeartManager::Init();

			HeartManager::Update();

			//Sounds::PlaySound("grass.wav", 50);

			blockPlaceDebounce -= World::deltaTime;
			//Renderer::drawTexturedBox(transform.position + glm::vec3{0, 0, -1}, glm::vec3{ 0.4, 1.2, 0.4 }, *topSprite, *sideSprite, *topSprite);
			Renderer::drawBox(transform.position + glm::vec3{ 0.14, 0.5, -0.37 }, glm::vec3{ 0.08, 0.08, 0.24 }, Minecraft::Styles::defaultStyle, glm::vec3{ 0, 15, 0 });
			//Renderer::draw3DModel(transform.position + (glm::vec3(0.0f, 0.0f, 1.0f) * -1.0f * 2.7f), glm::vec3(1.0f), 0.0f, stick.vertices, stick.verticesLength);
			if (!MainHud::viewingCraftScreen && !CommandLine::isActive && !MainHud::isPaused)
			{
				if (!rb.onGround)
				{
					fallTimer += 0.0087f;
					acceleration += 0.045f;

					std::cout << fallTimer << std::endl;
				}

				if (rb.onGround && fallTimer <= 0.5)
				{
					fallTimer = 0.0f;

					acceleration = 0.0f;
				}
				else if (rb.onGround && fallTimer >= 0.5)
				{
					damagefloat = fallTimer * acceleration;

					HeartManager::health -= damagefloat;

					std::cout << damagefloat << std::endl;

					fallTimer = 0;
					acceleration = 0;
				}

				RaycastStaticResult res = Physics::raycastStatic(transform.position + controller.cameraOffset, transform.forward, 5.0f);
				if (res.hit)
				{
					glm::vec3 blockLookingAtPos = res.point - (res.hitNormal * 0.1f);

					centerNow = res.blockCenter;

					if (centerNow != centerLast)
					{
						hardness = 0.65f;
						side = &BlockMap::getTextureFormat("destroy_stage_1");
						in1 = true;
					}

					if (Input::isMouseHeldDown(GLFW_MOUSE_BUTTON_LEFT) && MainHud::viewingCraftScreen == false)
					{
						time -= 0.01f;

						if (in1 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_1");
							in1 = false;
							in2 = true;
							time = hardness / 8;
						}
						else if (in2 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_2");
							in2 = false;
							in3 = true;
							time = hardness / 8;
						}
						else if (in3 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_3");
							in3 = false;
							in4 = true;
							time = hardness / 8;
						}
						else if (in4 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_4");
							in4 = false;
							in5 = true;
							time = hardness / 8;
						}
						else if (in5 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_5");
							in5 = false;
							in6 = true;
							time = hardness / 8;
						}
						else if (in6 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_6");
							in6 = false;
							in7 = true;
							time = hardness / 8;
						}
						else if (in7 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_7");
							in7 = false;
							in8 = true;
							time = hardness / 8;
						}
						else if (in8 == true && time <= 0)
						{
							side = &BlockMap::getTextureFormat("destroy_stage_8");
							in8 = false;
							time = hardness / 8;
						}

						glm::vec3 worldPos = res.point - (res.hitNormal * 0.1f);
						Block currBlockBroken2 = ChunkManager::getBlock(res.blockCenter);
						if (inventory.hotbar[inventory.currentHotbarSlot].blockId == 32012 && currBlockBroken2.id == 6)
						{
							hardness -= 0.08f;
						}
						else if (inventory.hotbar[inventory.currentHotbarSlot].blockId == 32011 && currBlockBroken2.id == 8)
						{
							hardness -= 0.08f;
						}
						else
						{
							hardness -= 0.01f;
						}

						Renderer::drawTexturedCube(res.blockCenter, res.blockSize + glm::vec3{ 0.0005f, 0.0005f, 0.0005f }, *side, *side, *side);
					}
					else
					{
						hardness = 0.65f;
						in1 = true;
						side = &BlockMap::getTextureFormat("destroy_stage_1");
					}

					centerLast = res.blockCenter;

					DebugStats::blockLookingAt = ChunkManager::getBlock(blockLookingAtPos);
					DebugStats::airBlockLookingAt = ChunkManager::getBlock(res.point + (res.hitNormal * 0.1f));

					// TODO: Clean this garbage up
					Renderer::drawBox(res.blockCenter, res.blockSize + glm::vec3(0.005f, 0.005f, 0.005f), blockHighlight);
					//Renderer::draw3DModel(transform.position, glm::vec3{ 1, 1, 1 }, 0, armVerts, 8);
					//Renderer::drawTexturedCube(transform.position, res.blockSize - glm::vec3{ 0.3, 0.3, 0.3 }, * sideSprite, * topSprite, * topSprite);
					//Renderer::drawBox(res.point, glm::vec3(0.1f, 0.1f, 0.1f), Styles::defaultStyle);
					static float rotation = 0.0f;
					static glm::vec3 verticalOffset = glm::vec3(0.0f);
					static float speedDir = 0.05f;
					static int changeDirTick = 0;
					verticalOffset.y += speedDir * World::deltaTime;
					//Renderer::drawTexturedCube(res.point + (res.hitNormal * 0.1f) + verticalOffset, glm::vec3(0.2f, 0.2f, 0.2f), *sideSprite, *topSprite, *bottomSprite, rotation);
					rotation = rotation + 30.0f * World::deltaTime;
					changeDirTick++;
					if (changeDirTick > 30)
					{
						changeDirTick = 0;
						speedDir *= -1.0f;
					}
					if (rotation > 360.0f)
					{
						rotation = rotation / 360.0f;
					}

					if (Input::isKeyPressed(GLFW_KEY_Q))
					{
						inventory.hotbar[inventory.currentHotbarSlot].blockId = 0;
					}

					if (Input::isMousePressed(GLFW_MOUSE_BUTTON_RIGHT) && blockPlaceDebounce <= 0)
					{
						static Block newBlock{
							0, 0, 0, 0
						};
						newBlock.id = inventory.hotbar[inventory.currentHotbarSlot].blockId;

						if (newBlock != BlockMap::NULL_BLOCK && newBlock != BlockMap::AIR_BLOCK && !newBlock.isItemOnly())
						{
							glm::vec3 worldPos = res.point + (res.hitNormal * 0.1f);
							ChunkManager::setBlock(worldPos, newBlock);

							inventory.hotbar[inventory.currentHotbarSlot].count -= 1;
							if (inventory.hotbar[inventory.currentHotbarSlot].count == 0)
							{
								inventory.hotbar[inventory.currentHotbarSlot].blockId = 0;
							}

							// If the network is enabled also send this across the network
							if (Network::isNetworkEnabled())
							{
								SizedMemory sizedMemory = pack<glm::vec3, Block>(worldPos, newBlock);
								Network::sendClientCommand(ClientCommandType::SetBlock, sizedMemory);
								g_memory_free(sizedMemory.memory);
							}
							blockPlaceDebounce = blockPlaceDebounceTime;
						}
					}
					else if (Input::isMousePressed(GLFW_MOUSE_BUTTON_LEFT) && blockPlaceDebounce <= 0 && hardness <= 0)
					{
						glm::vec3 worldPos = res.point - (res.hitNormal * 0.1f);

						Ecs::EntityId player = World::getLocalPlayer();
						Block currBlockBroken = ChunkManager::getBlock(worldPos);

						if (!currBlockBroken.isNull())
						{
							World::givePlayerBlock(player, currBlockBroken.id, 1);
						}

						hardness = 0.65f;

						in1 = true;

						ChunkManager::removeBlock(worldPos);

						// If the network is enabled also send this across the network
						if (Network::isNetworkEnabled())
						{
							SizedMemory sizedMemory = pack<glm::vec3>(worldPos);
							Network::sendClientCommand(ClientCommandType::RemoveBlock, sizedMemory);
							g_memory_free(sizedMemory.memory);
						}
						blockPlaceDebounce = blockPlaceDebounceTime;
					}
				}
				else
				{
					DebugStats::blockLookingAt = BlockMap::NULL_BLOCK;
				}

				controller.viewAxis.x = Input::deltaMouseX;
				controller.viewAxis.y = Input::deltaMouseY;
				controller.isRunning = Input::isKeyPressed(GLFW_KEY_LEFT_CONTROL);

				controller.movementAxis.x =
					Input::isKeyPressed(GLFW_KEY_W)
					? 1.0f
					: Input::isKeyPressed(GLFW_KEY_W && GLFW_KEY_LEFT_CONTROL)
					? 3.5f
					: Input::isKeyPressed(GLFW_KEY_S)
					? -1.0f
					: 0.0f;
				controller.movementAxis.z =
					Input::isKeyPressed(GLFW_KEY_D)
					? 1.0f
					: Input::isKeyPressed(GLFW_KEY_A)
					? -1.0f
					: 0.0f;

				if (rb.onGround)
				{
					if (Input::keyBeginPress(GLFW_KEY_SPACE))
					{
						controller.applyJumpForce = true;
					}
				}

				updateInventory(inventory);
			}

			if (Input::keyBeginPress(GLFW_KEY_F4))
			{
				controller.controllerBaseSpeed = 4.4f;
				controller.controllerRunSpeed = 6.2f;
				gameMode = GameMode::Creative;
				MainHud::notify("Game Mode Creative");
			}

			if (!CommandLine::isActive && Input::keyBeginPress(GLFW_KEY_E))
			{
				MainHud::viewingCraftScreen = !MainHud::viewingCraftScreen;
				CursorMode mode = MainHud::viewingCraftScreen
					? CursorMode::Normal
					: CursorMode::Locked;
				Application::getWindow().setCursorMode(mode);
			}
		}

		static void updateCreative(Transform& transform, CharacterController& controller, Rigidbody& rb, Inventory& inventory)
		{
			static float doubleJumpDebounce = 0.0f;
			const float doubleJumpDebounceTime = 0.5f;
			blockPlaceDebounce -= World::deltaTime;
			doubleJumpDebounce -= World::deltaTime;

			if (!MainHud::viewingCraftScreen && !CommandLine::isActive && !MainHud::isPaused)
			{
				RaycastStaticResult res = Physics::raycastStatic(transform.position + controller.cameraOffset, transform.forward, 5.0f);
				if (res.hit)
				{
					glm::vec3 blockLookingAtPos = res.point - (res.hitNormal * 0.1f);
					DebugStats::blockLookingAt = ChunkManager::getBlock(blockLookingAtPos);
					DebugStats::airBlockLookingAt = ChunkManager::getBlock(res.point + (res.hitNormal * 0.1f));

					Renderer::drawBox(res.blockCenter, res.blockSize + glm::vec3(0.005f, 0.005f, 0.005f), blockHighlight);

					if (Input::isMousePressed(GLFW_MOUSE_BUTTON_RIGHT) && blockPlaceDebounce <= 0)
					{
						static Block newBlock{
							0, 0, 0, 0
						};
						newBlock.id = inventory.hotbar[inventory.currentHotbarSlot].blockId;

						if (newBlock != BlockMap::NULL_BLOCK && newBlock != BlockMap::AIR_BLOCK && !newBlock.isItemOnly())
						{
							glm::vec3 worldPos = res.point + (res.hitNormal * 0.1f);
							ChunkManager::setBlock(worldPos, newBlock);
							// If the network is enabled also send this across the network
							if (Network::isNetworkEnabled())
							{
								SizedMemory sizedMemory = pack<glm::vec3, Block>(worldPos, newBlock);
								Network::sendClientCommand(ClientCommandType::SetBlock, sizedMemory);
								g_memory_free(sizedMemory.memory);
							}
							blockPlaceDebounce = blockPlaceDebounceTime;
						}
					}
					else if (Input::isMousePressed(GLFW_MOUSE_BUTTON_LEFT) && blockPlaceDebounce <= 0)
					{
						glm::vec3 worldPos = res.point - (res.hitNormal * 0.1f);

						Block currBlockBroken = ChunkManager::getBlock(worldPos);
						Ecs::EntityId player = World::getLocalPlayer();
						if (!currBlockBroken.isNull() && currBlockBroken.id != 1)
						{
							World::givePlayerBlock(player, currBlockBroken.id, 1);
						}

						// If the network is enabled also send this across the network
						ChunkManager::removeBlock(worldPos);
						if (Network::isNetworkEnabled())
						{
							SizedMemory sizedMemory = pack<glm::vec3>(worldPos);
							Network::sendClientCommand(ClientCommandType::RemoveBlock, sizedMemory);
							g_memory_free(sizedMemory.memory);
						}
						blockPlaceDebounce = blockPlaceDebounceTime;
					}
				}
				else
				{
					DebugStats::blockLookingAt = BlockMap::NULL_BLOCK;
				}

				controller.viewAxis.x = Input::deltaMouseX;
				controller.viewAxis.y = Input::deltaMouseY;
				controller.isRunning = Input::isKeyPressed(GLFW_KEY_LEFT_CONTROL);

				controller.movementAxis.x =
					Input::isKeyPressed(GLFW_KEY_W)
					? 1.0f
					: Input::isKeyPressed(GLFW_KEY_S)
					? -1.0f
					: 0.0f;
				if (!rb.useGravity)
				{
					controller.inMiddleOfJump = false;
					controller.movementAxis.y =
						Input::isKeyPressed(GLFW_KEY_SPACE)
						? 1.0f
						: Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT)
						? -1.0f
						: 0.0f;
				}
				controller.movementAxis.z =
					Input::isKeyPressed(GLFW_KEY_D)
					? 1.0f
					: Input::isKeyPressed(GLFW_KEY_A)
					? -1.0f
					: 0.0f;

				if (rb.onGround)
				{
					if (Input::keyBeginPress(GLFW_KEY_SPACE))
					{
						controller.applyJumpForce = true;
						doubleJumpDebounce = doubleJumpDebounceTime;
					}
				}
				else if (!rb.onGround && doubleJumpDebounce < 0 && !rb.useGravity)
				{
					if (Input::keyBeginPress(GLFW_KEY_SPACE))
					{
						doubleJumpDebounce = doubleJumpDebounceTime;
					}
				}
				else if (doubleJumpDebounce >= 0)
				{
					// They just double tapped spacebar
					if (Input::keyBeginPress(GLFW_KEY_SPACE))
					{
						controller.applyJumpForce = false;
						controller.inMiddleOfJump = false;
						rb.useGravity = !rb.useGravity;
						if (!rb.useGravity)
						{
							rb.zeroForces();
							controller.controllerBaseSpeed *= 2.0f;
							controller.controllerRunSpeed *= 2.0f;
						}
						else
						{
							controller.controllerBaseSpeed /= 2.0f;
							controller.controllerRunSpeed /= 2.0f;
						}
						doubleJumpDebounce = -1.0f;
					}
				}

				updateInventory(inventory);
			}

			if (Input::keyBeginPress(GLFW_KEY_F4))
			{
				if (rb.useGravity)
				{
					controller.controllerBaseSpeed *= 2.0f;
					controller.controllerRunSpeed *= 2.0f;
					rb.useGravity = false;
					rb.zeroForces();
					controller.inMiddleOfJump = false;
				}
				rb.isSensor = true;
				gameMode = GameMode::Spectator;
				MainHud::hotbarVisible = false;
				MainHud::notify("Game Mode Spectator");
			}

			if (!CommandLine::isActive && Input::keyBeginPress(GLFW_KEY_E))
			{
				MainHud::viewingCraftScreen = !MainHud::viewingCraftScreen;
				CursorMode mode = MainHud::viewingCraftScreen
					? CursorMode::Normal
					: CursorMode::Locked;
				Application::getWindow().setCursorMode(mode);
			}
		}

		static void updateSpectator(Transform& transform, CharacterController& controller, Rigidbody& rb)
		{
			controller.viewAxis.x = Input::deltaMouseX;
			controller.viewAxis.y = Input::deltaMouseY;
			controller.isRunning = Input::isKeyPressed(GLFW_KEY_LEFT_CONTROL);

			controller.movementAxis.x =
				Input::isKeyPressed(GLFW_KEY_W)
				? 1.0f
				: Input::isKeyPressed(GLFW_KEY_S)
				? -1.0f
				: 0.0f;
			controller.movementAxis.y =
				Input::isKeyPressed(GLFW_KEY_SPACE)
				? 1.0f
				: Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT)
				? -1.0f
				: 0.0f;
			controller.movementAxis.z =
				Input::isKeyPressed(GLFW_KEY_D)
				? 1.0f
				: Input::isKeyPressed(GLFW_KEY_A)
				? -1.0f
				: 0.0f;

			if (Input::keyBeginPress(GLFW_KEY_F4))
			{
				controller.controllerBaseSpeed /= 2.0f;
				controller.controllerRunSpeed /= 2.0f;
				gameMode = GameMode::Survival;
				rb.isSensor = false;
				rb.useGravity = true;
				MainHud::hotbarVisible = true;
				MainHud::notify("Game Mode Survival");
			}
		}

		static void updateInventory(Inventory& inventory)
		{
			for (int i = 0; i < Player::numHotbarSlots; i++)
			{
				if (Input::keyBeginPress(GLFW_KEY_1 + i))
				{
					inventory.currentHotbarSlot = i;
				}
			}

			if (Input::mouseScrollY != 0)
			{
				inventory.currentHotbarSlot -= (int)Input::mouseScrollY;
				if (inventory.currentHotbarSlot < 0)
				{
					inventory.currentHotbarSlot = CMath::negativeMod(inventory.currentHotbarSlot, 0, Player::numHotbarSlots - 1);
				}
				else if (inventory.currentHotbarSlot >= Player::numHotbarSlots)
				{
					inventory.currentHotbarSlot = inventory.currentHotbarSlot % 9;
				}
			}
		}
	}
}