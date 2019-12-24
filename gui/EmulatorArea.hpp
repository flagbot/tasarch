//
//  EmulatorArea.hpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#pragma once

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include <cairomm/surface.h>
#include "../emulator/Core.hpp"

class EmulatorArea : public Gtk::DrawingArea
{
public:
    EmulatorArea();

    static EmulatorArea* Current;
    
    void VideoRefresh(const void *data, unsigned width,
                       unsigned height, size_t pitch);
    
protected:
    Core* core;
    Cairo::RefPtr<Cairo::ImageSurface> m_surface;
    
    void CoreLoaded(Core* core);
    //Override default signal handler:
    bool Tick(const Glib::RefPtr<Gdk::FrameClock>& frame_clock);
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
};
