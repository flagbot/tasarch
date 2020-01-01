#pragma once

#include <gtkmm.h>
#include "../emulator/Emulator.h"

class EmulatorWindow : public Gtk::ApplicationWindow
{
public:
    EmulatorWindow();
    
    Emulator* emulator;
    
    void on_action_pause();
    void on_action_advance();
};
