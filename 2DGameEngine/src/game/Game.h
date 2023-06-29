#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include "../logger/Logger.h"
#include "../ECS/ECS.h"
#include <memory>

const int FPS = 30;
const int MILLISECS_PER_FRAME = 1000 / FPS;
const bool CAPPED_FPS = false;

class Game
{
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool isRunning;
	int windowWidth, windowHeight;

	std::unique_ptr<Registry> registry;

public:
	Game(int windowWidth, int windowHeight);
	Game();
	~Game();
	void Initialize();
	void Run();
	void Setup();
	void ProcessInput();
	void Update(double deltaTime);
	void Render();
};

