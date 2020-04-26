/*
  $Id$

  crossovertreeview Copyright (C) 2002 Daniel Sundberg

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

#include "crossovertreeview.h"

#include "cell_renderer_popup.hpp"
#include "common.h"
#include "popupentry.h"

#include <iostream>

CrossoverTreeView::CrossoverTreeView() : Gtk::Frame("")
{
    set_border_width(2);
    set_shadow_type(Gtk::SHADOW_NONE);
    static_cast<Gtk::Label*>(get_label_widget())
        ->set_markup("<b>" + Glib::ustring(_("Currently selected crossover")) + "</b>");

    m_ScrolledWindow.set_border_width(12);
    add(m_ScrolledWindow);

    m_ScrolledWindow.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // create model
    create_model();

    // create tree view
    m_TreeView.set_model(m_refTreeStore);
    m_TreeView.set_rules_hint();

    add_columns();
    m_ScrolledWindow.add(m_TreeView);

    signal_crossover_selected.connect(sigc::mem_fun(*this, &CrossoverTreeView::on_crossover_selected));
    signal_net_modified_by_wizard.connect(
        sigc::mem_fun(*this, &CrossoverTreeView::on_net_modified_by_wizard));

    show_all();
}

CrossoverTreeView::~CrossoverTreeView() = default;

void CrossoverTreeView::on_net_modified_by_wizard()
{
    std::puts("CrossoverTreeView::on_net_modified_by_wizard");

    on_crossover_selected(cover);
}

void CrossoverTreeView::on_cell_edited_value(const Glib::ustring& path_string,
                                             const Glib::ustring& new_text)
{
    std::puts("CrossoverTreeView::on_cell_edited_value");
    std::cout << "CrossoverTreeView::on_cell_edited_value: path string = " << path_string << "\n";

    Gtk::TreePath path(path_string);

    if (path.empty())
    {
        return;
    }

    Gtk::TreeRow row = *(m_refTreeStore->get_iter(path));
    row[m_columns.value] = std::atof(new_text.c_str());

#ifndef NDEBUG
    std::cout << "CrossoverTreeView::on_cell_edited_value: path[0:1:2:4] = " << path[0] << ":"
              << path[1] << ":" << path[2] << ":" << path[3] << std::endl;
#endif
    /* Since the stupid signals doesn't seem to work we have to go through the data-containers
       and update values for the particular part we change... */

    /* Index is a counter for the extra circuits (impedance correction network, damping
       network...) we have Number of extra circuits + the crossover is the total number of "base
       nodes" after filter type nodes */
    filter_network* n = &(cover->networks())[path[0]];

    int ndx = 0;
    bool mod = false;
    if (n->get_has_imp_corr())
    {
        // Check if we have edited imp corr
        if (path[1] == ndx)
        {
            // If we have edited imp corr, update the appropriate component
            switch (path[2])
            {
                case 0:
                    n->get_imp_corr_C().set_value(row[m_columns.value]);
                    mod = true;
                    break;
                case 1:
                    n->get_imp_corr_R().set_value(row[m_columns.value]);
                    mod = true;
                    break;
            }
        }
        ndx++;
    }
    if (n->get_has_damp())
    {
        if (path[1] == ndx)
        {
            switch (path[2])
            {
                case 0:
                    n->get_damp_R1().set_value(row[m_columns.value]);
                    mod = true;
                    break;
                case 1:
                    n->get_damp_R2().set_value(row[m_columns.value]);
                    mod = true;
                    break;
            }
        }
        ndx++;
    }
    if (n->get_has_res())
    {
        if (path[1] == ndx)
        {
            switch (path[2])
            {
                case 0:
                    n->get_res_L().set_value(row[m_columns.value]);
                    mod = true;
                    break;
                case 1:
                    n->get_res_C().set_value(row[m_columns.value]);
                    mod = true;
                    break;
                case 2:
                    n->get_res_R().set_value(row[m_columns.value]);
                    mod = true;
                    break;
            }
        }
        ndx++;
    }
    // If we did not modified anything until here, the actual crossover is the circuit to modify
    if (!mod)
    {
        /* check path[2], if this is 0 we edit the part with offset 0
           if path[2] = 1 we have a bandpassfilter and we should add an offset of the
           lowpassfilter order to path[3] which is the index of the highpass part to change */
        if (path[2] == 0)
        {
            n->parts()[path[3]].set_value(row[m_columns.value]);
        }
        else if (path[2] == 1)
        {
            n->parts()[path[3] + n->get_lowpass_order()].set_value(row[m_columns.value]);
        }
    }
#ifndef NDEBUG
    std::cout << "CrossoverTreeView::on_cell_edited_value: Id = " << row[m_columns.id] << '\n';
#endif
    // Tell others that we have modified a part
    signal_net_modified_by_user(n);
}

CrossoverTreeView::ModelColumns::ModelColumns()
{
    add(id_string);
    add(id);
    add(type);
    add(value);
    add(unit);
    add(editable);
    add(visible);
}

void CrossoverTreeView::on_crossover_selected(Crossover* new_crossover)
{
#ifndef NDEBUG
    std::puts("CrossoverTreeView::on_crossover_selected");
#endif

    m_refTreeStore->clear();
    m_vecItems.clear();

    cover = new_crossover;

    for (auto n : cover->networks())
    {
        std::vector<CellItem_Crossover> crossover_elements;
        std::vector<CellItem_Crossover> filter;
        std::vector<CellItem_Crossover> lowpass_filter;
        std::vector<CellItem_Crossover> highpass_filter;
        std::vector<CellItem_Crossover> imp_corr;
        std::vector<CellItem_Crossover> damp;

        if (n.get_has_imp_corr())
        {
            imp_corr.emplace_back(CellItem_Crossover(n.get_imp_corr_C()));
            imp_corr.emplace_back(CellItem_Crossover(n.get_imp_corr_R()));
            crossover_elements.emplace_back(CellItem_Crossover(_("Impedance correction"), imp_corr));
        }
        if (n.get_has_damp())
        {
            damp.emplace_back(CellItem_Crossover(n.get_damp_R1()));
            damp.emplace_back(CellItem_Crossover(n.get_damp_R2()));
            crossover_elements.emplace_back(CellItem_Crossover(_("Damping network"), damp));
        }

        // The rest of the parts
        auto const type = n.get_type();

        std::vector<passive_component> const& parts = n.parts();
        int counter = 0;
        if (type == NET_TYPE_LOWPASS)
        {
            int nof_lowpass_parts = n.get_lowpass_order();

            for (int i = counter; i < nof_lowpass_parts; i++, counter++)
            {
                lowpass_filter.emplace_back(parts[counter]);
            }
            filter.emplace_back(_("Lowpass"), lowpass_filter);
        }
        if (type == NET_TYPE_HIGHPASS)
        {
            for (; (unsigned)counter < parts.size(); counter++)
            {
                highpass_filter.emplace_back(parts[counter]);
            }
            filter.emplace_back(_("Highpass"), highpass_filter);
        }
        crossover_elements.emplace_back(_("Filter"), filter);

        switch (type)
        {
            case NET_TYPE_LOWPASS:
                m_vecItems.emplace_back(_("Lowpass"), crossover_elements);
                break;
            case NET_TYPE_BANDPASS:
                m_vecItems.emplace_back(_("Bandpass"), crossover_elements);
                break;
            case NET_TYPE_HIGHPASS:
                m_vecItems.emplace_back(_("Highpass"), crossover_elements);
                break;
            default:
                m_vecItems.emplace_back(_("Other"), crossover_elements);
        }
    }
    std::for_each(begin(m_vecItems),
                  end(m_vecItems),
                  sigc::mem_fun(*this, &CrossoverTreeView::treestore_add_item));

    m_TreeView.expand_all();

#ifndef NDEBUG
    std::puts("CrossoverTreeView::on_crossover_selected finished");
#endif
}

void CrossoverTreeView::create_model()
{
    m_refTreeStore = Gtk::TreeStore::create(m_columns);

    std::for_each(begin(m_vecItems),
                  end(m_vecItems),
                  sigc::mem_fun(*this, &CrossoverTreeView::treestore_add_item));
}

void CrossoverTreeView::treestore_add_item(const CellItem_Crossover& foo)
{
    Gtk::TreeRow row = *(m_refTreeStore->append());

    row[m_columns.id_string] = foo.m_label;
    row[m_columns.type] = 0;
    row[m_columns.value] = 0.0;
    row[m_columns.unit] = "";
    row[m_columns.editable] = false;
    row[m_columns.id] = 0;
    row[m_columns.visible] = false;

    for (CellItem_Crossover const& child : foo.m_children)
    {
        Gtk::TreeRow child_row = *(m_refTreeStore->append(row.children()));
        child_row[m_columns.id_string] = child.m_label;
        child_row[m_columns.id] = 0;
        child_row[m_columns.visible] = false;
        child_row[m_columns.editable] = false;
        for (CellItem_Crossover const& child2 : child.m_children)
        {
            Gtk::TreeRow child_row2 = *(m_refTreeStore->append(child_row.children()));
            /* If this is the filter parts node */
            if ((child2.m_label == _("Highpass")) || (child2.m_label == _("Lowpass")))
            {
                /* One more node with the filter type */
                child_row2[m_columns.id_string] = child2.m_label;
                child_row2[m_columns.id] = 0;
                child_row2[m_columns.visible] = false;
                child_row2[m_columns.editable] = false;

                /* Then insert the parts */
                for (const CellItem_Crossover& child3 : child2.m_children)
                {
                    Gtk::TreeRow child_row3 = *(m_refTreeStore->append(child_row2.children()));
                    child_row3[m_columns.id_string] = child3.m_label;
                    child_row3[m_columns.id] = child3.m_id;
                    child_row3[m_columns.type] = child3.m_type;
                    child_row3[m_columns.value] = child3.m_value;
                    child_row3[m_columns.unit] = child3.m_unit;
                    child_row3[m_columns.editable] = true;
                    child_row3[m_columns.visible] = true;
                }
            }
            else
            {
                // Impedance correction or damping network
                child_row2[m_columns.id_string] = child2.m_label;
                child_row2[m_columns.id] = child2.m_id;
                child_row2[m_columns.type] = child2.m_type;
                child_row2[m_columns.value] = child2.m_value;
                child_row2[m_columns.unit] = child2.m_unit;
                child_row2[m_columns.editable] = true;
                child_row2[m_columns.visible] = true;
            }
        }
    }
}

void CrossoverTreeView::add_columns()
{
    {
        auto pRenderer = Gtk::manage(new Gtk::CellRendererText());
        pRenderer->property_xalign().set_value(0.0);

        auto const col_cnt = m_TreeView.append_column(_("Id_string"), *pRenderer);
        Gtk::TreeViewColumn* pColumn = m_TreeView.get_column(col_cnt - 1);
        if (pColumn != nullptr)
        {
            pColumn->add_attribute(pRenderer->property_text(), m_columns.id_string);
            pColumn->set_resizable();
        }
    }
    {
        cell_renderer_popup* pRenderer = Gtk::manage(new cell_renderer_popup());
        pRenderer->property_xalign().set_value(0.0);
        pRenderer->signal_edited().connect(
            sigc::mem_fun(*this, &CrossoverTreeView::on_cell_edited_value));

        auto const col_cnt = m_TreeView.insert_column(_("Value"), *pRenderer, -1);

        Gtk::TreeViewColumn* pColumn = m_TreeView.get_column(col_cnt - 1);
        if (pColumn != nullptr)
        {
            pColumn->set_cell_data_func(*pRenderer,
                                        sigc::mem_fun(*this,
                                                      &CrossoverTreeView::value_cell_data_func));

            pColumn->add_attribute(pRenderer->property_editable(), m_columns.editable);
            pColumn->add_attribute(pRenderer->property_visible(), m_columns.visible);
            // Set width to 55 px, looks ok on my screen when we get the spinbutton widget
            pColumn->set_min_width(55);
            pColumn->set_resizable();
        }
    }
    {
        Gtk::CellRendererText* pRenderer = Gtk::manage(new Gtk::CellRendererText());
        pRenderer->property_xalign().set_value(0.0);

        auto const col_cnt = m_TreeView.append_column(_("Unit"), *pRenderer);
        Gtk::TreeViewColumn* pColumn = m_TreeView.get_column(col_cnt - 1);
        if (pColumn != nullptr)
        {
            pColumn->add_attribute(pRenderer->property_text(), m_columns.unit);
            pColumn->add_attribute(pRenderer->property_visible(), m_columns.visible);
            pColumn->set_resizable();
        }
    }
}

void CrossoverTreeView::value_cell_data_func(Gtk::CellRenderer* cell,
                                             const Gtk::TreeModel::iterator& iter)
{
    auto& renderer = dynamic_cast<Gtk::CellRendererText&>(*cell);
    renderer.property_text() = GSpeakers::double_to_ustring((*iter)[m_columns.value], 3, 1);
    renderer.property_xalign() = 1.0;
}

void CrossoverTreeView::on_realize()
{
    m_TreeView.expand_all();

    // call base class:
    Frame::on_realize();
}
