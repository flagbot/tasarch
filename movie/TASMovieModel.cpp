//
//  TASMovieModel.cpp
//  tasarch
//
//  Created by Leonardo Galli on 26.12.19.
//

#include "TASMovieModel.hpp"
#include "../emulator/Signals.h"

TASMovieModel::TASMovieModel(TASMovie* movie) : movie(movie)
{
    set_column_types(m_Columns);
    Signals::MovieFrameChanged.Connect(sigc::mem_fun(this, &TASMovieModel::OnMovieFrameChanged));
}

Glib::RefPtr<TASMovieModel> TASMovieModel::create(TASMovie* movie)
{
  return Glib::RefPtr<TASMovieModel>( new TASMovieModel(movie) );
}

void TASMovieModel::FillFromMovie()
{
    for (long frame = 0; frame <= movie->max_frame; frame++) {
        auto cur_frame = movie->inputs[frame];
        if (cur_frame.size()) {
            FillFromFrame(frame);
        }
    }
}

void TASMovieModel::FillFromFrame(long frame)
{
    auto cur_frame = movie->inputs[frame];
    auto row = *(this->append());
    FillRowFromInput(row, cur_frame[0], frame);
    for (int i = 1; i < cur_frame.size(); i++) {
        auto childrow = *(this->append(row.children()));
        FillRowFromInput(childrow, cur_frame[i], i);
    }
}

void TASMovieModel::FillRowFromInput(Gtk::TreeModel::Row row, bitmask<Input> input, long frame)
{
    row[m_Columns.m_frame_column] = frame;
    for (InputMapping inp : InputManager::InputMap) {
        auto m_input_column = m_Columns.m_input_columns[inp.input];
        row[m_input_column] = (bool)(input & inp.input);
    }
}

Gtk::TreeModel::Path TASMovieModel::PathForFrame(long frame)
{
    for (auto iter = this->children().begin(); iter != this->children().end(); iter++) {
        auto row = *iter;
        if (row[m_Columns.m_frame_column] == frame) {
            return this->get_path(iter);
        }
    }
    
    return {};
}

void TASMovieModel::CreateRows(Gtk::TreeView &view)
{
    view.append_column("Frame", m_Columns.m_frame_column);
    for (InputMapping inp : InputManager::InputMap) {
        auto m_input_column = m_Columns.m_input_columns[inp.input];
        view.append_column_editable(inp.name, m_input_column);
    }
}

void TASMovieModel::OnMovieFrameChanged(long frame)
{
    auto path = PathForFrame(frame);
    auto inputs = movie->inputs[frame];
    Gtk::TreeIter iter;
    
    if (inputs.size() > 0) {
        if (!path.empty()) {
            iter = this->get_iter(path);
        } else {
            iter = this->append();
        }
        
        auto row = *iter;

        this->FillRowFromInput(row, inputs[0], frame);
    }
}
