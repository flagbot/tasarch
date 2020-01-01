//
//  InputManager.hpp
//  tasarch
//
//  Created by Leonardo Galli on 24.12.19.
//

#pragma once

#include "../bitmask.hpp"
#include "Core.hpp"
#include "gtkmm.h"
#include <vector>

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
    RESET = 1 << 16,
    _bitmask_max_element = RESET
};

BITMASK_DEFINE(Input)
// We need to have this after BITMASK_DEFINE, otherwise the macro does not work!
using namespace bm;

class InputMapping {
public:
    InputMapping(Input input = Input::B, Gtk::AccelKey shortcut = Gtk::AccelKey(), std::string name = "");
    Input input;
    Gtk::AccelKey shortcut;
    std::string name;
};

#define INPUTMAP(inp, default) {InputMapping(Input::inp, Gtk::AccelKey(default), #inp)},

class InputManager {
public:
    InputManager(Gtk::Window* mainWindow);
    bitmask<Input> current;
    
    inline static std::vector<InputMapping> InputMap = {
        INPUTMAP(A, "A")
        INPUTMAP(B, "S")
        INPUTMAP(X, "X")
        INPUTMAP(Y, "Y")
        INPUTMAP(UP, "Up")
        INPUTMAP(DOWN, "Down")
        INPUTMAP(LEFT, "Left")
        INPUTMAP(RIGHT, "Right")
        INPUTMAP(START, "period")
        INPUTMAP(SELECT, "comma")
        INPUTMAP(RESET, "R")
    };
    
private:
    void CoreLoaded(Core* core);
    void InputPoll();
    int16_t GetInputState(unsigned port, unsigned device, unsigned index, unsigned id);
    bool KeyPressEvent(GdkEventKey* event);
    bool KeyReleaseEvent(GdkEventKey* event);
    void RegisterKeyEvent(GdkEventKey* event, bool pressed);
    
    bitmask<Input> next = 0;
};

