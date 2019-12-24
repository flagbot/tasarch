#pragma once

#include "Core.hpp"
#include "Signals.h"

class Emulator {
public:
    Emulator();
    
    void LoadCore(std::string filename);
    
    Core* core;
    InputManager* inputs;
    
};
