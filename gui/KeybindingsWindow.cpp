//
//  KeybindingsWindow.cpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#include "KeybindingsWindow.h"

KeybindingsWindow::KeybindingsWindow(BaseObjectType* cobject,
  const Glib::RefPtr<Gtk::Builder>& refBuilder)
: Gtk::ApplicationWindow(cobject),
  m_refBuilder(refBuilder)
{

}

KeybindingsWindow::KeybindingsWindow()
: Gtk::ApplicationWindow()
{
    set_title("Keybindings");
}


KeybindingsWindow* KeybindingsWindow::create()
{
  // Load the Builder file and instantiate its widgets.
  auto refBuilder = Gtk::Builder::create_from_resource("/resources/keybindings_window.ui");

  KeybindingsWindow* window = nullptr;
  refBuilder->get_widget_derived("keybindings", window);
  if (!window)
    throw std::runtime_error("No \"keybindings\" object in window.ui");

  return window;
}
