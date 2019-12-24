//
//  KeybindingsWindow.hpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#ifndef KeybindingsWindow_hpp
#define KeybindingsWindow_hpp

#include <gtkmm.h>

class KeybindingsWindow : public Gtk::ApplicationWindow
{
public:
    KeybindingsWindow();
     KeybindingsWindow(BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>& refBuilder);

      static KeybindingsWindow* create();

      void open_file_view(const Glib::RefPtr<Gio::File>& file);

protected:
      Glib::RefPtr<Gtk::Builder> m_refBuilder;
};


#endif /* KeybindingsWindow_hpp */
