#include <fmt/core.h>
#include <fmt/color.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <entt/entt.hpp>
#include <memory>
#include <vector>

#pragma region ENGINE_INCLUDES
#include <SuperCoco/Core.hpp>
#include <SuperCoco/Window.hpp>
#include <SuperCoco/Renderer.hpp>
#include <SuperCoco/Texture.hpp>
#include <SuperCoco/Sprite.hpp>
#include <SuperCoco/Surface.hpp>
#include <SuperCoco/ResourceManager.hpp>
#include <SuperCoco/InputManager.hpp>
#include <SuperCoco/Stopwatch.hpp>
#include <SuperCoco/Transform.hpp>
#include <SuperCoco/SpriteSheet.hpp>
#include <SuperCoco/Model.hpp>
#include <SuperCoco/ComponentRegistry.hpp>
#include <SuperCoco/TimerManager.hpp>
#include <SuperCoco/Maths.hpp>
#include <SuperCoco/Systems/RenderSystem.hpp>
#include <SuperCoco/Systems/VelocitySystem.hpp>
#include <SuperCoco/Systems/GravitySystem.hpp>
#include <SuperCoco/Systems/AnimationSystem.hpp>
#include <SuperCoco/Systems/PhysicsSystem.hpp>
#include <SuperCoco/Components/VelocityComponent.hpp>
#include <SuperCoco/Components/GraphicsComponent.hpp>
#include <SuperCoco/Components/CameraComponent.hpp>
#include <SuperCoco/Components/SpritesheetComponent.hpp>
#include <SuperCoco/Components/RigidBodyComponent.hpp>
#include <SuperCoco/Components/TextComponent.hpp>
#include <SuperCoco/CollisionShape.hpp>
#pragma endregion
#include <BulletForge/Game.hpp>
#include <BulletForge/HealthComponent.hpp>
#include <BulletForge/DeathSystem.hpp>

#ifdef WITH_SCE_EDITOR
#include <SuperCoco/WorldEditor.hpp>
#include <SuperCoco/ImGuiRenderer.hpp>
#include <imgui.h>
#endif

#include <SuperCoco/WelcomeMsg.inl>

int main(int argc, char* argv[])
{
	#pragma region SuperCocoEngine
	WelcomeMessage();
	
	srand(time(NULL));

	Sce::Core core(SDL_INIT_GAMECONTROLLER);
	Sce::Window window("Bullet Forge", 1080, 769);
	Sce::Renderer renderer(window, 1);
	Sce::ResourceManager rcmgr(&renderer);
	Sce::InputManager inputmgr;
	Sce::TimerManager timermgr;

#ifdef WITH_SCE_EDITOR
	Sce::ImGuiRenderer imgui(window, renderer);
	ImGui::SetCurrentContext(imgui.GetContext());
#endif

	entt::registry world;
	Sce::RenderSystem renderSystem(&world, &renderer);
	Sce::VelocitySystem velocitySystem(&world);
	Sce::GravitySystem gravitySystem(&world);
	Sce::AnimationSystem animationSystem(&world);
	Sce::PhysicsSystem physicSystem(world);

	Sce::ComponentRegistry componentRegistry;

#ifdef WITH_SCE_EDITOR
	std::optional<Sce::WorldEditor> worldEditor;
#endif
#pragma endregion

	BulletForge::Game game;
	BulletForge::DeathSystem deathSystem(&world);

	#pragma region ENTITIES
	entt::handle camera = core.CreateCamera(world, { -1080.f / 2.f, -769.f / 2.f }, {1.f, 1.f});

	entt::handle Player = game.CreatePlayer(world, renderer, { (1080.f / 2.f) * 1.5f, (769.f / 2.f) * 1.5f }, 1);
	Sce::Transform* pTransform = &Player.get<Sce::Transform>();
	Sce::RigidBodyComponent* rb = &Player.get<Sce::RigidBodyComponent>();
	Sce::SpritesheetComponent* pSheet = &Player.get<Sce::SpritesheetComponent>();

	entt::handle sword = game.CreateWeapon(world, renderer, { 16 * 10, 16 * 8, 16, 16 }, { (1080.f / 2.f) * 1.5f, (769.f / 2.f) * 1.5f }, {0.f, 0.f}, 2);
	Sce::RigidBodyComponent* swordrb = &sword.get<Sce::RigidBodyComponent>();

	timermgr.CreateTimer(5.f, [&world, &renderer, &game, &core, &pTransform]()
		{
			BulletForge::EnemyType enemyType = static_cast<BulletForge::EnemyType>(std::rand() % 5);
			Sce::Vector2f pos = pTransform->GetPosition();
			pos.x += static_cast<float>(-300 + std::rand() % 601);
			pos.y += static_cast<float>(-300 + std::rand() % 601);
			entt::handle enemy = game.CreateEnemy(world, renderer, enemyType, pos, 1, core);
			Sce::Transform* eTransform = &enemy.get<Sce::Transform>();
			eTransform->SetScale({ 0.f,0.f });
			Sce::TimerManager::CreateContinuousTimer(0.75f, [enemy](float delta, float current)
				{
					Sce::Transform* eTransform = &enemy.get<Sce::Transform>();
					float progress = Sce::Maths::EaseOutBack(current / 0.75f);
					eTransform->SetScale({ progress, progress });
				});
		}, 0.f, true);

	camera.get<Sce::Transform>().SetParent(pTransform);

	#pragma endregion

	#pragma region INPUTBINDING
	Sce::Vector2f moveInput{0.f,0.f};
	float rotationInput = 0.f;

	inputmgr.BindKeyPressed(SDLK_ESCAPE, "Close");
	inputmgr.BindControllerAxis(SDL_CONTROLLER_AXIS_LEFTX, "moveX");
	inputmgr.BindControllerAxis(SDL_CONTROLLER_AXIS_LEFTY, "moveY");
	inputmgr.BindControllerAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT, "rotateLeft");
	inputmgr.BindControllerAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, "rotateRight");
	
	inputmgr.BindAction("moveX", [&moveInput, &rb, &pSheet](bool active, int, float value)
		{
			if (!active)
				return;
			moveInput.x = value;
			if (value > -0.2f && value < 0.2f)
				moveInput.x = 0;
			rb->SetLinearVelocity(moveInput * 150);
			if (value < 0)
				pSheet->m_targetSprite.get()->Resize(-48, 48);
			else
				pSheet->m_targetSprite.get()->Resize(48, 48);
			
		});
	inputmgr.BindAction("moveY", [&moveInput, &rb](bool active, int, float value)
		{
			if (!active)
				return;
			moveInput.y = value;
			if (value > -0.2f && value < 0.2f)
				moveInput.y = 0;
			rb->SetLinearVelocity(moveInput * 150);
		});
	inputmgr.BindAction("rotateLeft", [&rotationInput ,&swordrb](bool active, int, float value)
		{
			if (!active)
				return;
			rotationInput = -180 * value;
			swordrb->SetAngularVelocity(rotationInput);
		});
	inputmgr.BindAction("rotateRight", [&rotationInput, &swordrb](bool active, int, float value)
		{
			if (!active)
				return;
			rotationInput = 180 * value;
			swordrb->SetAngularVelocity(rotationInput);
		});

	#ifdef WITH_SCE_EDITOR
	inputmgr.BindKeyPressed(SDLK_F1, "OpenEditor");
	inputmgr.BindAction("OpenEditor", [&](bool active, int, float) 
	{
		if (!active)
			return;

		if (worldEditor)
		{
			worldEditor.reset();
		}
		else
		{
			worldEditor.emplace(window, world, componentRegistry);
		}
	});
	#endif
	#pragma endregion

	Sce::Stopwatch stopwatch;
	 
	bool isOpen = true;

	inputmgr.BindAction("Close", [&isOpen](bool, int, float) { isOpen = false; });

	while (isOpen)
	{
		float deltaTime = stopwatch.Restart();

		SDL_Event event;
		while (Sce::Core::PollEvent(event) > 0)
		{
			switch (event.type)
			{
			case SDL_QUIT:
				isOpen = false;
				break;
			}

			inputmgr.HandleEvent(event);

#ifdef WITH_SCE_EDITOR
			imgui.ProcessEvent(event);
#endif
		}

		core.Update();
		renderer.RenderDrawColor(100, 0, 0, 0);
		renderer.RenderClear();

#ifdef WITH_SCE_EDITOR
		imgui.NewFrame();
#endif

		swordrb->TeleportTo(rb->GetPosition()); // <- C'est degueulasse mais vas-y il est 3h31 du matin, j'entends les oiseaux chanter

		timermgr.UpdateTimers(deltaTime);
		animationSystem.Update(deltaTime);
		gravitySystem.ApplyGravity(deltaTime);
		velocitySystem.ApplyVelocity(deltaTime);
		physicSystem.Update(deltaTime);

		deathSystem.DeathNote();

		renderSystem.Render(deltaTime);

#ifdef WITH_SCE_EDITOR
		physicSystem.DebugDraw(renderer, core.GetCameraTransform(world).WorldToLocalMatrix());

		if (worldEditor)
			worldEditor->Render();

		imgui.Render(renderer);
#endif

		renderer.RenderPresent();
	}

	return 0;
}
