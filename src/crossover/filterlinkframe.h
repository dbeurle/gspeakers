/*
  $Id$

  filterlinkframe Copyright (C) 2002 Daniel Sundberg

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

#ifndef __FILTER_LINK_FRAME_H
#define __FILTER_LINK_FRAME_H

#include "crossover.h"
#include "gspeakersplot.h"
#include "speakerlist.h"

#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>
#include <gtkmm/spinbutton.h>

class FilterLinkFrame : public Gtk::Frame {
public:
  FilterLinkFrame(Net* net, const std::string& description, SpeakerList* speaker_list);

  ~FilterLinkFrame() override;

private:
  /* callbacks */
  void on_order_selected(Gtk::ComboBoxText const* order_box, Gtk::ComboBoxText* type_box);
  void on_param_changed();
  void on_net_updated(Net* net);
  void on_plot_crossover();
  void on_clear_and_plot();
  void on_speakerlist_loaded(SpeakerList* speaker_list);
  void on_settings_changed(const std::string& s);

private:
  void connect_signals();

  void initialise_speaker_combobox();
  void initialise_dampening();
  void initialise_highpass_filter();
  void initialise_lowpass_filter();

private:
  /*
   * Numerical coefficients for the filter principles
   * net_name_type = NET_BESSEL, ..., net_order = NET_ORDER_1ST, ..., net_type = NET_TYPE_LOWPASS,
   * NET_TYPE_HIGHPASS */
  std::vector<double> get_filter_params(int net_name_type, int net_order, int net_type);

  void set_family(Gtk::ComboBoxText* option_menu, int order, int family);

private:
  Gtk::Adjustment adj;

  Gtk::Label m_label;
  Gtk::VBox m_vbox;
  Gtk::ComboBoxText m_speaker_combo;
  Gtk::CheckButton m_enable_checkbutton;

  /* For lowpass filter */
  Gtk::ComboBoxText* m_lower_order_combo;
  Gtk::ComboBoxText* m_lower_type_combo;
  Gtk::SpinButton* m_lower_co_freq_spinbutton;

  /* For highpass filter */
  Gtk::ComboBoxText* m_higher_order_combo;
  Gtk::ComboBoxText* m_higher_type_combo;
  Gtk::SpinButton* m_higher_co_freq_spinbutton;

  /* For both */
  Gtk::CheckButton m_inv_pol_checkbutton;
  Gtk::SpinButton m_damp_spinbutton;
  Gtk::CheckButton m_imp_corr_checkbutton;
  Gtk::CheckButton m_adv_imp_model_checkbutton;

  Net* m_net;
  std::string m_description;
  SpeakerList* m_speaker_list;
  bool enable_edit{false};
  bool init{true};

  int my_filter_plot_index{-1};
  std::vector<GSpeakers::Point> points;
};

#endif