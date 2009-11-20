#include "mainwindow.h"

#include <gtkmm/stock.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include <memory>

enum DIALOG_OPTIONS 
{OPTION_ONE, OPTION_TWO, OPTION_INVALID};

DIALOG_OPTIONS ShowDialog(const std::string & titletext, const std::string & labeltext, const std::string & buttontext1, const std::string & buttontext2)
{
	Gtk::Label label(labeltext);
	Gtk::Dialog dialog(titletext, true);
	dialog.get_vbox()->pack_start(label, Gtk::PACK_SHRINK);
	dialog.get_vbox()->show_all_children();
	dialog.add_button(buttontext1, Gtk::RESPONSE_YES);
	dialog.add_button(buttontext2, Gtk::RESPONSE_NO);
	int result = dialog.run();
	if (result == Gtk::RESPONSE_YES)
	{
		return OPTION_ONE;
	}
	else if (result == Gtk::RESPONSE_NO)
	{
		return OPTION_TWO;
	}
	else
	{
		return OPTION_INVALID;
	}
}

void ShowErrorDialog(const std::string & errortext)
{
	Gtk::MessageDialog dialog("Error");
	dialog.set_secondary_text(errortext);
	dialog.run();
}

MainWindow::MainWindow() : plot(adjustmentmap)
{
	set_title("Joe's Pacejka Editor");
	set_default_size(700, 500);
	
	InitMenu();
	
	menubox.pack_start(mainbox, Gtk::PACK_EXPAND_WIDGET);
	mainbox.pack_start(adjustparentbox, Gtk::PACK_SHRINK);
	adjustparentbox.pack_start(adjustbox, Gtk::PACK_SHRINK);
	
	CONFIGFILE defaults;
	if (!defaults.Load("default.txt"))
	{
		std::cerr << "Error loading default car file: default.txt" << std::endl;
		assert(0);
	}
	
	InitAdjustments(defaults, "a", "Lateral (a)", abox, adjustmentmap);
	InitAdjustments(defaults, "b", "Longitudinal (b)", bbox, adjustmentmap);
	InitAdjustments(defaults, "c", "Aligning (c)", cbox, adjustmentmap);
	
	InitAdjustment("zoom", 0, 0, 1);
	InitAdjustment("load", 2500, 100, 10000);
	InitAdjustment("camber", 0, -10, 10);
	
	InitTooltips();
	
	InitPlot();
	
	show_all_children();
}

void MainWindow::InitMenu()
{
	add(menubox);
	
	actiongroup = Gtk::ActionGroup::create();

	//File menu:
	actiongroup->add(Gtk::Action::create("FileMenu", "File"));
	actiongroup->add(Gtk::Action::create("FileNew", Gtk::Stock::NEW),
					 sigc::mem_fun(*this, &MainWindow::ResetAdjustments));
	actiongroup->add(Gtk::Action::create("FileOpen", Gtk::Stock::OPEN),
					 sigc::mem_fun(*this, &MainWindow::on_menu_file_open));
	actiongroup->add(Gtk::Action::create("FileSave", Gtk::Stock::SAVE),
					 sigc::mem_fun(*this, &MainWindow::on_menu_file_save));
	actiongroup->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT),
						  sigc::mem_fun(*this, &MainWindow::on_menu_file_quit));
	
	uimanager = Gtk::UIManager::create();
	uimanager->insert_action_group(actiongroup);

	add_accel_group(uimanager->get_accel_group());
	
	//Layout the actions in a menubar and toolbar:
	Glib::ustring ui_info = 
			"<ui>"
			"  <menubar name='MenuBar'>"
			"    <menu action='FileMenu'>"
			"      <menuitem action='FileNew'/>"
			"      <menuitem action='FileOpen'/>"
			"      <menuitem action='FileSave'/>"
			"      <separator/>"
			"      <menuitem action='FileQuit'/>"
			"    </menu>"
			"  </menubar>"
			"</ui>";

	uimanager->add_ui_from_string(ui_info);

	//Get the menubar and toolbar widgets, and add them to a container widget:
	Gtk::Widget* pMenubar = uimanager->get_widget("/MenuBar");
	if(pMenubar)
		menubox.pack_start(*pMenubar, Gtk::PACK_SHRINK);
}

float ClosestBigValue(float somefloat)
{
	float sign = 1.0;
	if (somefloat < 0)
		sign = -1.0;
	float val = somefloat * sign; //make it positive
	if (val < 1)
		val = 1;
	else if (val < 10)
		val = 10;
	else if (val < 100)
		val = 100;
	else if (val < 1000)
		val = 1000;
	else if (val < 5000)
		val = 5000;
	else
		val = val * 10.0;
	
	return val * sign;
}

void MainWindow::InitAdjustments(CONFIGFILE & defaults, const std::string & shortname, const std::string & name, Gtk::VBox & box, std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator> & adjustmentmap)
{
	assert(!shortname.empty());
	std::string section("default");
	std::list <std::string> params;
	defaults.GetParamList(params, section);
	params.sort(AdjustmentComparator());
	for (std::list <std::string>::iterator i = params.begin(); i != params.end(); i++)
	{
		assert(!i->empty());
		if ((*i)[0] == shortname[0])
		{
			std::string paramname = *i;
			float val(0);
			defaults.GetParam(section+"."+paramname, val);
			std::pair <std::map <std::string, DERIVED <Adjustment> >::iterator, bool> result = adjustmentmap.insert(std::make_pair(paramname, DERIVED <Adjustment>()));
			
			float min = -10;
			float max = 10;
			defaults.GetParam("min."+paramname, min);
			defaults.GetParam("max."+paramname, max);
			
			result.first->second = new Adjustment(paramname, val, min, max);
			result.first->second->GetAdjustment().signal_value_changed().connect(sigc::mem_fun(*this,
					&MainWindow::slider_changed));
			box.pack_start(result.first->second->GetBox(), Gtk::PACK_SHRINK);
		}
	}
	
	adjustbox.append_page(box, name);
}

void MainWindow::ResetAdjustments()
{
	DIALOG_OPTIONS result = ShowDialog("New Tire", "Clear current settings and start over?", "Yes", "No");
	if (result == OPTION_TWO)
	{
		return;
	}
	
	CONFIGFILE defaults;
	if (!defaults.Load("default.txt"))
	{
		std::cerr << "Error loading default car file: default.txt" << std::endl;
		assert(0);
	}
	ResetAdjustmentsForAll(defaults, "default");
}

void MainWindow::ResetAdjustmentsForAll(CONFIGFILE & sourcefile, const std::string & section, bool save_to_file)
{
	ResetAdjustmentsFor(sourcefile, section, "a", save_to_file);
	ResetAdjustmentsFor(sourcefile, section, "b", save_to_file);
	ResetAdjustmentsFor(sourcefile, section, "c", save_to_file);
}

void MainWindow::ResetAdjustmentsFor(CONFIGFILE & sourcefile, const std::string & section, const std::string & shortname, bool save_to_file)
{
	assert(!shortname.empty());
	std::list <std::string> params;
	sourcefile.GetParamList(params, section);
	params.sort(AdjustmentComparator());
	for (std::list <std::string>::iterator i = params.begin(); i != params.end(); i++)
	{
		assert(!i->empty());
		if ((*i)[0] == shortname[0])
		{
			std::string paramname = *i;
			if (adjustmentmap.find(*i) != adjustmentmap.end())
			{
				if (save_to_file)
					sourcefile.SetParam(section+"."+paramname, adjustmentmap[*i]->GetValue());
				else
				{
					float val(0);
					sourcefile.GetParam(section+"."+paramname, val);
					adjustmentmap[*i]->GetAdjustment().set_value(val);
				}
			}
		}
	}
}

void MainWindow::InitTooltips()
{
	CONFIGFILE tipfile;
	tipfile.Load("tooltip.txt");
	for (std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator>::iterator i = adjustmentmap.begin(); i != adjustmentmap.end(); i++)
	{
		std::string tooltip;
		tipfile.GetParam(i->first, tooltip);
		i->second->GetBox().set_tooltip_text(tooltip);
	}
}

void MainWindow::InitPlot()
{
	mainbox.pack_start(plot, Gtk::PACK_EXPAND_WIDGET);
}

void MainWindow::InitAdjustment(const std::string & name, float val, float min, float max)
{
	std::pair <std::map <std::string, DERIVED <Adjustment> >::iterator, bool> result = adjustmentmap.insert(std::make_pair(name, DERIVED <Adjustment>()));
	result.first->second = new Adjustment(name, val, min, max);
	result.first->second->GetAdjustment().signal_value_changed().connect(sigc::mem_fun(*this,
										&MainWindow::slider_changed));
	adjustparentbox.pack_start(result.first->second->GetBox(), Gtk::PACK_SHRINK);
}

void MainWindow::on_menu_file_open()
{
	std::string filename = GetFile(false);
	if (filename.size() > 4 && filename.substr(filename.size()-4) == ".car")
	{
		CONFIGFILE car;
		if (!car.Load(filename))
		{
			ShowErrorDialog("Couldn't open file " + filename);
		}
		else
		{
			DIALOG_OPTIONS result = ShowDialog("Load Tire", "Load front or rear tire?", "Front tire", "Rear tire");
			if (result == OPTION_ONE)
			{
				ResetAdjustmentsForAll(car, "tire-front");
			}
			else if (result == OPTION_TWO)
			{
				ResetAdjustmentsForAll(car, "tire-rear");
			}
		}
	}
	else
		ShowErrorDialog("Only VDrift .car format files are supported");
}

void MainWindow::on_menu_file_save()
{
	std::string filename = GetFile(true);
	if (filename.size() > 4 && filename.substr(filename.size()-4) == ".car")
	{
		CONFIGFILE car;
		if (!car.Load(filename))
		{
			ShowErrorDialog("Couldn't open file " + filename + ".\nYou can only save to existing VDrift .car files.");
		}
		else
		{
			DIALOG_OPTIONS result = ShowDialog("Save Tire", "Save to front or rear tire?", "Front tire", "Rear tire");
			if (result == OPTION_ONE)
			{
				ResetAdjustmentsForAll(car, "tire-front", true);
				car.Write();
			}
			else if (result == OPTION_TWO)
			{
				ResetAdjustmentsForAll(car, "tire-rear", true);
				car.Write();
			}
		}
	}
	else
		ShowErrorDialog("Couldn't open file " + filename + "\nOnly saving to existing VDrift .car files is supported.");
}

std::string MainWindow::GetFile(bool save)
{
	Gtk::FileChooserAction action = Gtk::FILE_CHOOSER_ACTION_OPEN;
	std::string prompt = "Please choose a file to open";
	if (save)
	{
		action = Gtk::FILE_CHOOSER_ACTION_SAVE;
		prompt = "Please choose a file to save to";
	}
	Gtk::FileChooserDialog dialog(prompt, action);
	dialog.set_transient_for(*this);

	//Add response buttons the the dialog:
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	//Add filters, so that only certain file types can be selected:

	Gtk::FileFilter filter_any;
	filter_any.set_name("Any files");
	filter_any.add_pattern("*");
	Gtk::FileFilter filter_cars;
	filter_cars.set_name("VDrift .car files");
	filter_cars.add_pattern("*.car");
	dialog.add_filter(filter_cars);
	dialog.add_filter(filter_any);

	//Show the dialog and wait for a user response:
	int result = dialog.run();

	//Handle the response:
	switch (result)
	{
	case (Gtk::RESPONSE_OK):
	{
		return dialog.get_filename();
		break;
	}
	case (Gtk::RESPONSE_CANCEL):
	{
		return "";
		break;
	}
	default:
	{
		return "";
		break;
	}
	}
}

void MainWindow::on_menu_file_quit()
{
	DIALOG_OPTIONS result = ShowDialog("Quit", "Are you sure you want to quit?", "Yes", "No");
	if (result == OPTION_ONE)
	{
		hide();
	}
}
