#include "adjustment.h"

Adjustment::Adjustment(const std::string & name, float value, float min, float max) :
		adjustment(value, min, max, (max-min)/100.0, (max-min)/10.0, 0), entry(adjustment, (max-min)/100.0, (max-min>10) ? 1 : 2), label(name+": "), bar(adjustment)
{
	bar.set_update_policy(Gtk::UPDATE_CONTINUOUS);
	bar.set_digits(1);
	bar.set_value_pos(Gtk::POS_TOP);
	bar.set_draw_value(false);
	bar.set_size_request(200, 30);
	
	entry.set_snap_to_ticks(false);
	entry.set_numeric(true);
	
	box.pack_start(label, Gtk::PACK_SHRINK);
	box.pack_start(bar);
	box.pack_end(entry, Gtk::PACK_SHRINK);
}