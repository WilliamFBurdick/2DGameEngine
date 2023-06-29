#include "Game.h"
#include "../ECS/ECS.h"
#include <iostream>
#include "../components/TransformComponent.h"
#include "../components/RigidbodyComponent.h"

Game::Game(int windowWidth, int windowHeight)
	:windowWidth(windowWidth), windowHeight(windowHeight)
{
	registry = std::make_unique<Registry>();
	isRunning = false;
}

Game::Game() {
	registry = std::make_unique<Registry>();
	isRunning = false;
}

Game::~Game() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Game::Initialize() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		Logger::Err("Error initializing SDL.");
		return;
	}

	SDL_DisplayMode display;
	SDL_GetCurrentDisplayMode(0, &display);

	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, display.w, display.h, SDL_WINDOW_BORDERLESS);
	if (!window) {
		Logger::Err("Error creating SDL window.");
		return;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		Logger::Err("Error creating SDL renderer.");
		return;
	}

	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	isRunning = true;
}

void Game::Setup() {
	// Create some entities
	Entity tank = registry->CreateEntity();
	
	// Add some components
	tank.AddComponent<TransformComponent>(glm::vec2(10.0, 30.0), glm::vec2(1.0, 1.0), 0.0);
	tank.AddComponent<RigidbodyComponent>(glm::vec2(50.0, 0.0));

	// Remove a components
	tank.RemoveComponent<TransformComponent>();
	
}

void Game::Run() {
	Setup();
	int msPreviousFrame = SDL_GetTicks();
	while (isRunning) {
		if (CAPPED_FPS) {
			int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - msPreviousFrame);
			if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
				SDL_Delay(timeToWait);
			}
		}

		ProcessInput();
		Update((SDL_GetTicks() - msPreviousFrame) / 1000.0f);
		Render();

		msPreviousFrame = SDL_GetTicks();
	}
}

void Game::ProcessInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					isRunning = false;
				}
				break;
		}
	}
}

void Game::Update(double deltaTime) {
	// TODO: Update game objects
	
}

void Game::Render() {
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	// TODO: Render all game objects
	

	SDL_RenderPresent(renderer);
}