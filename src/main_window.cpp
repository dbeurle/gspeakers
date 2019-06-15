/*
  $Id$

  mainwindow Copyright (C) 2002-2003 Daniel Sundberg

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

#include "main_window.hpp"

#include "crossover.h"
#include "settings_dialog.hpp"

enum NOTEBOOK_PAGE { DRIVERS = 0, ENCLOSURE = 1, FILTER = 2 };

inline bool is_state_modified()
{
    return GSpeakers::driverlist_modified() || GSpeakers::enclosurelist_modified()
           || GSpeakers::crossoverlist_modified() || GSpeakers::measurementlist_modified();
}

main_window::main_window()
{
    m_in_quit_phase = false;

    m_crossover_paned.select_first_crossover();

    this->set_title_and_icons();

    add(m_main_vbox);

    this->set_defaults();

    // You should be able to specify this in the settings dialog, if the window
    // manager can set the size of the window it may as well do it, at least
    // sawfish can do this
    if (g_settings.getValueBool("SetMainWindowSize"))
    {
        resize(g_settings.getValueUnsignedInt("MainWindowWidth"),
               g_settings.getValueUnsignedInt("MainWindowHeight"));
    }
    if (g_settings.getValueBool("SetMainWindowPosition"))
    {
        move(g_settings.getValueUnsignedInt("MainWindowPositionX"),
             g_settings.getValueUnsignedInt("MainWindowPositionY"));
    }

    // Setup the menu
    {
        m_file_menu_item.set_label("File");

        auto file_submenu = Gtk::manage(new Gtk::Menu());

        m_file_menu_item.set_submenu(*file_submenu);

        {
            auto save = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::SAVE));
            save->signal_activate().connect(sigc::mem_fun(*this, &main_window::on_save_all));
            file_submenu->append(*save);
        }
        file_submenu->append(*Gtk::manage(new Gtk::SeparatorMenuItem{}));
        {
            auto quit = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::QUIT));
            quit->signal_activate().connect(sigc::mem_fun(*this, &main_window::on_quit));
            file_submenu->append(*quit);
        }
    }
    {
        m_edit_menu_item.set_label("Edit");

        auto edit_submenu = Gtk::manage(new Gtk::Menu());
        m_edit_menu_item.set_submenu(*edit_submenu);

        auto preferences = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::PREFERENCES));
        preferences->signal_activate().connect(sigc::mem_fun(*this, &main_window::on_edit_settings));
        edit_submenu->append(*preferences);
    }
    {
        m_help_menu_item.set_label("Help");

        auto help_submenu = Gtk::manage(new Gtk::Menu());
        m_help_menu_item.set_submenu(*help_submenu);

        auto help = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::ABOUT));
        help->signal_activate().connect(sigc::mem_fun(*this, &main_window::on_about));
        help_submenu->append(*help);
    }

    m_menubar.append(m_file_menu_item);
    m_menubar.append(m_edit_menu_item);
    m_menubar.append(m_speaker_editor.get_menu());
    m_menubar.append(m_enclosure_paned.get_menu());
    m_menubar.append(m_crossover_paned.get_menu());
    m_menubar.append(m_help_menu_item);

    // FIXME gtk3 port
    // m_file_menu.signal_expose_event().connect(
    //     sigc::mem_fun(*this, &main_window::on_edit_menu_expose_event));

    // Add the MenuBar to the window
    m_main_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);

    // add toolbars
    m_speaker_editor.get_toolbar().hide();
    m_main_vbox.pack_start(m_speaker_editor.get_toolbar(), Gtk::PACK_SHRINK);
    m_enclosure_paned.get_toolbar().hide();
    m_main_vbox.pack_start(m_enclosure_paned.get_toolbar(), Gtk::PACK_SHRINK);
    m_crossover_paned.get_toolbar().hide();
    m_main_vbox.pack_start(m_crossover_paned.get_toolbar(), Gtk::PACK_SHRINK);

    // Add main notebook
    m_main_vbox.pack_start(m_main_notebook);

    this->connect_driver_tab();
    this->connect_enclosure_tab();
    this->connect_crossover_tab();

    show_all_children();

    // For some reason I had to put this after calling show_all_children()

    // FIXME gtk3 port
    // m_main_notebook.signal_switch_page().connect(sigc::mem_fun(*this, &main_window::on_switch_page));

    m_main_notebook.set_current_page(g_settings.getValueUnsignedInt("MainNotebookPage"));
}

void main_window::set_title_and_icons()
{
    try
    {
#ifdef TARGET_WIN32
        Glib::RefPtr<Gdk::Pixbuf> main_icon = Gdk::Pixbuf::create_from_file("gspeakers.png");
#else
        Glib::RefPtr<Gdk::Pixbuf> main_icon = Gdk::Pixbuf::create_from_file(
            std::string(GSPEAKERS_PREFIX) + "/share/pixmaps/gspeakers.png");
#endif
        set_icon(main_icon);
    }
    catch (Gdk::PixbufError const& error)
    {
        std::cout << error.code() << "\n";
    }
    catch (Glib::FileError const& error)
    {
        std::cout << error.code() << "\n";
    }
    set_title("GSpeakers " + std::string(VERSION));
}

void main_window::set_defaults()
{
    g_settings.defaultValueBool("SetMainWindowSize", true);
    g_settings.defaultValueUnsignedInt("MainWindowWidth", 640);
    g_settings.defaultValueUnsignedInt("MainWindowHeight", 480);
    g_settings.defaultValueBool("SetMainWindowPosition", false);
    g_settings.defaultValueUnsignedInt("MainWindowPositionX", 0);
    g_settings.defaultValueUnsignedInt("MainWindowPositionY", 0);
    g_settings.defaultValueBool("AutoUpdateFilterPlots", false);
    g_settings.defaultValueString("SPICECmdLine", "gnucap");
    g_settings.defaultValueBool("SPICEUseNGSPICE", false);
    g_settings.defaultValueBool("SPICEUseGNUCAP", true);
}

void main_window::connect_driver_tab()
{
    // Driver tab
    m_main_notebook.append_page(m_driver_hpaned, *Gtk::manage(new Gtk::Label("Driver")));
    m_driver_hpaned.add1(m_speaker_editor.get_editor_table());
    g_settings.defaultValueUnsignedInt("DriverMainPanedPosition", 400);
    g_settings.defaultValueUnsignedInt("DriverPlotPanedPosition", 250);
    m_driver_hpaned.set_position(g_settings.getValueUnsignedInt("DriverMainPanedPosition"));
    m_driver_vpaned.set_position(g_settings.getValueUnsignedInt("DriverPlotPanedPosition"));
    m_driver_hpaned.add2(m_driver_vpaned);
    m_driver_vpaned.add1(m_speaker_editor.get_plot());
    m_driver_vpaned.add2(m_speaker_editor.get_treeview_table());
}

void main_window::connect_enclosure_tab()
{
    m_main_notebook.append_page(m_enclosure_paned, *Gtk::manage(new Gtk::Label("Enclosure")));
}

void main_window::connect_crossover_tab()
{
    m_main_notebook.append_page(m_crossover_paned, *Gtk::manage(new Gtk::Label("Crossover")));
}

bool main_window::on_delete_event(GdkEventAny* event)
{
    // Popup dialog and ask if user want to save changes
    if (is_state_modified())
    {
        std::puts("main_window::on_quit: opening confirmation dialog");

        Gtk::Dialog dialog("", true);
        dialog.set_border_width(6);
        dialog.get_vbox()->set_spacing(12);

        Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
        dialog.get_vbox()->pack_start(*hbox);
        hbox->set_border_width(6);
        hbox->set_spacing(12);

        auto image = Gtk::manage(new Gtk::Image(Gtk::Stock::DIALOG_WARNING, Gtk::ICON_SIZE_DIALOG));
        hbox->pack_start(*image);

        dialog.get_action_area()->set_border_width(12);
        dialog.get_action_area()->set_spacing(6);

        Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());

        auto label1 = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_START));
        label1->set_markup(Glib::ustring("<b>") + _("Save changes before closing?")
                           + Glib::ustring("</b>\n\n"));
        vbox->pack_start(*label1);

        Gtk::Label* label3 = new Gtk::Label(_("There are unsaved files in GSpeakers. If you choose")
                                                + Glib::ustring("\n")
                                                + _("to quit without saving all changes since last "
                                                    "save")
                                                + Glib::ustring("\n") + _("will be lost."),
                                            Gtk::ALIGN_START);

        vbox->pack_start(*label3);
        hbox->pack_start(*vbox);

        dialog.add_button("Close without saving", 0);
        dialog.add_button(Gtk::Stock::CANCEL, 1);
        dialog.add_button(Gtk::Stock::SAVE, 2);
        dialog.show_all();

        auto const response = dialog.run();

        dialog.hide();

        switch (response)
        {
            case 0:
                break;
            case 1:
                return true;
                break;
            case 2:
                signal_save_open_files();
                break;
            default:
                break;
        }
    }
    on_quit_common();

    return false;
}

void main_window::on_quit()
{
    /* Popup dialog and ask if user want to save changes */
    if (is_state_modified())
    {
        std::puts("main_window::on_quit: opening confirmation dialog");

        auto dialog = std::make_unique<Gtk::Dialog>("", true);

        dialog->set_border_width(6);
        dialog->get_vbox()->set_spacing(12);

        Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
        dialog->get_vbox()->pack_start(*hbox);
        hbox->set_border_width(6);
        hbox->set_spacing(12);

        auto image = Gtk::manage(new Gtk::Image(Gtk::Stock::DIALOG_WARNING, Gtk::ICON_SIZE_DIALOG));
        hbox->pack_start(*image);

        dialog->get_action_area()->set_border_width(12);
        dialog->get_action_area()->set_spacing(6);

        Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());
        Gtk::Label* label1 = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_START));
        label1->set_markup("<b>" + Glib::ustring(_("Save changes before closing?")) + "</b>\n\n");
        vbox->pack_start(*label1);

        Gtk::Label* label3 = Gtk::manage(
            new Gtk::Label(Glib::ustring(_("There are unsaved files. If you choose")) + "\n"
                               + Glib::ustring(_("to quit without saving all changes since last "
                                                 "save"))
                               + "\n" + Glib::ustring(_("will be lost.")),
                           Gtk::ALIGN_START));
        vbox->pack_start(*label3);
        hbox->pack_start(*vbox);

        dialog->add_button(Glib::ustring(_("Close without saving")), 0);
        dialog->add_button(Gtk::Stock::CANCEL, 1);
        dialog->add_button(Gtk::Stock::SAVE, 2);
        dialog->show_all();

        auto const response = dialog->run();

        dialog->hide();

        if (response == 2)
        {
            signal_save_open_files();
        }
    }

    on_quit_common();

    Gtk::Main::quit();
}

void main_window::on_quit_common()
{
    // used to avoid segfault when some widget gets destructed at different times...
    m_in_quit_phase = true;

    g_settings.setValue("DriverMainPanedPosition", m_driver_hpaned.get_position());
    g_settings.setValue("DriverPlotPanedPosition", m_driver_vpaned.get_position());

    // Save window size
    {
        int width, height;
        get_size(width, height);
        g_settings.setValue("MainWindowWidth", width);
        g_settings.setValue("MainWindowHeight", height);
    }
    // Save position
    {
        int pos_x, pos_y;
        get_position(pos_x, pos_y);
        g_settings.setValue("MainWindowPositionX", pos_x);
        g_settings.setValue("MainWindowPositionY", pos_y);
    }

    // Save current notebook page
    g_settings.setValue("MainNotebookPage", m_main_notebook.get_current_page());

    try
    {
        g_settings.save();
    }
    catch (std::runtime_error const& error)
    {
        std::puts("main_window::on_delete_event: could not save settings");
    }
}

void main_window::on_save_all()
{
    std::puts("Saving all files!");

    signal_save_open_files();
}

// FIXME gtk3 port
// bool main_window::on_edit_menu_expose_event(GdkEventExpose* event) {
//   // Check whether to ungrey "save all" menuitem or not
//   m_file_menu.items()[0].set_sensitive(is_state_modified());
//   return false;
// }

// FIXME gtk3 port
// void main_window::on_switch_page(GtkNotebookPage* page, guint page_num) {
//
//   if (!m_in_quit_phase) {
//     switch (page_num) {
//     case NOTEBOOK_PAGE::DRIVERS:
//       if (!m_speaker_editor.get_toolbar().get_visible()) {
//         m_speaker_editor.get_toolbar().show();
//       }
//       if (m_enclosure_paned.get_toolbar().get_visible()) {
//         m_enclosure_paned.get_toolbar().hide();
//       }
//       if (m_crossover_paned.get_toolbar().get_visible()) {
//         m_crossover_paned.get_toolbar().hide();
//       }
//       break;
//     case NOTEBOOK_PAGE::ENCLOSURE:
//       if (m_speaker_editor.get_toolbar().get_visible()) {
//         m_speaker_editor.get_toolbar().hide();
//       }
//       if (!m_enclosure_paned.get_toolbar().get_visible()) {
//         m_enclosure_paned.get_toolbar().show();
//       }
//       if (m_crossover_paned.get_toolbar().get_visible()) {
//         m_crossover_paned.get_toolbar().hide();
//       }
//       break;
//     case NOTEBOOK_PAGE::FILTER:
//       if (m_speaker_editor.get_toolbar().get_visible()) {
//         m_speaker_editor.get_toolbar().hide();
//       }
//       if (m_enclosure_paned.get_toolbar().get_visible()) {
//         m_enclosure_paned.get_toolbar().hide();
//       }
//       if (!m_crossover_paned.get_toolbar().get_visible()) {
//         m_crossover_paned.get_toolbar().show();
//       }
//       break;
//     }
//   }
// }

void main_window::on_about()
{
    Gtk::MessageDialog(*this,
                       "GSpeakers-" + std::string(VERSION)
                           + "\n\n(C) Daniel Sundberg "
                             "<sumpan@sumpan.com>\n\nhttp://gspeakers.sf.net",
                       false,
                       Gtk::MESSAGE_INFO,
                       Gtk::BUTTONS_OK,
                       true)
        .run();
}

void main_window::on_edit_settings() { settings_dialog{}.run(); }
