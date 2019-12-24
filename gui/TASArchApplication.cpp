#include "TASArchApplication.h"
#include "KeybindingsWindow.h"
#define G_LOG_USE_STRUCTURED 1
#include <glib.h>

TASArchApplication::TASArchApplication()
: Gtk::Application("ch.flagbot.tasarch", Gio::APPLICATION_HANDLES_COMMAND_LINE)
{
}

Glib::RefPtr<TASArchApplication> TASArchApplication::create()
{
  return Glib::RefPtr<TASArchApplication>(new TASArchApplication());
}

EmulatorWindow* TASArchApplication::create_appwindow()
{
  auto appwindow = new EmulatorWindow();

  // Make sure that the application runs for as long this window is still open.
  add_window(*appwindow);

  // Gtk::Application::add_window() connects a signal handler to the window's
  // signal_hide(). That handler removes the window from the application.
  // If it's the last window to be removed, the application stops running.
  // Gtk::Window::set_application() does not connect a signal handler, but is
  // otherwise equivalent to Gtk::Application::add_window().

  // Delete the window when it is hidden.
  appwindow->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this,
    &TASArchApplication::on_hide_window), appwindow));

    main_window = appwindow;
    
  return appwindow;
}

void TASArchApplication::on_startup()
{
    // Call the base class's implementation.
    Gtk::Application::on_startup();

    // Add actions and keyboard accelerators for the application menu.
    add_action("quit", sigc::mem_fun(*this, &TASArchApplication::on_action_quit));
    set_accel_for_action("app.quit", "<Primary>Q");
    add_action("keybindings", sigc::mem_fun(*this, &TASArchApplication::on_action_keybindings));
    //set_accel_for_action("app.keybindings", "<Primary>Q");
    
    auto refBuilder = Gtk::Builder::create();
    try
    {
      refBuilder->add_from_resource("/resources/app_menu.ui");
    }
    catch (const Glib::Error& ex)
    {
        g_error("Error while trying to build menu: %s", ex.what().c_str());
        return;
    }

    auto object = refBuilder->get_object("appmenu");
    auto app_menu = Glib::RefPtr<Gio::MenuModel>::cast_dynamic(object);
    if (app_menu)
        set_app_menu(app_menu);
    else
        g_error("No appmenu object in app_menu.ui!");
    
    auto menubar_obj = refBuilder->get_object("menubar");
    auto menubar = Glib::RefPtr<Gio::MenuModel>::cast_dynamic(menubar_obj);
    if (menubar)
        set_menubar(menubar);
    else
        g_error("No menubar object in app_menu.ui!");
}

void TASArchApplication::on_activate()
{
  // The application has been started, so let's show a window.
  auto appwindow = create_appwindow();
  appwindow->present();
}

void TASArchApplication::on_open(const Gio::Application::type_vec_files& files,
  const Glib::ustring& /* hint */)
{
    EmulatorWindow* appwindow = nullptr;
    auto windows = get_windows();
    if (windows.size() > 0)
      appwindow = dynamic_cast<EmulatorWindow*>(windows[0]);

    if (!appwindow)
      appwindow = create_appwindow();

    appwindow->present();
}

int TASArchApplication::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine> &cmd)
{
    Glib::OptionContext ctx;
    Glib::OptionGroup group("options", "main options");
    
    Glib::OptionEntry core_entry;
    std::string core_filename = Glib::build_filename(cmd->get_cwd(), "cores", "gambatte.core");
    core_entry.set_long_name("core");
    
    group.add_entry_filename(core_entry, core_filename);
    
    Glib::OptionEntry game_entry;
    std::string game_filename = Glib::build_filename(cmd->get_cwd(), "pokemon_yellow.gbc");
    core_entry.set_long_name("game");
    
    group.add_entry_filename(game_entry, game_filename);
    
    ctx.add_group(group);
    
    int argc;
    char **argv = cmd->get_arguments(argc);
    ctx.parse(argc, argv);
    
    activate();
    main_window->emulator->LoadCore(core_filename);
    main_window->emulator->core->LoadGame(game_filename);
    return 0;
}

void TASArchApplication::on_hide_window(Gtk::Window* window)
{
  delete window;
}

void TASArchApplication::on_action_quit()
{
  // Gio::Application::quit() will make Gio::Application::run() return,
  // but it's a crude way of ending the program. The window is not removed
  // from the application. Neither the window's nor the application's
  // destructors will be called, because there will be remaining reference
  // counts in both of them. If we want the destructors to be called, we
  // must remove the window from the application. One way of doing this
  // is to hide the window. See comment in create_appwindow().
  auto windows = get_windows();
  for (auto window : windows)
    window->hide();

  // Not really necessary, when Gtk::Widget::hide() is called, unless
  // Gio::Application::hold() has been called without a corresponding call
  // to Gio::Application::release().
  quit();
}

void TASArchApplication::on_action_keybindings()
{
    KeybindingsWindow* window = nullptr;
    auto windows = get_windows();
    if (windows.size() > 0) {
        for (auto iter = windows.begin(); iter != windows.end(); iter++) {
            window = dynamic_cast<KeybindingsWindow*>(*iter);
            if (window) break;
        }
    }

    if (!window) {
        window = new KeybindingsWindow();
        add_window(*window);
    }

    window->present();
}
