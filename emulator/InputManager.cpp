//
//  InputManager.cpp
//  tasarch
//
//  Created by Leonardo Galli on 24.12.19.
//

#include "InputManager.hpp"
#include "Signals.h"

InputManager::InputManager()
{
    Signals::CoreLoaded.Connect(sigc::mem_fun(this, &InputManager::CoreLoaded));
}

void InputManager::CoreLoaded(Core* core)
{
    core->RetroSetInputPoll;
    core->RetroSetInputState;
}
