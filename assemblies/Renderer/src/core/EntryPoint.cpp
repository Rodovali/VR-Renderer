// VR Renderer - Entry point
// Rodolphe VALICON
// 2025

#include "core/Application.h"
#include "core/Logger.h"

#include <exception>
#include <iostream>

int main() {
	try {
		vr::Application* app = vr::createApplication();
		app->run();
		delete app;
	} catch (const std::exception& exception) {
		vr::logger::fatal("A fatal error occured: {}", exception.what());
		return 1;
	}
	return 0;
}