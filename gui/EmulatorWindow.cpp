#include "EmulatorWindow.h"
#include "EmulatorArea.hpp"

EmulatorWindow::EmulatorWindow()
: Gtk::ApplicationWindow()
{
    this->emulator = new Emulator();
    this->override_background_color(Gdk::RGBA("red"));
    this->set_default_size(160*3, 144*3);
    
    EmulatorArea* area = new EmulatorArea();
    this->add(*area);
    area->show();
    this->emulator->LoadCore("/Users/leonardogalli/Code/C/noarch/cores/libgambatte_libretro.dylib");
    this->emulator->core->LoadGame("/Users/leonardogalli/Code/C/noarch/pokemon_yellow.gbc");
}
