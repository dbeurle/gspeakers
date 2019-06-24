/*
  $Id$

  boxhistory Copyright (C) 2002 Daniel Sundberg

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

#ifndef __GSPEAKERS_BOXHISTORY
#define __GSPEAKERS_BOXHISTORY

#include "enclosure_list.hpp"
#include "common.h"

#include <gdkmm/event.h>

#include <gtkmm/box.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>

#include <sigc++/signal.h>

extern sigc::signal1<void, bool> signal_enclosure_set_save_state;

/*
 * This class implements a TreeView connected with a ListStore
 * It is pretty much ripped off the liststore example in the
 * gtk-demo program so you can study this example to learn a
 * little bit more about how to use the TreeView.
 *
 * The list contains a list of all enclosures...you can select
 * to plot, not plot or whatever for each enclosure.
 */
class enclosure_history : public Gtk::Frame
{
public:
    enclosure_history();

    ~enclosure_history() override;

    /* public callbacks, used in container classes from toolbar/menu events */
    void on_new();
    void on_new_copy();
    void on_open_xml();
    void on_append_xml();
    void on_new_xml();
    void on_save();
    void on_save_as();
    void on_remove();
    void on_delete_plot();

protected:
    /* callbacks */
    void open_xml(const std::string& filename);
    void append_xml(const std::string& filename);
    void on_selection_changed();
    void save_as_xml(const std::string& filename);
    void on_box_modified(enclosure* box);
    void on_add_plot(enclosure* box, driver* speaker);
    void on_append_box(enclosure* box);
    void on_add_to_boxlist(enclosure* b);
    bool on_delete_event(GdkEventAny* event) override;
    void on_plot_selected(int i);
    void on_save_open_files();
    void type_cell_data_func(Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter);
    void vb1_cell_data_func(Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter);
    void fb1_cell_data_func(Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter);

    /* Helper member functions */
    virtual void create_model();
    virtual void add_columns();
    virtual void add_item(enclosure const& box);

protected:
    Gtk::ScrolledWindow m_ScrolledWindow;
    Gtk::TreeView m_TreeView;
    Glib::RefPtr<Gtk::ListStore> m_refListStore;
    Gtk::VBox m_vbox;
    Gtk::Label m_label;

    enclosure_list m_box_list;

    // This is used in the treemodel
    struct ModelColumns : public Gtk::TreeModelColumnRecord
    {
        Gtk::TreeModelColumn<int> type;
        Gtk::TreeModelColumn<Glib::ustring> id_string;
        Gtk::TreeModelColumn<Glib::ustring> speaker;
        Gtk::TreeModelColumn<double> vb1;
        Gtk::TreeModelColumn<double> fb1;
        Gtk::TreeModelColumn<double> vb2;
        Gtk::TreeModelColumn<double> fb2;

        ModelColumns()
        {
            add(type);
            add(id_string);
            add(speaker);
            add(vb1);
            add(vb2);
            add(fb1);
            add(fb2);
        }
    };
    ModelColumns m_columns;

    std::string m_filename;
    bool new_xml_pressed;
    int index;
    int selected_plot;
};

#endif
