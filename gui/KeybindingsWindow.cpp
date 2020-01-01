//
//  KeybindingsWindow.cpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#include "KeybindingsWindow.h"
#include "../emulator/InputManager.hpp"

KeybindingsWindow::KeybindingsWindow()
: Gtk::ApplicationWindow()
{
    set_title("Keybindings");
    set_default_size(400, 200);
    
    //Add the TreeView, inside a ScrolledWindow
    m_ScrolledWindow.add(m_TreeView);
    //Only show the scrollbars when they are necessary:
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    add(m_ScrolledWindow);
    
    //Create the Tree model:
    m_refTreeModel = Gtk::ListStore::create(m_Columns);
    m_TreeView.set_model(m_refTreeModel);
    
    for (auto mapping : InputManager::InputMap) {
        Gtk::TreeModel::Row row = *(m_refTreeModel->append());
        row[m_Columns.m_col_name] = mapping.name;
        row[m_Columns.m_col_accel_key] = mapping.shortcut.get_key();
        row[m_Columns.m_col_accel_mods] = mapping.shortcut.get_mod();
        row[m_Columns.m_col_input] = mapping.input;
    }
    
    m_TreeView.append_column("Name", m_Columns.m_col_name);
    auto cell = Gtk::make_managed<Gtk::CellRendererAccel>();
    cell->property_editable() = true;
    cell->property_accel_mode() = Gtk::CellRendererAccelMode::CELL_RENDERER_ACCEL_MODE_OTHER;
    int cols_count = m_TreeView.append_column("Key", *cell);
    auto pColumn = m_TreeView.get_column(cols_count - 1);
    if(pColumn)
    {
        pColumn->add_attribute(cell->property_accel_key(), m_Columns.m_col_accel_key);
        pColumn->add_attribute(cell->property_accel_mods(), m_Columns.m_col_accel_mods);
    }
    cell->signal_accel_edited().connect(sigc::mem_fun(this, &KeybindingsWindow::OnAccelEdited));
    
    show_all_children();
}

void KeybindingsWindow::OnAccelEdited(const Glib::ustring &path_string, guint accel_key, Gdk::ModifierType accel_mods, guint hardware_keycode)
{
    Gtk::TreePath path(path_string);
    Gtk::TreeModel::iterator iter = m_refTreeModel->get_iter(path);
    if(iter)
    {
        Gtk::TreeModel::Row row = *iter;

        //Put the new value in the model:
        row[m_Columns.m_col_accel_key] = accel_key;
        row[m_Columns.m_col_accel_mods] = accel_mods;
        
        Input inp = row[m_Columns.m_col_input];
        
        auto iter = std::find_if(InputManager::InputMap.begin(), InputManager::InputMap.end(), [inp](auto mapping){
            return mapping.input == inp;
        });
        
        iter->shortcut = Gtk::AccelKey(accel_key, accel_mods);
        Glib::ustring name = row[m_Columns.m_col_name];
        g_message("Update mapping for %s: %s", name.c_str(), Gtk::AccelGroup::name(accel_key, accel_mods).c_str());
    }
    
}

KeybindingsWindow* KeybindingsWindow::create()
{
  // Load the Builder file and instantiate its widgets.
    auto window = new KeybindingsWindow();

    return window;
}
