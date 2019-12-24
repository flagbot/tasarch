#pragma once

#include <gtkmm.h>
#include "../emulator/Emulator.h"

class EmulatorWindow : public Gtk::ApplicationWindow
{
public:
    EmulatorWindow();
    
    Emulator* emulator;
};
