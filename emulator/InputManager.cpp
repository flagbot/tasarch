//
//  InputManager.cpp
//  tasarch
//
//  Created by Leonardo Galli on 24.12.19.
//

#include "InputManager.hpp"
#include "Signals.h"
#include <gtkmm/window.h>

#pragma mark InputMapping

InputMapping::InputMapping(Input input, Gtk::AccelKey shortcut, std::string name) : input(input), shortcut(shortcut), name(name)
{
    
}

#pragma mark InputManager

InputManager::InputManager(Gtk::Window* mainWindow)
{
    Signals::CoreLoaded.Connect(sigc::mem_fun(this, &InputManager::CoreLoaded));
    Signals::RetroInputState.Connect(sigc::mem_fun(this, &InputManager::GetInputState));
    Signals::RetroInputPoll.Connect(sigc::mem_fun(this, &InputManager::InputPoll));
    mainWindow->signal_key_press_event().connect(sigc::mem_fun(this, &InputManager::KeyPressEvent));
    mainWindow->signal_key_release_event().connect(sigc::mem_fun(this, &InputManager::KeyReleaseEvent));
}

void InputManager::InputPoll()
{
    current = next;
}

int16_t InputManager::GetInputState(unsigned int port, unsigned int device, unsigned int index, unsigned int id)
{
    if (port != 0) return 0;
    
    return (current & (Input)(1 << id)).bits();
}

bool InputManager::KeyPressEvent(GdkEventKey *event)
{
    RegisterKeyEvent(event, true);
    return false;
}

bool InputManager::KeyReleaseEvent(GdkEventKey *event)
{
    RegisterKeyEvent(event, false);
    return false;
}

void InputManager::RegisterKeyEvent(GdkEventKey *event, bool pressed)
{
    auto found = std::find_if(InputMap.begin(), InputMap.end(), [event](const std::pair<Input, InputMapping>& v){
        return v.second.shortcut.get_key() == event->keyval && v.second.shortcut.get_mod() == event->state;
    });
    if (found != InputMap.end()) {
        auto pair = *found;
        if (pressed) {
            next |= pair.first;
        } else {
            next = next & ~pair.first;
        }
    }
}

void InputManager::CoreLoaded(Core* core)
{
}
