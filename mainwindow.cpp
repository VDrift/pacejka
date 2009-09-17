#include "mainwindow.h"

#include <gtkmm/stock.h>
#include <memory>

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
	CONFIGFILE defaults;
	if (!defaults.Load("default.txt"))
	{
		std::cerr << "Error loading default car file: default.txt" << std::endl;
		assert(0);
	}
	ResetAdjustmentsFor(defaults, "a", adjustmentmap);
	ResetAdjustmentsFor(defaults, "b", adjustmentmap);
	ResetAdjustmentsFor(defaults, "c", adjustmentmap);
}

void MainWindow::ResetAdjustmentsFor(CONFIGFILE & defaults, const std::string & shortname, std::map <std::string, DERIVED <Adjustment>, AdjustmentComparator> & adjustmentmap)
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
			adjustmentmap[*i]->GetAdjustment().set_value(val);
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
