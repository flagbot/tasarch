//
//  TASMovieEditor.cpp
//  tasarch
//
//  Created by Leonardo Galli on 26.12.19.
//

#include "TASMovieEditor.hpp"
#include "../emulator/Signals.h"

TASMovieEditor::TASMovieEditor(TASMovie* movie) : Gtk::ApplicationWindow(), movie(movie), m_VBox(Gtk::ORIENTATION_VERTICAL), m_Recording("Record Inputs"), m_ButtonBox(Gtk::ORIENTATION_HORIZONTAL), m_Reset("Reset to Start"), m_Reset_from("Reset from selected frame")
{
    SetWindowTitle();
    set_default_size(600, 200);
    
    add(m_VBox);
    
    //Add the TreeView, inside a ScrolledWindow, with the button underneath:
    m_ScrolledWindow.add(m_TreeView);

    //Only show the scrollbars when they are necessary:
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    m_VBox.pack_start(m_ScrolledWindow);
    m_VBox.pack_start(m_ButtonBox, Gtk::PACK_SHRINK);
    
    m_ButtonBox.pack_start(m_Reset);
    m_ButtonBox.pack_start(m_Reset_from);
    m_ButtonBox.pack_start(m_Recording);

    if (movie->is_recording)
        m_Recording.set_active();
    
    m_refTreeModel = TASMovieModel::create(movie);
    m_TreeView.set_model(m_refTreeModel);
    
    m_refTreeModel->FillFromMovie();
    m_refTreeModel->CreateRows(m_TreeView);
    
    Signals::MovieFrameChanged.Connect(sigc::mem_fun(*this, &TASMovieEditor::on_frame_changed));
    
    m_Recording.signal_toggled().connect(sigc::mem_fun(*this, &TASMovieEditor::on_state_btn_changed));
    
    m_Reset.signal_clicked().connect(sigc::mem_fun(*this, &TASMovieEditor::on_reset_to_start));
    
    m_Reset_from.signal_clicked().connect(sigc::mem_fun(*this, &TASMovieEditor::on_reset_from));
    
    m_refTreeModel->signal_row_changed().connect(sigc::mem_fun(*this, &TASMovieEditor::on_row_changed));
    
    add_action("tasmovie.save", sigc::mem_fun(*this, &TASMovieEditor::on_save_movie));
    
    show_all_children();
}

void TASMovieEditor::SetWindowTitle() {
    std::string title = "TASMovie";
    
    if (movie->filepath != "") {
        std::string filename = Glib::path_get_basename(movie->filepath);
        title.append(" - ");
        title.append(filename);
    }
    
    set_title(title);
}

void TASMovieEditor::on_frame_changed(long frame)
{
    auto path = this->m_refTreeModel->PathForFrame(frame);
    this->m_TreeView.scroll_to_row(path);
    this->m_TreeView.set_cursor(path);
}

void TASMovieEditor::on_state_btn_changed()
{
    if (m_Recording.get_active()) {
        this->movie->is_recording = true;
    } else {
        this->movie->is_recording = false;
    }
}

void TASMovieEditor::on_reset_to_start()
{
    this->movie->ResetMovie();
}

void TASMovieEditor::on_reset_from()
{
    auto selected = this->m_TreeView.get_selection();
    auto row = *selected->get_selected();
    long frame = row[this->m_refTreeModel->m_Columns.m_frame_column];
    this->movie->ResetToFrame(frame);
}

void TASMovieEditor::on_row_changed(const Gtk::TreeModel::Path &path, const Gtk::TreeModel::iterator &iter)
{
    auto row = *iter;
    long frame = row[this->m_refTreeModel->m_Columns.m_frame_column];
    bitmask<Input> new_input;
    for (InputMapping inp : InputManager::InputMap) {
        auto m_input_column = this->m_refTreeModel->m_Columns.m_input_columns[inp.input];
        bool is_set = row[m_input_column];
        if (is_set) new_input |= inp.input;
    }
    this->movie->inputs[frame][0] = new_input;
}

void TASMovieEditor::on_save_movie()
{
    Gtk::FileChooserDialog dialog("Choose a path to save the movie",
            Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);

    //Add response buttons the the dialog:
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Save", Gtk::RESPONSE_OK);
    
    auto pattern = "*.tasmovie";
    
    auto gb_filter = Gtk::FileFilter::create();
    gb_filter->add_pattern(pattern);
    
    gb_filter->set_name("TASMovie");
    
    dialog.add_filter(gb_filter);
    
    dialog.set_current_name("my_movie.tasmovie");

    int result = dialog.run();

    //Handle the response:
    switch(result)
    {
      case(Gtk::RESPONSE_OK):
      {
          g_message("Selected file: %s", dialog.get_filename().c_str());
          this->movie->Save(dialog.get_filename());
          g_message("Saved TASMovie!");
          SetWindowTitle();
          break;
      }
      case(Gtk::RESPONSE_CANCEL):
      {
          g_message("Cancel clicked");
          break;
      }
      default:
      {
          g_warning("Unknown response");
          break;
      }
    }
}
