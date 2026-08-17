#include "Application/Application.h"

Application::Application()
	: database()
	, sdl()
	, events()
	, windowing()
	, rendering(windowing, database)
{}

std::unique_ptr<Application> application;
