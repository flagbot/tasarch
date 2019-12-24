//
//  EmulatorArea.cpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#include "EmulatorArea.hpp"
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <gdkmm/general.h>
#include <sstream>
#include <gdkmm.h>
#include <gdkmm/window.h>
//#include <gdkmm/frameclock.h>
#include "../emulator/Signals.h"

EmulatorArea::EmulatorArea()
{
    Signals::CoreLoaded.Connect(sigc::mem_fun(this, &EmulatorArea::CoreLoaded));
    Signals::RetroVideoRefresh.Connect(sigc::mem_fun(this, &EmulatorArea::VideoRefresh));
    this->add_tick_callback(sigc::mem_fun(*this, &EmulatorArea::Tick));
}

void EmulatorArea::CoreLoaded(Core *core)
{
    this->core = core;
}

bool EmulatorArea::Tick(const Glib::RefPtr<Gdk::FrameClock> &frame_clock)
{
    if (core)
        core->RetroRun();
    return true;
}

void EmulatorArea::VideoRefresh(const void *data, unsigned int width, unsigned int height, size_t pitch)
{
    if (data) {
        int w = width;
        int h = height;
        m_surface = Cairo::ImageSurface::create((unsigned char*)data, Cairo::Format::FORMAT_RGB24, w, h, pitch);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned char* s = (unsigned char*)((guchar*)data + y*pitch + x*4);
                unsigned char red = (s[0] & 0x1f);
                unsigned char green = (s[1] & 0x1f);
                unsigned char blue = s[2] & 0x1f;
                guchar* p = s;
                p[0] = (red << 3) | (red >> 2);
                p[1] = (green << 3) | (green >> 2);
                p[2] = (blue << 3) | (blue >> 2);
            }
        }
        this->queue_draw();
    }
        
}

bool EmulatorArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
    if (m_surface) {
        try {
            auto surface_pattern = Cairo::SurfacePattern::create(m_surface);
            int s_width = m_surface->get_width();
            int s_height = m_surface->get_height();
            surface_pattern->set_filter(Cairo::Filter::FILTER_NEAREST);
            
            auto matrix = Cairo::identity_matrix();
            matrix.scale(s_width / (double)width, s_height / (double)height);
            surface_pattern->set_matrix(matrix);

            cr->set_source(surface_pattern);
            cr->paint();
            cr->set_font_size(14);
            cr->select_font_face("Arial", Cairo::FontSlant::FONT_SLANT_NORMAL, Cairo::FontWeight::FONT_WEIGHT_BOLD);
            cr->set_source_rgb(0.7, 0.7, 0.1);
            cr->move_to(10, 20);
            
            auto curr_frame = this->get_frame_clock()->get_frame_time();
            auto prev_frame = this->get_frame_clock()->get_timings(this->get_frame_clock()->get_frame_counter() - 1)->get_frame_time();
            std::stringstream fps;
            
            fps << (curr_frame - prev_frame) / (1000.0 * 1000.0) << "ms, " << 1 / ((curr_frame - prev_frame) / (1000.0 * 1000.0)) << " fps";
            
            cr->show_text(fps.str());
        } catch(std::exception& ex) {
            g_error("Had exception when painting: %s", ex.what());
            return false;
        }

    }

  return true;
}
