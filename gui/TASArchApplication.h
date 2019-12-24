#pragma once

#include <gtkmm.h>
#include "EmulatorWindow.h"

class TASArchApplication: public Gtk::Application
{
protected:
    TASArchApplication();

public:
    static Glib::RefPtr<TASArchApplication> create();

protected:
    // Override default signal handlers:
    void on_startup() override;
    void on_activate() override;
    void on_open(const Gio::Application::type_vec_files& files,
                 const Glib::ustring& hint) override;
    int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine> &cmd) override;
    
private:
    EmulatorWindow* main_window;
    EmulatorWindow* create_appwindow();
    void on_hide_window(Gtk::Window* window);
    void on_action_quit();
    void on_action_keybindings();
};
