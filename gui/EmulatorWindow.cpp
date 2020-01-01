#include "EmulatorWindow.h"
#include "EmulatorArea.hpp"
#include "../emulator/InputManager.hpp"

EmulatorWindow::EmulatorWindow()
: Gtk::ApplicationWindow()
{
    this->emulator = new Emulator();
    this->emulator->inputs = new InputManager(this);
    this->emulator->RunWithUI(this);
    this->set_default_size(160*2, 144*2);
    
    EmulatorArea* area = new EmulatorArea();
    this->add(*area);
    area->show();
    
    add_action("emulator.pause", sigc::mem_fun(*this, &EmulatorWindow::on_action_pause));
    add_action("emulator.advance", sigc::mem_fun(*this, &EmulatorWindow::on_action_advance));
    //this->emulator->core->LoadGame("/Users/leonardogalli/Code/C/noarch/pokemon_yellow.gbc");
}

void EmulatorWindow::on_action_pause()
{
    if (this->emulator->is_running)
        this->emulator->Pause();
    else
        this->emulator->RunWithUI(this);
}

void EmulatorWindow::on_action_advance()
{
    if (this->emulator->is_running) {
        this->emulator->Pause();
    }
    
    this->emulator->RunFor(1);
}
