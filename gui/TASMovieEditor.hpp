//
//  TASMovieEditor.hpp
//  tasarch
//
//  Created by Leonardo Galli on 26.12.19.
//

#pragma once

#include <gtkmm.h>
#include "../movie/TASMovie.hpp"
#include "../movie/TASMovieModel.hpp"

class TASMovieEditor : public Gtk::ApplicationWindow
{
public:
    TASMovieEditor(TASMovie* movie);
    
private:
    TASMovie* movie;
    
    //Child widgets:
    Gtk::Box m_VBox;

    Gtk::ScrolledWindow m_ScrolledWindow;
    Gtk::TreeView m_TreeView;
    Glib::RefPtr<TASMovieModel> m_refTreeModel;

    Gtk::CheckButton m_Recording;
    Gtk::ButtonBox m_ButtonBox;
    Gtk::Button m_Reset;
    Gtk::Button m_Reset_from;
    
    void SetWindowTitle();
    
    void on_frame_changed(long frame);
    void on_state_btn_changed();
    void on_reset_to_start();
    void on_reset_from();
    void on_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
    void on_save_movie();
};
