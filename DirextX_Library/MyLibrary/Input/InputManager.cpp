#include "InputManager.h"

InputManager::InputManager()
{
	keyInput = std::make_unique<KeyInput>();
	mouseInput = std::make_unique<MouseInput>();
}


InputManager::~InputManager()
{
	
}

void InputManager::Update()
{
	keyInput->UpdateInputState();
	mouseInput->UpdateInputState();
}