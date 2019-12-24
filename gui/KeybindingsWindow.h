//
//  KeybindingsWindow.hpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#ifndef KeybindingsWindow_hpp
#define KeybindingsWindow_hpp

#include <gtkmm.h>
#include "../emulator/InputManager.hpp"

class KeybindingsWindow : public Gtk::ApplicationWindow
{
public:
    KeybindingsWindow();

    static KeybindingsWindow* create();
    
protected:
    // Tree Model Columns
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(m_col_name);
            add(m_col_accel_key);
            add(m_col_accel_mods);
            add(m_col_input);
        }
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<guint> m_col_accel_key;
        Gtk::TreeModelColumn<Gdk::ModifierType> m_col_accel_mods;
        Gtk::TreeModelColumn<Input> m_col_input;
    };
    
    ModelColumns m_Columns;
    
    Gtk::ScrolledWindow m_ScrolledWindow;
    Gtk::TreeView m_TreeView;
    Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
    
    void OnAccelEdited(const Glib::ustring& path_string, guint accel_key, Gdk::ModifierType accel_mods, guint hardware_keycode);
};


#endif /* KeybindingsWindow_hpp */
