#ifndef ADJUSTMENT_H
#define ADJUSTMENT_H

#include <gtkmm.h>

class Adjustment
{
	private:
		Gtk::Adjustment adjustment;
		Gtk::HBox box;
		Gtk::SpinButton entry;
		Gtk::Label label;
		Gtk::HScale bar;
		
	public:
		Adjustment(const std::string & name, float value, float min, float max);

		Gtk::HBox & GetBox()
		{
			return box;
		}
		
		float GetValue() const {return adjustment.get_value();}

		Gtk::Adjustment & GetAdjustment()
		{
			return adjustment;
		}
};

struct AdjustmentComparator
{
	bool operator() (std::string first, std::string second) const
	{
		if (first.size() == second.size() || (first.size() > 2 && second.size() > 2))
			return first < second;
		else
			return first.size() < second.size();
	}
};

#endif
