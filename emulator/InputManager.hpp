//
//  InputManager.hpp
//  tasarch
//
//  Created by Leonardo Galli on 24.12.19.
//

#pragma once

#include "../bitmask.hpp"
#include "Core.hpp"

enum class Input {
    B = 1 << 0,
    Y = 1 << 1,
    SELECT = 1 << 2,
    START = 1 << 3,
    UP = 1 << 4,
    DOWN = 1 << 5,
    LEFT = 1 << 6,
    RIGHT = 1 << 7,
    A = 1 << 8,
    X = 1 << 9,
    L = 1 << 10,
    R = 1 << 11,
    
    _bitmask_max_element = R
};

BITMASK_DEFINE(Input)
// We need to have this after BITMASK_DEFINE, otherwise the macro does not work!
using namespace bitmask;

class InputManager {
public:
    InputManager();
    bitmask<Input> current;
    
private:
    void CoreLoaded(Core* core);
    void InputPoll();
};

