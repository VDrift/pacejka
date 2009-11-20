#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "derived.h"
#include "configfile.h"
#include "plotter.h"
#include "adjustment.h"

#include <list>
#include <string>
#include <gtkmm.h>
#include <iostream>

class MainWindow : public Gtk::Window
{
	private:
		//handlers
		virtual void on_menu_file_quit();
		
		virtual void on_menu_file_open();
		
		virtual void on_menu_file_save();
		
		virtual void slider_changed()
		{
			plot.get_window()->invalidate(true);
		}
		
		void InitMenu();
		void InitAdjustments(CONFIGFILE & defaults, const std::string & shortname, const std::string & name, Gtk::VBox & box, std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator> & adjustmentmap);
		void InitTooltips();
		void InitPlot();
		void InitAdjustment(const std::string & name, float val, float min, float max);
		void ResetAdjustments();
		void ResetAdjustmentsFor(CONFIGFILE & sourcefile, const std::string & section, const std::string & shortname, bool save_to_file = false);
		void ResetAdjustmentsForAll(CONFIGFILE & sourcefile, const std::string & section, bool save_to_file = false);
		std::string GetFile(bool save);
		
		Glib::RefPtr<Gtk::UIManager> uimanager;
		Glib::RefPtr<Gtk::ActionGroup> actiongroup;
		Gtk::VBox menubox;
		Gtk::HBox mainbox;
		Gtk::VBox adjustparentbox;
		Gtk::Notebook adjustbox;
		Plotter plot;
		
		Gtk::VBox abox;
		Gtk::VBox bbox;
		Gtk::VBox cbox;
		
		std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator> adjustmentmap;
	
	public:
		MainWindow();
};

#endif
