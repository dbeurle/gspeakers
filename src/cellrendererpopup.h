/*
  $Id$

  Copyright (C) 1998-2002 The gtkmm Development Team & Daniel Sundberg

  The file is pretty much ripped right out of the gtkmm-2.0
  example custom cellrenderer with small modifications by
  Daniel Sundberg <dss@home.se>.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __CELLRENDERER_POPUP_H
#define __CELLRENDERER_POPUP_H

#include <gdkmm/rectangle.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/window.h>

class PopupEntry;

class CellRendererPopup : public Gtk::CellRendererText
{
public:
    CellRendererPopup();

    ~CellRendererPopup() override;

    PopupEntry* get_popup_entry();
    Gtk::Window* get_popup_window();

    void set_focus_widget(Gtk::Widget& focus_widget);
    Gtk::Widget* get_focus_widget();

    sigc::signal5<void, const Glib::ustring&, int, int, int, int>& signal_show_popup();
    sigc::signal0<void>& signal_hide_popup();

    void hide_popup();

protected:
    virtual void get_size_vfunc(Gtk::Widget& widget,
                                const Gdk::Rectangle* cell_area,
                                int* x_offset,
                                int* y_offset,
                                int* width,
                                int* height) const;

    Gtk::CellEditable* start_editing_vfunc(GdkEvent* event,
                                           Gtk::Widget& widget,
                                           const Glib::ustring& path,
                                           const Gdk::Rectangle& background_area,
                                           const Gdk::Rectangle& cell_area,
                                           Gtk::CellRendererState flags) override;

    virtual void on_show_popup(const Glib::ustring& path, int x1, int y1, int x2, int y2);

    virtual void on_hide_popup();

private:
    bool on_button_press_event(GdkEventButton* event);
    bool on_key_press_event(GdkEventKey* event);
    void on_style_changed(const Glib::RefPtr<Gtk::Style>& previous_style);

    void on_popup_editing_done();
    void on_popup_arrow_clicked();
    void on_popup_hide();

private:
    using Self = CellRendererPopup;

    sigc::signal5<void, const Glib::ustring&, int, int, int, int> signal_show_popup_;
    sigc::signal0<void> signal_hide_popup_;

    // Cached button width
    mutable int button_width_{-1};

    Gtk::Window popup_window_;
    Gtk::Widget* focus_widget_{nullptr};
    PopupEntry* popup_entry_{nullptr};
    bool shown_{false};
    bool editing_canceled_{false};
};

#endif
