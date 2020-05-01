/*
  crossover_wizard Copyright (C) 2002 Daniel Sundberg

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

#include "crossover_wizard.hpp"

#include "signal.hpp"
#include "common.h"

crossover_wizard::crossover_wizard()
    : Gtk::ScrolledWindow(), m_vbox(Gtk::ORIENTATION_VERTICAL), m_speaker_list{nullptr}
{
    signal_crossover_selected.connect(
        sigc::mem_fun(*this, &crossover_wizard::on_crossover_selected));

    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    set_border_width(2);

    add(m_vbox);

    show_all();

    signal_speakerlist_loaded.connect(
        sigc::mem_fun(*this, &crossover_wizard::on_speaker_list_loaded));
}

void crossover_wizard::on_crossover_selected(Crossover* crossover)
{
#ifndef NDEBUG
    std::puts("crossover_wizard::on_crossover_selected");
#endif

    int index = 0;

    for (auto& network : crossover->networks())
    {
        auto const type = crossover->get_type();

        // The following code is here because we want pretty labels for what
        // was each subfilter is there for
        if (type == CROSSOVER_TYPE_TWOWAY)
        {
            if (index == 0)
            {
                m_vbox.pack_start(*Gtk::manage(new filter_link_frame(&network,
                                                                     _("Woofer/midrange "
                                                                       "filter"),
                                                                     m_speaker_list)));
            }
            else
            {
                m_vbox.pack_start(*Gtk::manage(
                    new filter_link_frame(&network, _("Tweeter filter"), m_speaker_list)));
            }
        }
        else if (type == CROSSOVER_TYPE_THREEWAY)
        {
            m_vbox.pack_start(*Gtk::manage(
                new filter_link_frame(&network,
                                      (index == 0 ? _("Woofer filter")
                                                  : index == 1 ? _("Midrange filter")
                                                               : _("Tweeter filter")),
                                      m_speaker_list)));
        }
        else if (type == (CROSSOVER_TYPE_TWOWAY | CROSSOVER_TYPE_LOWPASS))
        {
            if (index == 0)
            {
                m_vbox.pack_start(*Gtk::manage(
                    new filter_link_frame(&network, _("Woofer filter"), m_speaker_list)));
            }
            else if (index == 1)
            {
                m_vbox.pack_start(*Gtk::manage(new filter_link_frame(&network,
                                                                     _("Woofer/midrange "
                                                                       "filter"),
                                                                     m_speaker_list)));
            }
            else
            {
                m_vbox.pack_start(*Gtk::manage(
                    new filter_link_frame(&network, _("Tweeter filter"), m_speaker_list)));
            }
        }
        else
        {
            m_vbox.pack_start(*Gtk::manage(
                new filter_link_frame(&network, _("filter"), m_speaker_list)));
        }
        ++index;
    }
}

void crossover_wizard::on_speaker_list_loaded(driver_list* driver_list)
{
#ifndef NDEBUG
    std::puts("crossover_wizard::on_speaker_list_loaded");
#endif
    m_speaker_list = driver_list;
}

void crossover_wizard::on_button_plot_clicked()
{
#ifndef NDEBUG
    std::puts("CrossoverWiard::on_button_plot_clicked");
#endif
}

void crossover_wizard::on_button_update_clicked()
{
#ifndef NDEBUG
    std::puts("CrossoverWiard::on_button_update_clicked");
#endif
}
