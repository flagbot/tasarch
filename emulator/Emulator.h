#pragma once

#include "Core.hpp"
#include "Signals.h"
#include <gtkmm.h>

class Emulator {
public:
    Emulator();
    
    void LoadCore(std::string filename);
    
    void RunWithUI(Gtk::Widget* widget);
    void RunFor(int frames);
    void Pause();
    
    Core* core;
    InputManager* inputs;
    bool is_running = false;
    
private:
    guint ui_connection = -1;
    Gtk::Widget* ui_widget = nullptr;
    sigc::connection ui_iter;
    
    void RunOneFrame();
    bool Tick(const Glib::RefPtr<Gdk::FrameClock>& frame_clock);
    void Tick2();
};
