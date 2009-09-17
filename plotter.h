#ifndef PLOTTER_H
#define PLOTTER_H

#include "adjustment.h"
#include "derived.h"
#include "cartire.h"

#include <gtkmm/drawingarea.h>
#include <map>
#include <string>

class Plotter : public Gtk::DrawingArea
{
	private:
		std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator> & adjustmentmap;
		CARTIRE <float> tire;
		
		//returns true if the tire has valid parameters
		bool UpdateTire();
	
	public:
		Plotter(std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator> & newmap);

		virtual bool on_expose_event(GdkEventExpose* event);
};

#endif
