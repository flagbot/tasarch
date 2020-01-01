//
//  Emulator.cpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#include "Emulator.h"

Emulator::Emulator()
{
    
}

void Emulator::LoadCore(std::string filename)
{
    core = Core::LoadCore(filename);
    Signals::CoreLoaded.Emit(core);
}

void Emulator::RunWithUI(Gtk::Widget* widget)
{
    if (!is_running) {
        is_running = true;
        this->ui_connection = widget->add_tick_callback(sigc::mem_fun(this, &Emulator::Tick));
        this->ui_widget = widget;
    }
}

void Emulator::RunFor(int frames)
{
    is_running = true;
    for (int i = 0; i < frames; i++) {
        this->RunOneFrame();
    }
    is_running = false;
}

void Emulator::Pause()
{
    if (is_running && this->ui_connection >= 0) {
        this->ui_widget->remove_tick_callback(this->ui_connection);
        this->ui_connection = -1;
        is_running = false;
    }
}

bool Emulator::Tick(const Glib::RefPtr<Gdk::FrameClock> &frame_clock)
{
    this->RunOneFrame();
    return true;
}

void Emulator::RunOneFrame()
{
    // if no core, just exit
    if (!core) return;
    // First we check if we should reset the console!
    if (inputs->current & Input::RESET) {
        core->RetroReset();
    }
    
    // Then we render one frame
    core->RetroRun();
    
    Signals::FrameRendered.Emit();
}
