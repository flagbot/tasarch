#include "EmulatorWindow.h"
#include "EmulatorArea.hpp"
#include "../emulator/InputManager.hpp"

EmulatorWindow::EmulatorWindow()
: Gtk::ApplicationWindow()
{
    this->emulator = new Emulator();
    this->emulator->inputs = new InputManager(this);
    this->set_default_size(160*2, 144*2);
    
    EmulatorArea* area = new EmulatorArea();
    this->add(*area);
    area->show();
    //this->emulator->core->LoadGame("/Users/leonardogalli/Code/C/noarch/pokemon_yellow.gbc");
}
