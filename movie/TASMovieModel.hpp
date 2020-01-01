//
//  TASMovieModel.hpp
//  tasarch
//
//  Created by Leonardo Galli on 26.12.19.
//

#pragma once

#include <gtkmm.h>
#include "../emulator/InputManager.hpp"
#include "TASMovie.hpp"
#include <unordered_map>

class TASMovieModel : public Gtk::TreeStore
{
protected:
    TASMovieModel(TASMovie* movie);

public:

    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
        {
            add(m_frame_column);
            for (InputMapping inp : InputManager::InputMap) {
                Gtk::TreeModelColumn<bool> m_input_column;
                add(m_input_column);
                m_input_columns[inp.input] = m_input_column;
            }
        }
        Gtk::TreeModelColumn<long> m_frame_column;
        std::unordered_map<Input, Gtk::TreeModelColumn<bool>> m_input_columns;
    };

    ModelColumns m_Columns;

    static Glib::RefPtr<TASMovieModel> create(TASMovie* movie);
    
    void FillFromMovie();
    void FillFromFrame(long frame);
    void FillRowFromInput(Gtk::TreeModel::Row row, bitmask<Input> input, long frame);
    Gtk::TreeModel::Path PathForFrame(long frame);
    void CreateRows(Gtk::TreeView& view);
    void OnMovieFrameChanged(long frame);
    
private:
    TASMovie *movie;
};
