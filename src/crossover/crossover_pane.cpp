/*
  $Id$

  crossoverpaned Copyright (C) 2002-2004 Daniel Sundberg

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

#include "crossover_pane.hpp"

#include <gtkmm/separatormenuitem.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/stock.h>

#include <iostream>

constexpr auto MENU_INDEX_SAVE = 6;
constexpr auto TOOLBAR_INDEX_SAVE = 3;

crossover_pane::crossover_pane()
{
    g_settings.settings_changed.connect(sigc::mem_fun(*this, &crossover_pane::on_settings_changed));

    add1(m_crossover_notebook);
    g_settings.defaultValueUnsignedInt("crossover_pane1Position", 220);
    set_position(g_settings.getValueUnsignedInt("crossover_pane1Position"));
    add2(m_plot_notebook);

    m_crossover_notebook.append_page(m_crossover_wizard, _("Wizard"));
    m_crossover_notebook.append_page(m_crossover_treeview, _("Filter"));
    m_crossover_notebook.append_page(m_crossover_history, _("History"));

    m_plot_notebook.append_page(m_filter_plot, _("Crossover frequency response"));
    m_plot_notebook.append_page(m_total_filter_plot, _("Total crossover frequency response"));
    m_plot_notebook.append_page(m_summed_freq_resp_plot, _("Total frequency response"));
    m_plot_notebook.append_page(m_crossover_image_view, _("Crossover network"));

    m_plot_notebook.set_scrollable();

    signal_crossover_set_save_state.connect(sigc::mem_fun(*this, &crossover_pane::set_save_state));
}

void crossover_pane::select_first_crossover() { m_crossover_history.select_first_row(); }

crossover_pane::~crossover_pane()
{
    g_settings.setValue("crossover_pane1Position", get_position());

    try
    {
        g_settings.save();
    }
    catch (std::runtime_error const& error)
    {
        std::cout << "crossover_pane::~crossover_pane: " << error.what() << "\n";
    }
}

Gtk::MenuItem& crossover_pane::get_menu()
{
    m_menu_item.set_label("Crossover");

    // Create new crossover submenu
    auto crossover_menu = Gtk::manage(new Gtk::Menu());

    {
        auto crossover_submenu = Gtk::manage(new Gtk::MenuItem("New"));

        auto crossover_types_menu = Gtk::manage(new Gtk::Menu());

        crossover_submenu->set_submenu(*crossover_types_menu);

        {
            auto filter = Gtk::manage(new Gtk::MenuItem("Lowpass"));
            filter->signal_activate().connect(
                sigc::bind(sigc::mem_fun(*this, &crossover_pane::on_new_crossover_menu_action),
                           CROSSOVER_TYPE_LOWPASS));
            crossover_types_menu->append(*filter);
        }
        {
            auto filter = Gtk::manage(new Gtk::MenuItem("Subsonic"));
            filter->signal_activate().connect(
                sigc::bind(sigc::mem_fun(*this, &crossover_pane::on_new_crossover_menu_action),
                           CROSSOVER_TYPE_SUBSONIC));
            crossover_types_menu->append(*filter);
        }
        {
            auto filter = Gtk::manage(new Gtk::MenuItem("Highpass"));
            filter->signal_activate().connect(
                sigc::bind(sigc::mem_fun(*this, &crossover_pane::on_new_crossover_menu_action),
                           CROSSOVER_TYPE_HIGHPASS));
            crossover_types_menu->append(*filter);
        }
        {
            auto filter = Gtk::manage(new Gtk::MenuItem("2 way"));
            filter->signal_activate().connect(
                sigc::bind(sigc::mem_fun(*this, &crossover_pane::on_new_crossover_menu_action),
                           CROSSOVER_TYPE_TWOWAY));
            crossover_types_menu->append(*filter);
        }
        {
            auto filter = Gtk::manage(new Gtk::MenuItem("2.5 way"));
            filter->signal_activate().connect(
                sigc::bind(sigc::mem_fun(*this, &crossover_pane::on_new_crossover_menu_action),
                           CROSSOVER_TYPE_LOWPASS | CROSSOVER_TYPE_TWOWAY));
            crossover_types_menu->append(*filter);
        }
        {
            auto filter = Gtk::manage(new Gtk::MenuItem("3 way"));
            filter->signal_activate().connect(
                sigc::bind(sigc::mem_fun(*this, &crossover_pane::on_new_crossover_menu_action),
                           CROSSOVER_TYPE_THREEWAY));
            crossover_types_menu->append(*filter);
        }
        {
            auto filter = Gtk::manage(new Gtk::MenuItem("4 way"));
            filter->signal_activate().connect(
                sigc::bind(sigc::mem_fun(*this, &crossover_pane::on_new_crossover_menu_action),
                           CROSSOVER_TYPE_FOURWAY));
            crossover_types_menu->append(*filter);
        }
        crossover_menu->append(*crossover_submenu);
    }

    {
        auto copy = Gtk::manage(new Gtk::MenuItem("Copy"));
        copy->signal_activate().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_new_copy));
        crossover_menu->append(*copy);
    }
    crossover_menu->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
    {
        auto new_xml = Gtk::manage(new Gtk::MenuItem("New XML"));
        new_xml->signal_activate().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_new_xml));
        crossover_menu->append(*new_xml);
    }
    {
        auto append_xml = Gtk::manage(new Gtk::MenuItem("Append XML"));
        append_xml->signal_activate().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_append_xml));
        crossover_menu->append(*append_xml);
    }
    {
        auto open_xml = Gtk::manage(new Gtk::MenuItem("Open XML"));
        open_xml->signal_activate().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_open_xml));
        crossover_menu->append(*open_xml);
    }
    crossover_menu->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
    {
        auto save = Gtk::manage(new Gtk::MenuItem("Save"));
        save->signal_activate().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_save));
        save->set_sensitive(false);
        crossover_menu->append(*save);
    }
    {
        auto save_as = Gtk::manage(new Gtk::MenuItem("Save As"));
        save_as->signal_activate().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_save_as));
        crossover_menu->append(*save_as);
    }
    crossover_menu->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
    {
        auto delete_item = Gtk::manage(new Gtk::MenuItem("Delete"));
        delete_item->signal_activate().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_remove));
        crossover_menu->append(*delete_item);
    }
    crossover_menu->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
    {
        auto plot = Gtk::manage(new Gtk::MenuItem("Plot"));
        plot->signal_activate().connect(sigc::mem_fun(*this, &crossover_pane::on_plot_crossover));
        crossover_menu->append(*plot);
    }
    m_menu_item.set_submenu(*crossover_menu);

    return m_menu_item;
}

Gtk::Toolbar& crossover_pane::get_toolbar()
{
    if (m_toolbar != nullptr)
    {
        return *m_toolbar;
    }

    m_toolbar = Gtk::manage(new Gtk::Toolbar());

    {
        auto im = Gtk::manage(new Gtk::Image(Gtk::Stock::COPY, Gtk::ICON_SIZE_LARGE_TOOLBAR));
        auto t = Gtk::manage(new Gtk::ToolButton(*im, _("Copy")));
        t->signal_clicked().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_new_copy));
        m_toolbar->append(*t);
    }

    m_toolbar->append(*Gtk::manage(new Gtk::SeparatorToolItem()));

    {
        auto im = Gtk::manage(new Gtk::Image(Gtk::Stock::OPEN, Gtk::ICON_SIZE_LARGE_TOOLBAR));
        auto t = Gtk::manage(new Gtk::ToolButton(*im, _("Open")));
        t->signal_clicked().connect(
            sigc::mem_fun(m_crossover_history, &crossover_history::on_open_xml));
        m_toolbar->append(*t);
    }
    {
        auto im = Gtk::manage(new Gtk::Image(Gtk::Stock::SAVE, Gtk::ICON_SIZE_LARGE_TOOLBAR));
        auto t = Gtk::manage(new Gtk::ToolButton(*im, _("Save")));
        t->signal_clicked().connect(sigc::mem_fun(m_crossover_history, &crossover_history::on_save));
        m_toolbar->append(*t);
    }
    m_toolbar->append(*Gtk::manage(new Gtk::SeparatorToolItem()));
    {
        auto im = Gtk::manage(new Gtk::Image(Gtk::Stock::DELETE, Gtk::ICON_SIZE_LARGE_TOOLBAR));
        auto t = Gtk::manage(new Gtk::ToolButton(*im, _("Delete")));
        t->signal_clicked().connect(sigc::mem_fun(m_crossover_history, &crossover_history::on_remove));
        m_toolbar->append(*t);
    }
    m_toolbar->append(*Gtk::manage(new Gtk::SeparatorToolItem()));
    {
        auto t = Gtk::manage(
            new Gtk::ToolButton(GSpeakers::image_widget("stock_plot_crossover_24.png"), _("Plot")));
        t->signal_clicked().connect(sigc::mem_fun(*this, &crossover_pane::on_plot_crossover));
        m_toolbar->append(*t);
    }
    m_toolbar->set_toolbar_style((Gtk::ToolbarStyle)g_settings.getValueUnsignedInt("ToolbarStyl"
                                                                                   "e"));
    m_toolbar->get_nth_item(TOOLBAR_INDEX_SAVE)->set_sensitive(false);

    g_settings.settings_changed.connect(sigc::mem_fun(*this, &crossover_pane::on_settings_changed));

    return *m_toolbar;
}

void crossover_pane::on_plot_crossover()
{
    m_filter_plot.clear();
    signal_plot_crossover();
}

void crossover_pane::on_settings_changed(const std::string& setting)
{
    if (setting == "ToolbarStyle")
    {
        m_toolbar->set_toolbar_style(
            static_cast<Gtk::ToolbarStyle>(g_settings.getValueUnsignedInt("ToolbarStyle")));
    }
    if (setting == "AutoUpdateFilterPlots" && g_settings.getValueBool("AutoUpdateFilterPlots"))
    {
        on_plot_crossover();
    }
}

void crossover_pane::on_new_crossover_menu_action(int index) { signal_new_crossover(index); }

void crossover_pane::set_save_state(bool state)
{
    if (m_toolbar != nullptr)
    {
        m_toolbar->get_nth_item(TOOLBAR_INDEX_SAVE)->set_sensitive(state);
    }
    if (m_menu_item.has_submenu())
    {
        // FIXME gtk3 port
        // for (auto& child : m_menu_item.get_children())
        // {
        //     if (child->get_name() == "Save")
        //     {
        //         child->set_sensitive(state);
        //     }
        // }
        // m_menu_item.get_children().at(MENU_INDEX_SAVE)->set_sensitive(state);

        GSpeakers::crossoverlist_modified() = state;
    }
}
