
#include "signal.hpp"

sigc::signal1<void, Crossover*> signal_crossover_selected;

sigc::signal1<void, std::shared_ptr<driver_list const>> signal_drivers_loaded;

sigc::signal1<void, enclosure*> signal_box_selected;
sigc::signal1<void, enclosure*> signal_add_to_boxlist;
sigc::signal3<void, enclosure*, driver*, Gdk::Color&> signal_add_plot;
sigc::signal1<void, enclosure*> signal_box_modified;
sigc::signal2<int, std::vector<gspk::point>&, Gdk::Color&> signal_add_box_plot;
sigc::signal1<void, int> signal_remove_box_plot;
sigc::signal1<void, int> signal_hide_box_plot;
sigc::signal1<void, int> signal_select_plot;
sigc::signal0<void> signal_net_modified_by_wizard;
sigc::signal1<void, filter_network*> signal_net_modified_by_user;
sigc::signal1<void, int> signal_new_crossover;
sigc::signal0<void> signal_plot_crossover;
sigc::signal4<int, std::vector<gspk::point> const&, Gdk::Color const&, int&, filter_network*>
    signal_add_crossover_plot;
sigc::signal0<void> signal_save_open_files;
