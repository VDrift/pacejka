#include "plotter.h"

#include <vector>

#if defined(WIN32) || defined(_WIN32) || defined (__WIN32) || defined(__WIN32__) \
	|| defined (_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__)
bool isnan(float f)
{return f != f;}
#endif

Plotter::Plotter(std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator> & newmap) : adjustmentmap(newmap)
{
}

bool Plotter::on_expose_event(GdkEventExpose* event)
{
	// This is where we draw on the window
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(window)
	{
		Gtk::Allocation allocation = get_allocation();
		const int width = allocation.get_width();
		const int height = allocation.get_height();

    	// coordinates for the center of the window
		int xc, yc;
		xc = width / 2;
		yc = height / 2;

		Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
		
		// clip to the area indicated by the expose event so that we only redraw
    	// the portion of the window that needs to be redrawn
		cr->rectangle(event->area.x, event->area.y,
					  event->area.width, event->area.height);
		cr->clip();

		//draw curve
		cr->set_line_width(1.0);
	
		float zoom = adjustmentmap["zoom"]->GetValue();
		float camber = adjustmentmap["camber"]->GetValue();
		float load = adjustmentmap["load"]->GetValue();
		zoom = (1.0-zoom)*4.0+0.1;
		load = load * 0.001;
		
		if (UpdateTire())
		{
			{
				//draw lateral line
				cr->set_source_rgb(1.0, 0.0, 0.0);
				float x0 = -zoom*0.5*20.0;
				float xn = zoom*0.5*20.0;
				float ymin = -1000.0;
				float ymax = 1000.0;
				int points = 500;
				
				for (float x = x0; x <= xn; x += (xn-x0)/points)
				{
					float maxforce;
					float yval = tire.Pacejka_Fy(x, load, camber, 1.0, maxforce)/load;
					float xval = width*(x-x0)/(xn-x0);
					yval /= ymax-ymin;
					yval = (yval+1.0)*0.5;
					yval = 1.0 - yval;
					yval *= height;
					if (x == x0)
						cr->move_to(xval, yval);
					else
						cr->line_to(xval, yval);
					//std::cout << xval << ", " << yval << std::endl;
				}
				cr->stroke();
			}
			
			{
				//draw longitudinal line
				cr->set_source_rgb(0.0, 1.0, 0.0);
				float x0 = -zoom*0.5;
				float xn = zoom*0.5;
				float ymin = -1000.0;
				float ymax = 1000.0;
				int points = 500;
				
				for (float x = x0; x <= xn; x += (xn-x0)/points)
				{
					float maxforce;
					float yval = tire.Pacejka_Fx(x, load, 1.0, maxforce)/load;
					float xval = width*(x-x0)/(xn-x0);
					yval /= ymax-ymin;
					yval = (yval+1.0)*0.5;
					yval = 1.0 - yval;
					yval *= height;
					if (x == x0)
						cr->move_to(xval, yval);
					else
						cr->line_to(xval, yval);
					//std::cout << xval << ", " << yval << std::endl;
				}
				cr->stroke();
			}
			
			{
				//draw aligning line
				cr->set_source_rgb(0.0, 0.0, 1.0);
				float x0 = -zoom*0.5*10.0;
				float xn = zoom*0.5*10.0;
				float ymin = -60.0;
				float ymax = 60.0;
				int points = 500;
				
				for (float x = x0; x <= xn; x += (xn-x0)/points)
				{
					float maxforce;
					float yval = tire.Pacejka_Mz(0, x, load, camber*(180.0/3.141593), 1.0, maxforce)/load;
					float xval = width*(x-x0)/(xn-x0);
					yval /= ymax-ymin;
					yval = (yval+1.0)*0.5;
					yval = 1.0 - yval;
					yval *= height;
					if (x == x0)
						cr->move_to(xval, yval);
					else
						cr->line_to(xval, yval);
					//std::cout << xval << ", " << yval << std::endl;
				}
				cr->stroke();
			}
			
			// draw grid lines
			cr->set_line_width(1.0);
			cr->set_source_rgb(0.0, 0.0, 0.0);
			cr->move_to(xc, 0);
			cr->line_to(xc, yc*2);
			cr->move_to(0, yc);
			cr->line_to(xc*2, yc);
			cr->stroke();
		}
	}

	return true;
}

bool Plotter::UpdateTire()
{
	std::map <char, std::vector <float> > params;
	for (std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator>::const_iterator i = adjustmentmap.begin(); i != adjustmentmap.end(); i++)
	{
		if (i->first != "camber")
			params[i->first[0]].push_back(i->second->GetValue());
	}
	tire.SetPacejkaParameters(params['b'], params['a'], params['c']);
	/*for (int i = 0; i < params['b'].size(); i++)
		std::cout << "b" << i << ": " << params['b'][i] << std::endl;*/
	tire.CalculateSigmaHatAlphaHat();
	
	return (params['b'][0] != 0);
}
