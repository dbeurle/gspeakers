/* 
 * simtoolbar.cc
 *
 * Copyright (C) 2001 Daniel Sundberg <dss@home.se>
 *
 * http://sumpan.campus.luth.se/software/gspeakers
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <vector>
#include <string>
#include <math.h>
#include <gnome--/about.h>
#include "simtoolbar.h"
#include "../config.h"

using SigC::bind;

/*
 * This is the constructor that sets up the toolbar
 */
SimToolbar::SimToolbar(SpeakerToolbar *stoolbar, BoxToolbar *btoolbar, 
		       GSpeakersPlot *gplot, BoxList *iblist, 
		       GSpeakersCFG *icfg) : Gtk::HandleBox() {
  sbar = stoolbar;
  bbar = btoolbar;
  plot = gplot;
  cfg = icfg;
  red = 0;
  green = 0;
  blue = 1.0;
  last_color = COLOR_BLUE;

  color = new Gdk_Color();
  color->set_rgb_p( red, green, blue );
  blist = iblist;

  /* Toolbar buttons */
  sim_button = manage( new Gtk::Button( "Plot" ) );
  sim_button->clicked.connect( slot( this, &SimToolbar::sim_clicked ) );
  rem_plot_button = manage( new Gtk::Button( "Remove Plot" ) );
  rem_plot_button->clicked.connect( slot( this, &SimToolbar::rem_plot_clicked ) );
  rem_all_button = manage( new Gtk::Button( "Remove All Plots" ) );
  rem_all_button->clicked.connect( slot( this, &SimToolbar::rem_all_clicked ) );

  opt_box_button = manage( new Gtk::Button( "Opt box" ) );
  opt_box_button->clicked.connect( slot( this, &SimToolbar::opt_box_clicked ) );
  cfg_button = manage( new Gtk::Button( "Config" ) );
  cfg_button->clicked.connect( slot( this, &SimToolbar::cfg_clicked ) );
  about_button = manage( new Gtk::Button( "About" ) );
  about_button->clicked.connect( slot( this, &SimToolbar::about_clicked ) );

  /* The horizontal box */
  hbox = manage( new Gtk::HBox() );
  add( *hbox );
  hbox->pack_start( *sim_button, false, true );
  hbox->pack_start( *rem_plot_button, false, true );
  hbox->pack_start( *rem_all_button, false, true );
  hbox->pack_start( *opt_box_button, false, true );
  hbox->pack_start( *cfg_button, false, true );
  hbox->pack_start( *about_button, false, true );
}

SimToolbar::~SimToolbar() {
  delete color;
}

void SimToolbar::cfg_clicked() {
  cout << "config clicked" << endl;
}

void SimToolbar::on_cfg_ok() {
  cout << "cfg ok" << endl;
}

void SimToolbar::on_cfg_cancel() {
  cout << "cfg cancel" << endl;
}

/*
 * Plot a curve with current enclosure and speaker
 */
void SimToolbar::sim_clicked() {

  Box *b = bbar->get_box();
  Speaker *s = sbar->get_speaker();
  set_new_color();

  plot->add_plot( calc_dbmag( b, s ), color );
  
  /* Setup vector for clist-row */
  vector<string> v;
  /* First clist entry should be blank (this is where we show the color) */
  v.push_back( "" );
  switch ( b->type ) {
  case SEALED:
    v.push_back( "Sealed" );
    break;
  case PORTED:
    v.push_back( "Ported" );
    break;
  }
  v.push_back( g_strdup_printf("%5.1f", b->vol1 ) );
  v.push_back( g_strdup_printf("%5.1f", b->fb1 ) );
  v.push_back( s->name );
  v.push_back( b->name );
  blist->add_row( v, color );
}

/*
 * Remove selected plots from plot-window and from clist (boxlist)
 */
void SimToolbar::rem_plot_clicked() {
  /* Get plot index from clist */
  int plot_nr = blist->remove_selected_row();
  plot->remove_plot( plot_nr );
}

/*
 * Remove all plots from graph
 */
void SimToolbar::rem_all_clicked() {
  blist->clear();
  plot->remove_all_plots();
}

/*
 * Calculate optimum enclosure volume for current speaker.
 *
 * This function sets the vol and fb on the speaker toolbar.
 *
 * Note:
 * This should be implemented for every new enclosure type.
 */
void SimToolbar::opt_box_clicked() {
  Box *b = bbar->get_box();
  Speaker *s = sbar->get_current_speaker();

  /* Ported box */
  //  vb=20*vas*qts^3.3;
  //  fb=fs*(vas/vb)^0.31;

  /* Sealed box */
  //  qr=(1/qts)/(1/0.707-0.1);
  //  fb=qr*fs;
  //  vr=qr^2-1;
  //  vb=vas/vr;

  switch ( b->type ) {
  case PORTED:
    b->vol1 = 20 * s->vas * pow( s->qts, 3.3 );
    b->fb1  = s->fs * pow( s->vas / b->vol1 , 0.31);
    break;
  case SEALED:
    double qr = ( 1 / s->qts ) / ( 1 / 0.707 - 0.1 );
    b->fb1 = qr * s->fs;
    double vr = pow( qr, 2 ) - 1;
    b->vol1 = s->vas / vr;
    break;
  }
  bbar->set_box_data( b->vol1, b->fb1 );
}

/*
 *  Set current color (color used to plot graph)
 *
 *  This function works like somthing near what i want, however, this might be a 
 *  config option in the future with user configurable custom colors.
 *
 *  If YOU look at this and have some spare time, please fix it...:-)
 *  I'm not really sure of what this piece of code actually does, however, it 
 *  seems to rotate the colors pretty nice. Do not try to understand what I 
 *  want to do with the following code, i've not got a clue...
 */
void SimToolbar::set_new_color() {
  switch ( last_color ) {
  case  COLOR_BLUE:
    if ( green == 1.0 ) {
      green = 0;
      if ( red == 1.0 ) {
	red = 0;
      }
    } else { 
      green = green + 0.2;
    }
    last_color = COLOR_GREEN;
    if ( ( red == 1.0 ) || ( green == 1.0 ) || ( blue == 1.0 ) ) {
      green = 0;
    }
    break;
  case  COLOR_GREEN:
    if ( red == 1.0 ) {
      red = 0;
    } else {
      red = red + 0.2;
      blue = blue + 0.2;
    }
    last_color = COLOR_RED;
    if ( ( red == 1.0 ) || ( green == 1.0 ) || ( blue == 1.0 ) ) {
      green = 0;
    }
    break;
  case  COLOR_RED:
    if ( blue == 1.0 ) {
      blue = 0;
    } else {
      green = green + 0.2;
      blue = blue + 0.2;
    }
    last_color = COLOR_BLUE;
    if ( ( red == 1.0 ) || ( green == 1.0 ) || ( blue == 1.0 ) ) {
      blue = 0;
    }
    break;
  }
  color->set_rgb_p( red, green, blue );
}

void SimToolbar::about_clicked() {
  vector<string> s;
  s.push_back("Daniel Sundberg <dss@home.se>");
  Gnome::About *a = manage( new Gnome::About( "GnomeSpeakers", VERSION, "(C) Daniel Sundberg 2001", s ) );
  a->run_and_close();
}

/*
 * This is a helper-function that calculates the frequency response for this 
 * speaker in this particular box.
 */
double *SimToolbar::calc_dbmag( Box *b, Speaker *s ) {
  double *db_mag = new double[UPPER_LIMIT];
  double A, B, C, D, fn2, fn4, fr, vr, qr, qb;
  
  /* Calculate the frequency response for the supplied speaker and box */
  switch ( b->type ) {
  case PORTED:
    for (int f = 1; f < UPPER_LIMIT; f++) {
      A = pow( ( b->fb1 / s->fs ), 2 );
      B = A / s->qts + b->fb1 / ( 7 * s->fs * s->qts );
      C = 1 + A + ( s->vas / b->vol1 ) + b->fb1 / ( 7 * s->fs * s->qts );
      D = 1 / s->qts + b->fb1 / ( 7 * s->fs );
      fn2 = pow( ( f / s->fs ), 2 );
      fn4 = pow( fn2, 2 );
      db_mag[f] = 10 * log10( pow( fn4, 2 ) / ( pow( ( fn4 - C * fn2 + A ), 2 ) + 
						fn2 * pow( ( D * fn2 - B), 2 ) ) );
    }
    break;
  case SEALED:
    for (int f = 1; f < UPPER_LIMIT; f++) {
      fr = pow( ( f / b->fb1 ), 2 );
      vr = s->vas / b->vol1;
      qr = sqrt( vr + 1 );
      qb = 1 / ( ( 1 / s->qts ) / qr + 0.1 );
      db_mag[f] = 10 * log10( pow( fr, 2 ) / ( pow( ( fr - 1 ), 2 ) + fr / ( pow( qb, 2 ) ) ) );
    }
    break;
  }
  return db_mag;
}
