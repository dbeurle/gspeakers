/* crossoverimageview.cc
 *
 * $Id$
 *
 * Copyright (C) 2001-2002 Daniel Sundberg <dss@home.se>
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

#include "crossoverimageview.h"

#include "common.h"

#include <gdkmm/rectangle.h>
#include <pangomm/context.h>

CrossoverImageView::CrossoverImageView()
{
    g_settings.defaultValueBool("ScaleCrossoverImageParts", true);
    m_scale_image_parts = g_settings.getValueBool("ScaleCrossoverImageParts");

    g_settings.settings_changed.connect(
        sigc::mem_fun(*this, &CrossoverImageView::on_settings_changed));

    signal_crossover_selected.connect(
        sigc::mem_fun(*this, &CrossoverImageView::on_crossover_selected));

    signal_net_modified_by_wizard.connect(sigc::mem_fun(*this, &CrossoverImageView::on_net_modified));

    signal_speakerlist_loaded.connect(
        sigc::mem_fun(*this, &CrossoverImageView::on_speakerlist_selected));
}

bool CrossoverImageView::on_expose_event(GdkEventExpose* event)
{
    // get_window()->draw_drawable(get_style()->get_fg_gc(get_state()), m_refPixmap,
    //                             // Only copy the area that was exposed
    //                             event->area.x, event->area.y, event->area.x, event->area.y,
    //                             event->area.width, event->area.height);
    return false;
}

bool CrossoverImageView::on_draw(Cairo::RefPtr<Cairo::Context> const& context)
{
    Gtk::Allocation allocation = get_allocation();
    auto const width = allocation.get_width();
    auto const height = allocation.get_height();

    m_visible = true;
}

bool CrossoverImageView::on_configure_event(GdkEventConfigure* event)
{
    m_visible = true;

    // m_refPixmap = Gdk::Pixmap::create(get_window(), get_allocation().get_width(),
    //                                   get_allocation().get_height(), -1);

    // m_refGC = get_style()->get_fg_gc(get_state());

    // m_refColormap = m_refGC->get_colormap();
    white = Gdk::RGBA("rgb (0.0, 0.0, 0.0)");
    black = Gdk::RGBA("rgb (1.0, 1.0, 1.0)");

    // m_refColormap->alloc_color(white);
    // m_refColormap->alloc_color(black);

    Glib::RefPtr<Pango::Context> refPangoContext = get_pango_context();
    m_refLayout = Pango::Layout::create(refPangoContext);

    redraw();

    // We've handled the configure event, no need for further processing
    return true;
}

void CrossoverImageView::redraw()
{
    if (m_visible)
    {
        auto const& allocation = get_allocation();

        /* Clear to white background color */
        // m_refGC->set_rgb_fg_color(white);
        // m_refPixmap
        //     ->draw_rectangle(m_refGC, true, 0, 0, allocation.get_width(), allocation.get_height());
        // m_refGC->set_rgb_fg_color(black);

        if (m_crossover != nullptr)
        {
            int vert_space_per_net_divider = 0;
            if (m_crossover->get_type() == CROSSOVER_TYPE_LOWPASS)
            {
                vert_space_per_net_divider++;
            }
            if (m_crossover->get_type() == CROSSOVER_TYPE_SUBSONIC)
            {
                vert_space_per_net_divider++;
            }
            if (m_crossover->get_type() == CROSSOVER_TYPE_HIGHPASS)
            {
                vert_space_per_net_divider++;
            }
            if (m_crossover->get_type() == CROSSOVER_TYPE_TWOWAY)
            {
                vert_space_per_net_divider += 2;
            }
            if (m_crossover->get_type() == CROSSOVER_TYPE_THREEWAY)
            {
                vert_space_per_net_divider += 3;
            }
            if (m_crossover->get_type() == CROSSOVER_TYPE_FOURWAY)
            {
                vert_space_per_net_divider += 4;
            }

            if (vert_space_per_net_divider == 0)
            {
                return;
            }

            auto const window_height = allocation.get_height();
            auto const window_width = allocation.get_width();
            auto const vert_space_per_net = std::round(double(window_height)
                                                       / double(vert_space_per_net_divider));

            /* Draw first net here */
            std::vector<Net>& net_vector = *m_crossover->networks();

            for (std::size_t i = 0; i < net_vector.size(); i++)
            {
                int net_vert_divider = 3;
                int net_horz_divider = 2;

                int part_height = std::round(double(vert_space_per_net) / double(net_vert_divider));

                switch (net_vector[i].get_lowpass_order())
                {
                    case NET_ORDER_1ST:
                        net_horz_divider++;
                        break;
                    case NET_ORDER_2ND:
                        net_horz_divider += 2;
                        break;
                    case NET_ORDER_3RD:
                        net_horz_divider += 3;
                        break;
                    case NET_ORDER_4TH:
                        net_horz_divider += 4;
                        break;
                }
                switch (net_vector[i].get_highpass_order())
                {
                    case NET_ORDER_1ST:
                        net_horz_divider++;
                        break;
                    case NET_ORDER_2ND:
                        net_horz_divider += 2;
                        break;
                    case NET_ORDER_3RD:
                        net_horz_divider += 3;
                        break;
                    case NET_ORDER_4TH:
                        net_horz_divider += 4;
                        break;
                }

                auto const lowpass_order = net_vector[i].get_lowpass_order();
                auto const highpass_order = net_vector[i].get_highpass_order();

                if (net_vector[i].get_has_imp_corr())
                {
                    net_horz_divider++;
                }
                if (net_vector[i].get_has_damp())
                {
                    net_horz_divider += 2;
                }

                auto part_width = std::round(window_width / static_cast<double>(net_horz_divider));

                if (m_scale_image_parts)
                {
                    if (part_width > (1.5 * part_height) && net_vector[i].parts().size() <= 4)
                    {
                        part_width = part_height;
                    }
                    else if (part_width > 3 * part_height)
                    {
                        part_width = std::round(1.7 * part_height);
                    }
                }

                draw_connector(0, i * vert_space_per_net, part_width, part_height, true);
                draw_connector(0,
                               i * vert_space_per_net + 2 * part_height,
                               part_width,
                               part_height,
                               false);

                // lowpass part
                std::vector<passive_component> const& part_vector = net_vector[i].parts();
                if (lowpass_order > 0)
                {
                    std::vector<passive_component> lowpass_parts(part_vector.begin(),
                                                                 part_vector.begin() + lowpass_order);
                    draw_lowpass_net(part_width,
                                     i * vert_space_per_net,
                                     part_width,
                                     part_height,
                                     lowpass_parts);
                }

                /* highpass part */
                if (highpass_order > 0)
                {
                    std::vector<passive_component> highpass_parts(part_vector.begin() + lowpass_order,
                                                                  part_vector.begin() + lowpass_order
                                                                      + highpass_order);
                    this->draw_highpass_net(part_width + lowpass_order * part_width,
                                            i * vert_space_per_net,
                                            part_width,
                                            part_height,
                                            highpass_parts);
                }

                int driver_offset = 0;

                if (net_vector[i].get_has_imp_corr())
                {
                    draw_imp_corr_net((1 + lowpass_order + highpass_order) * part_width,
                                      i * vert_space_per_net,
                                      part_width,
                                      part_height,
                                      net_vector[i].get_imp_corr_R(),
                                      net_vector[i].get_imp_corr_C());
                    driver_offset++;
                }
                if (net_vector[i].get_has_damp())
                {
                    draw_damp_net((1 + lowpass_order + highpass_order + driver_offset) * part_width,
                                  i * vert_space_per_net,
                                  part_width,
                                  part_height,
                                  net_vector[i].get_damp_R1(),
                                  net_vector[i].get_damp_R2());
                    driver_offset += 2;
                }
                std::string const& spk = net_vector[i].get_speaker();

                Speaker speaker;

                if (m_speaker_list != nullptr)
                {
                    speaker = m_speaker_list->get_speaker_by_id_string(spk);
                }
                draw_driver((1 + lowpass_order + highpass_order + driver_offset) * part_width,
                            i * vert_space_per_net,
                            part_width,
                            part_height,
                            speaker);
            }
        }
    }
}

void CrossoverImageView::on_settings_changed(const std::string& setting)
{
    if (setting == "ScaleCrossoverImageParts")
    {
        m_scale_image_parts = g_settings.getValueBool("ScaleCrossoverImageParts");

        if (m_visible)
        {
            // redraw();
            Gdk::Rectangle update_rect(0,
                                       0,
                                       get_allocation().get_width(),
                                       get_allocation().get_height());
            get_window()->invalidate_rect(update_rect, true);
        }
    }
}

void CrossoverImageView::on_crossover_selected(Crossover* selected_crossover)
{
    m_crossover = selected_crossover;
    if (m_visible)
    {
        // redraw();
        Gdk::Rectangle update_rect(0, 0, get_allocation().get_width(), get_allocation().get_height());
        get_window()->invalidate_rect(update_rect, true);
    }
}

void CrossoverImageView::on_net_modified()
{
    if (m_crossover != nullptr)
    {
        on_crossover_selected(m_crossover);
    }
}

void CrossoverImageView::on_speakerlist_selected(speaker_list* selected_speaker_list)
{
    m_speaker_list = selected_speaker_list;
    if (m_visible)
    {
        //   redraw();
        Gdk::Rectangle update_rect(0, 0, get_allocation().get_width(), get_allocation().get_height());
        get_window()->invalidate_rect(update_rect, true);
    }
}

void CrossoverImageView::draw_lowpass_net(int x,
                                          int y,
                                          int part_width,
                                          int part_height,
                                          std::vector<passive_component> const& parts)
{
    for (std::size_t i = 0; i < parts.size(); i++)
    {
        if (parts[i].get_type() == PART_TYPE_INDUCTOR)
        {
            draw_inductor(parts[i].get_id(), x + part_width * i, y, part_width, part_height);
            draw_line(x + part_width * i, y + 2 * part_height, part_width, part_height);
        }
        else
        {
            draw_t_cross(x + part_width * i, y, part_width, part_height);
            draw_capacitor(parts[i].get_id(),
                           x + part_width * i,
                           y + part_height,
                           part_width,
                           part_height,
                           true);
            draw_t_cross(x + part_width * i, y + 2 * part_height, part_width, part_height, false);
        }
    }
}

void CrossoverImageView::draw_highpass_net(int x,
                                           int y,
                                           int part_width,
                                           int part_height,
                                           std::vector<passive_component> const& parts)
{
    for (std::size_t i = 0; i < parts.size(); i++)
    {
        if (parts[i].get_type() == PART_TYPE_CAPACITOR)
        {
            draw_capacitor(parts[i].get_id(), x + part_width * i, y, part_width, part_height);
            draw_line(x + part_width * i, y + 2 * part_height, part_width, part_height);
        }
        else
        {
            draw_t_cross(x + part_width * i, y, part_width, part_height);
            draw_inductor(parts[i].get_id(),
                          x + part_width * i,
                          y + part_height,
                          part_width,
                          part_height,
                          true);
            draw_t_cross(x + part_width * i, y + 2 * part_height, part_width, part_height, false);
        }
    }
}

void CrossoverImageView::draw_imp_corr_net(int x,
                                           int y,
                                           int part_width,
                                           int part_height,
                                           passive_component const& capacitor,
                                           passive_component const& resistor)
{
    int local_part_height = std::round(part_height / 2.0);

    draw_t_cross(x, y, part_width, part_height, true);

    draw_capacitor(capacitor.get_id(), x, y + part_height, part_width, local_part_height, true);

    draw_resistor(resistor.get_id(),
                  x,
                  y + part_height + local_part_height,
                  part_width,
                  local_part_height,
                  true);

    draw_t_cross(x, y + 2 * part_height, part_width, part_height, false);
}

void CrossoverImageView::draw_damp_net(int x,
                                       int y,
                                       int part_width,
                                       int part_height,
                                       passive_component const& r1,
                                       passive_component const& r2)
{
    draw_resistor(r1.get_id(), x, y, part_width, part_height, false);
    draw_line(x, y + 2 * part_height, part_width, part_height, false);
    draw_t_cross(x + part_width, y, part_width, part_height, true);
    draw_resistor(r2.get_id(), x + part_width, y + part_height, part_width, part_height, true);
    draw_t_cross(x + part_width, y + 2 * part_height, part_width, part_height, false);
}

void CrossoverImageView::draw_driver(int x, int y, int part_width, int part_height, Speaker const& speaker)
{
    draw_corner(x, y, part_width, part_height, true);

    if (speaker.get_type() == SPEAKER_TYPE_MIDRANGE)
    {
        draw_midrange(x, y + part_height, part_width, part_height);
    }
    else if (speaker.get_type() == SPEAKER_TYPE_BASS)
    {
        draw_woofer(x, y + part_height, part_width, part_height);
    }
    else
    {
        draw_tweeter(x, y + part_height, part_width, part_height);
    }
    draw_corner(x, y + 2 * part_height, part_width, part_height, false);
}

void CrossoverImageView::draw_capacitor(int id, int x, int y, int width, int height, bool rotate)
{
    double half_space_y = std::round(height / 2.0);
    double half_space_x = std::round(width / 2.0);
    double small_space_x = std::round(width / 20.0);
    double small_space_y = std::round(height / 20.0);

    // m_refLayout->set_text("C" + std::to_string(id));
    // m_refPixmap->draw_layout(m_refGC, x, y, m_refLayout);
    //
    // if (rotate) {
    //   // Horizontal line in capacitor
    //   m_refPixmap->draw_line(m_refGC, std::round(x + half_space_x), y, std::round(x +
    //   half_space_x),
    //                          std::round(y + half_space_y - small_space_y));
    //   m_refPixmap->draw_line(m_refGC, std::round(x + half_space_x),
    //                          y + std::round(half_space_y + small_space_y),
    //                          std::round(x + half_space_x), y + height);
    //
    //   // Vertical lines in capacitor
    //   m_refPixmap->draw_line(
    //       m_refGC, std::round(x + 2 * small_space_x), std::round(y + half_space_y - small_space_y),
    //       std::round(x + width - 2 * small_space_x), std::round(y + half_space_y - small_space_y));
    //   m_refPixmap->draw_line(
    //       m_refGC, std::round(x + 2 * small_space_x), std::round(y + half_space_y + small_space_y),
    //       std::round(x + width - 2 * small_space_x), std::round(y + half_space_y + small_space_y));
    // } else {
    //   /* Horizontal line in capacitor */
    //   m_refPixmap->draw_line(m_refGC, x, std::round(y + half_space_y),
    //                          std::round(x + half_space_x - small_space_x),
    //                          std::round(y + half_space_y));
    //   m_refPixmap->draw_line(m_refGC, std::round(x + half_space_x + small_space_x),
    //                          std::round(y + half_space_y), x + width, std::round(y +
    //                          half_space_y));
    //
    //   /* Vertical lines in capacitor */
    //   m_refPixmap->draw_line(
    //       m_refGC, std::round(x + half_space_x - small_space_x), std::round(y + 2 * small_space_y),
    //       std::round(x + half_space_x - small_space_x), std::round(y + height - 2 *
    //       small_space_y));
    //   m_refPixmap->draw_line(
    //       m_refGC, std::round(x + half_space_x + small_space_x), std::round(y + 2 * small_space_y),
    //       std::round(x + half_space_x + small_space_x), std::round(y + height - 2 *
    //       small_space_y));
    // }
}

void CrossoverImageView::draw_inductor(int id, int x, int y, int width, int height, bool rotate)
{
    double half_space_y = height / 2.0;
    double half_space_x = width / 2.0;
    double small_space_x = width / 20.0;
    double small_space_y = height / 20.0;

    // m_refLayout->set_text("L" + std::to_string(id));
    // m_refPixmap->draw_layout(m_refGC, x, y, m_refLayout);
    //
    // if (rotate) {
    //   // Horizontal line in inductor
    //   m_refPixmap->draw_line(m_refGC, std::round(x + half_space_x), y, std::round(x +
    //   half_space_x),
    //                          std::round(y + 2 * small_space_y));
    //   m_refPixmap->draw_line(m_refGC, std::round(x + half_space_x),
    //                          y + height - std::round(2 * small_space_y), std::round(x +
    //                          half_space_x), y + height);
    //
    //   // Arcs in inductor
    //   for (int i = 0; i <= 12; i += 4) {
    //     m_refPixmap->draw_arc(m_refGC, false, std::round(x + half_space_x - 2 * small_space_x),
    //                           y + std::round(2 * small_space_y + i * small_space_y),
    //                           std::round(4 * small_space_x), std::round(4 * small_space_y),
    //                           17280, 11520);
    //   }
    //
    // } else {
    //   // Horizontal line in inductor
    //   m_refPixmap->draw_line(m_refGC, x, std::round(y + half_space_y),
    //                          std::round(x + 2 * small_space_x), std::round(y + half_space_y));
    //   m_refPixmap->draw_line(m_refGC, std::round(x + width - 2 * small_space_x),
    //                          std::round(y + half_space_y), x + width, std::round(y +
    //                          half_space_y));
    //
    //   // Arcs in inductor
    //   for (int i = 0; i <= 12; i += 4) {
    //     m_refPixmap->draw_arc(m_refGC, false, std::round(x + 2 * small_space_x + i *
    //     small_space_x),
    //                           std::round(y + half_space_y - 2 * small_space_y),
    //                           std::round(4 * small_space_x), std::round(4 * small_space_y), 0,
    //                           11520);
    //   }
    // }
}

void CrossoverImageView::draw_resistor(int id, int x, int y, int width, int height, bool rotate)
{
    int half_space_y = std::round(double(height) / 2);
    int half_space_x = std::round(double(width) / 2);
    int small_space_x = std::round(double(width) / 20);
    int small_space_y = std::round(double(height) / 20);

    // m_refLayout->set_text("R" + std::to_string(id));
    // m_refPixmap->draw_layout(m_refGC, x, y, m_refLayout);
    //
    // if (rotate) {
    //   // Horizontal line in resistor
    //   m_refPixmap->draw_line(m_refGC, x + half_space_x, y, x + half_space_x, y + 3 *
    //   small_space_y); m_refPixmap->draw_line(m_refGC, x + half_space_x, y + height - 3 *
    //   small_space_y,
    //                          x + half_space_x, y + height);
    //
    //   // Draw rectangular box in resistor
    //   m_refPixmap->draw_rectangle(m_refGC, false, x + half_space_x - 2 * small_space_x,
    //                               y + 3 * small_space_y, 4 * small_space_x,
    //                               height - 6 * small_space_y);
    //
    // } else {
    //   // Horizontal line in resistor
    //   m_refPixmap->draw_line(m_refGC, x, y + half_space_y, x + (3 * small_space_x), y +
    //   half_space_y); m_refPixmap->draw_line(m_refGC, x + width - (3 * small_space_x), y +
    //   half_space_y, x + width,
    //                          y + half_space_y);
    //
    //   // Draw rectangular box in resistor
    //   m_refPixmap->draw_rectangle(m_refGC, false, x + 3 * small_space_x,
    //                               y + half_space_y - 2 * small_space_y, width - 6 *
    //                               small_space_x, 4 * small_space_y);
    // }
}

void CrossoverImageView::draw_connector(int x, int y, int width, int height, bool positive)
{
    int half_space_x = std::round(double(width) / 2);
    int half_space_y = std::round(double(height) / 2);
    int small_space_x = std::round(double(width) / 20);
    int small_space_y = std::round(double(height) / 20);

    // if (positive) {
    //   m_refLayout->set_text("+");
    // } else {
    //   m_refLayout->set_text("-");
    // }
    // m_refPixmap->draw_layout(m_refGC, x + std::round(half_space_x / 2),
    //                          y + std::round(half_space_y / 2), m_refLayout);
    //
    // // Draw a "connector" cricle
    // m_refPixmap->draw_arc(m_refGC, false, x + half_space_x - 2 * small_space_x,
    //                       y + half_space_y - 2 * small_space_y, 4 * small_space_x, 4 *
    //                       small_space_y, 0, 23040);
    // // Draw line from connector to edge of this component
    // m_refPixmap->draw_line(m_refGC, x + half_space_x + 2 * small_space_x, y + half_space_y, x +
    // width,
    //                        y + half_space_y);
}

void CrossoverImageView::draw_t_cross(int x, int y, int width, int height, bool upper)
{
    // int half_space_x = std::round(double(width) / 2);
    // int half_space_y = std::round(double(height) / 2);
    //
    // m_refPixmap->draw_line(m_refGC, x, y + half_space_y, x + width, y + half_space_y);
    //
    // if (upper) {
    //   m_refPixmap->draw_line(m_refGC, x + half_space_x, y + half_space_y, x + half_space_x,
    //                          y + height);
    // } else {
    //   m_refPixmap->draw_line(m_refGC, x + half_space_x, y, x + half_space_x, y + half_space_y);
    // }
}

void CrossoverImageView::draw_corner(int x, int y, int width, int height, bool upper)
{
    // int half_space_x = std::round(double(width) / 2);
    // int half_space_y = std::round(double(height) / 2);
    //
    // m_refPixmap->draw_line(m_refGC, x, y + half_space_y, x + half_space_x, y + half_space_y);
    //
    // if (upper) {
    //   m_refPixmap->draw_line(m_refGC, x + half_space_x, y + half_space_y, x + half_space_x,
    //                          y + height);
    // } else {
    //   m_refPixmap->draw_line(m_refGC, x + half_space_x, y, x + half_space_x, y + half_space_y);
    // }
}

void CrossoverImageView::draw_line(int x, int y, int width, int height, bool rotate)
{
    // if (rotate) {
    //   int half_space_x = std::round(double(width) / 2);
    //   m_refPixmap->draw_line(m_refGC, x + half_space_x, y, x + half_space_x, y + height);
    // } else {
    //   int half_space_y = std::round(double(height) / 2);
    //   m_refPixmap->draw_line(m_refGC, x, y + half_space_y, x + width, y + half_space_y);
    // }
}

void CrossoverImageView::draw_woofer(int x, int y, int width, int height, bool positive_up)
{
    // int half_space_x = std::round(double(width) / 2);
    // int half_space_y = std::round(double(height) / 2);
    // int small_space_x = std::round(double(width) / 20);
    // int small_space_y = std::round(double(height) / 20);
    //
    // if (positive_up) {
    //   m_refLayout->set_text("+");
    //   m_refPixmap->draw_layout(m_refGC, x + 3 * small_space_x, y + small_space_y, m_refLayout);
    // } else {
    //   m_refLayout->set_text("-");
    //   m_refPixmap->draw_layout(m_refGC, x + 3 * small_space_x, y + small_space_y, m_refLayout);
    // }
    //
    // // Draw a vertical line through the driver
    // m_refPixmap->draw_line(m_refGC, x + half_space_x, y, x + half_space_x, y + height);
    //
    // // Draw the actual woofer
    // m_refPixmap->draw_rectangle(m_refGC, false, x + half_space_x - 4 * small_space_x,
    //                             y + half_space_y - 5 * small_space_y, 4 * small_space_x,
    //                             10 * small_space_y);
    // m_refPixmap->draw_line(m_refGC, x + half_space_x, y + half_space_y - 4 * small_space_y,
    //                        x + half_space_x + 6 * small_space_x, y);
    // m_refPixmap->draw_line(m_refGC, x + half_space_x, y + half_space_y + 4 * small_space_y,
    //                        x + half_space_x + 6 * small_space_x, y + height);
    // m_refPixmap->draw_line(m_refGC, x + half_space_x + 6 * small_space_x, y,
    //                        x + half_space_x + 6 * small_space_x, y + height);
}

void CrossoverImageView::draw_midrange(int x, int y, int width, int height, bool positive_up)
{
    // int half_space_x = std::round(double(width) / 2);
    // int half_space_y = std::round(double(height) / 2);
    // int small_space_x = std::round(double(width) / 20);
    // int small_space_y = std::round(double(height) / 20);
    //
    // if (positive_up) {
    //   m_refLayout->set_text("+");
    //   m_refPixmap->draw_layout(m_refGC, x + 5 * small_space_x, y + 3 * small_space_y, m_refLayout);
    // } else {
    //   m_refLayout->set_text("-");
    //   m_refPixmap->draw_layout(m_refGC, x + 5 * small_space_x, y + 3 * small_space_y, m_refLayout);
    // }
    //
    // // Draw a vertical line through the driver
    // m_refPixmap->draw_line(m_refGC, x + half_space_x, y, x + half_space_x, y + height);
    //
    // // Draw the actual woofer
    // m_refPixmap->draw_rectangle(m_refGC, false, x + half_space_x - 2 * small_space_x,
    //                             y + half_space_y - 3 * small_space_y, 2 * small_space_x,
    //                             6 * small_space_y);
    //
    // m_refPixmap->draw_line(m_refGC, x + half_space_x, y + half_space_y - 2 * small_space_y,
    //                        x + half_space_x + 4 * small_space_x, y + 2 * small_space_y);
    //
    // m_refPixmap->draw_line(m_refGC, x + half_space_x, y + half_space_y + 2 * small_space_y,
    //                        x + half_space_x + 4 * small_space_x, y + height - 2 * small_space_y);
    //
    // m_refPixmap->draw_line(m_refGC, x + half_space_x + 4 * small_space_x, y + 2 * small_space_y,
    //                        x + half_space_x + 4 * small_space_x, y + height - 2 * small_space_y);
}

void CrossoverImageView::draw_tweeter(int x, int y, int width, int height, bool positive_up)
{
    // int half_space_x = std::round(double(width) / 2);
    // int half_space_y = std::round(double(height) / 2);
    // int small_space_x = std::round(double(width) / 20);
    // int small_space_y = std::round(double(height) / 20);
    //
    // if (positive_up) {
    //   m_refLayout->set_text("+");
    //   m_refPixmap->draw_layout(m_refGC, x + 5 * small_space_x, y + 3 * small_space_y, m_refLayout);
    // } else {
    //   m_refLayout->set_text("-");
    //   m_refPixmap->draw_layout(m_refGC, x + 5 * small_space_x, y + 3 * small_space_y, m_refLayout);
    // }
    //
    // /* Draw a vertical line through the driver */
    // m_refPixmap->draw_line(m_refGC, x + half_space_x, y, x + half_space_x, y + height);
    // /* Draw the actual woofer */
    // m_refPixmap->draw_rectangle(m_refGC, false, x + half_space_x,
    //                             y + half_space_y - 3 * small_space_y, 2 * small_space_x,
    //                             6 * small_space_y);
    //
    // m_refPixmap->draw_arc(m_refGC, false, x + half_space_x + small_space_x,
    //                       y + half_space_y - small_space_y, 2 * small_space_x, 2 * small_space_y,
    //                       17280, 11520);
    // m_refPixmap->draw_line(m_refGC, x + half_space_x + 2 * small_space_x,
    //                        y + half_space_y - 5 * small_space_y, x + half_space_x + 2 *
    //                        small_space_x, y + half_space_y + 5 * small_space_y);
}
