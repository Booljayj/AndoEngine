#include "HAL/EventFramework.h"
#include "HAL/SDLFramework.h"
#include "HAL/WindowFramework.h"
#include "Rendering/RenderingFramework.h"
#include "Resources/MemoryDatabase.h"
#include "Engine/SmartPointers.h"

/**
 * Represents the application that is running. A singleton instance of this is created during startup.
 * The application is a collection of frameworks, which handle high-level intiailization and management of different parts of the application.
 */
struct Application {
	Resources::MemoryDatabase database;

	HAL::SDLFramework sdl;
	HAL::EventFramework events;
	HAL::WindowFramework windowing;
	Rendering::RenderingFramework rendering;

	Application();
	virtual ~Application() = default;
};

/** The singleton application that is running. */
extern std::unique_ptr<Application> application;
