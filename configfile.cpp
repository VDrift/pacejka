#include "configfile.h"
#include "containeralgorithm.h"

#include <sstream>
using std::stringstream;

#include <algorithm>

#include <string>
using std::string;

#include <map>
using std::map;

#include <list>
using std::list;

#include <iostream>
using std::endl;
using std::ostream;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <cstdlib>
using std::atoi;
using std::atof;

CONFIGFILE::CONFIGFILE()
{
	//vars = NULL;
	filename = "";
	SUPPRESS_ERROR = false;
}

CONFIGFILE::CONFIGFILE(string fname)
{
	//vars = NULL;
	
	SUPPRESS_ERROR = false;
	
	Load(fname);
}

CONFIGFILE::~CONFIGFILE()
{
	Clear();
}

bool CONFIGFILE::GetParam(string param, int & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			ppos++;
			param = param.substr(ppos, param.length() - ppos);
		}
	}
	
	const CONFIGVARIABLE * v = variables.Get(param);
	
	if (!v)
		return false;
	
	outvar = v->val_i;
	
	return true;
}

bool CONFIGFILE::GetParam(string param, bool & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			ppos++;
			param = param.substr(ppos, param.length() - ppos);
		}
	}
	
	const CONFIGVARIABLE * v = variables.Get(param);
	
	if (!v)
		return false;
	
	outvar = v->val_b;
	
	return true;
}

bool CONFIGFILE::GetParam(string param, float & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			ppos++;
			param = param.substr(ppos, param.length() - ppos);
		}
	}
	
	const CONFIGVARIABLE * v = variables.Get(param);
	
	if (!v)
		return false;
	
	outvar = v->val_f;
	
	return true;
}

bool CONFIGFILE::GetParam(string param, float * outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			ppos++;
			param = param.substr(ppos, param.length() - ppos);
		}
	}
	
	const CONFIGVARIABLE * v = variables.Get(param);
	
	if (!v)
		return false;
	
	for (int i = 0; i < 3; i++)
		outvar[i] = v->val_v[i];
	
	return true;
}

bool CONFIGFILE::GetParam(string param, string & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			ppos++;
			param = param.substr(ppos, param.length() - ppos);
		}
	}
	
	const CONFIGVARIABLE * v = variables.Get(param);
	
	if (!v)
		return false;
	
	outvar = v->val_s;
	
	return true;
}

void CONFIGFILE::Clear()
{
	filename.clear();
	variables.Clear();
}

void CONFIGFILE::Add(string & paramname, CONFIGVARIABLE & newvar)
{
	variables.Set(paramname, newvar);
}

CONFIGVARIABLE::CONFIGVARIABLE()
{
	val_s = "";
	val_i = 0;
	val_f = 0;
	val_b = false;
	int i;
	for (i = 0; i < 3; i++)
		val_v[i] = 0;
	
	next = NULL;
}

const string CONFIGVARIABLE::GetFullName() const
{
	string outstr = "";
	
	if (section != "")
		outstr = outstr + section + ".";
	outstr = outstr + name;
	
	return outstr;
}

void CONFIGVARIABLE::Set(string newval)
{
	newval = strTrim(newval);
	
	val_i = atoi(newval.c_str());
	val_f = atof(newval.c_str());
	val_s = newval;
	
	val_b = false;
	if (val_i == 0)
		val_b = false;
	if (val_i == 1)
		val_b = true;
	if (strLCase(newval) == "true")
		val_b = true;
	if (strLCase(newval) == "false")
		val_b = false;
	if (strLCase(newval) == "on")
		val_b = true;
	if (strLCase(newval) == "off")
		val_b = false;
	
	//now process as vector information
	int pos = 0;
	int arraypos = 0;
	string::size_type nextpos = newval.find(",", pos);
	string frag;
	
	while (nextpos < /*(int)*/ newval.length() && arraypos < 3)
	{
		frag = newval.substr(pos, nextpos - pos);
		val_v[arraypos] = atof(frag.c_str());
		
		pos = nextpos+1;
		arraypos++;
		nextpos = newval.find(",", pos);
	}
	
	//don't forget the very last one
	if (arraypos < 3)
	{
		frag = newval.substr(pos, newval.length() - pos);
		val_v[arraypos] = atof(frag.c_str());
	}
}

void CONFIGVARIABLE::DebugPrint(ostream & out)
{
	if (section != "")
		out << section << ".";
	out << name << endl;
	out << "string: " << val_s << endl;
	out << "int: " << val_i << endl;
	out << "float: " << val_f << endl;
	out << "vector: (" << val_v[0] << "," << val_v[1] << "," << val_v[2] << ")" << endl;
	out << "bool: " << val_b << endl;
	
	out << endl;
}

string CONFIGVARIABLE::strLTrim(string instr)
{
	return instr.erase(0, instr.find_first_not_of(" \t"));
}

string CONFIGVARIABLE::strRTrim(string instr)
{
	if (instr.find_last_not_of(" \t") != string::npos)
		return instr.erase(instr.find_last_not_of(" \t") + 1);
	else
		return instr;
}

string CONFIGVARIABLE::strTrim(string instr)
{
	return strLTrim(strRTrim(instr));
}

bool CONFIGFILE::Load(string fname)
{
	filename = fname;
	
	//work string
	string ws;
	
	ifstream f;
	f.open(fname.c_str());
	
	if (!f && !SUPPRESS_ERROR)
	{
		//cout << "CONFIGFILE.Load: Couldn't find file:  " << fname << endl;
		return false;
	}
	
	return Load(f);
}

bool CONFIGFILE::Load(std::istream & f)
{
	string cursection = "";
	const int MAXIMUMCHAR = 1024;
	char trashchar[MAXIMUMCHAR];
	
	while (f && !f.eof())
	{
		f.getline(trashchar, MAXIMUMCHAR, '\n');
		ProcessLine(cursection, trashchar);
	}
	
	//if (fname.find(".car") < fname.length())
	//	DebugPrint();
	
	return true;
}

string CONFIGFILE::Trim(string instr)
{
	CONFIGVARIABLE trimmer;
	string outstr = trimmer.strTrim(instr);
	return outstr;
}

void CONFIGFILE::ProcessLine(string & cursection, string linestr)
{
	linestr = Trim(linestr);
	linestr = Strip(linestr, '\r');
	linestr = Strip(linestr, '\n');
	
	//remove comments
	string::size_type commentpos = linestr.find("#", 0);
	if (commentpos < /*(int)*/ linestr.length())
	{
		linestr = linestr.substr(0, commentpos);
	}
	
	linestr = Trim(linestr);
	
	//only continue if not a blank line or comment-only line
	if (linestr.length() > 0)
	{
		if (linestr.find("=", 0) < linestr.length())
		{
			//find the name part
			string::size_type equalpos = linestr.find("=", 0);
			string name = linestr.substr(0, equalpos);
			equalpos++;
			string val = linestr.substr(equalpos, linestr.length() - equalpos);
			name = Trim(name);
			val = Trim(val);
			
			//only continue if valid
			if (name.length() > 0 && val.length() > 0)
			{
				CONFIGVARIABLE newvar;
				//newvar = new CONFIGVARIABLE;
				newvar.section = cursection;
				newvar.name = name;
				newvar.Set(val);
				
				string paramname = name;
				if (!cursection.empty())
					paramname = cursection + "." + paramname;
				
				Add(paramname, newvar);
			}
		}
		else
		{
			//section header
			linestr = Strip(linestr, '[');
			linestr = Strip(linestr, ']');
			linestr = Trim(linestr);
			cursection = linestr;
		}
	}
}

string CONFIGFILE::Strip(string instr, char stripchar)
{
	string::size_type pos = 0;
	string outstr = "";
	
	while (pos < /*(int)*/ instr.length())
	{
		if (instr.c_str()[pos] != stripchar)
			outstr = outstr + instr.substr(pos, 1);
		
		pos++;
	}
	
	return outstr;
}

void CONFIGFILE::DebugPrint(ostream & out)
{
	/*CONFIGVARIABLE * cur = vars;
	cout << "*** " << filename << " ***" << endl << endl;
	while (cur != NULL)
	{
		cur->DebugPrint();
		
		cur = cur->next;
	}*/
	
	out << "*** " << filename << " ***" << endl << endl;
	
	std::list <CONFIGVARIABLE> vlist;
	
	for (bucketed_hashmap <std::string, CONFIGVARIABLE>::iterator i = variables.begin(); i != variables.end(); ++i)
	{
		//cout << incsuccess << endl;
		//variables.IteratorGet()->DebugPrint();
		vlist.push_back(*i);
	}
	
	vlist.sort();
	
	for (list <CONFIGVARIABLE>::iterator i = vlist.begin(); i != vlist.end(); ++i)
	{
		i->DebugPrint(out);
	}
}

string CONFIGVARIABLE::strLCase(string instr)
{
	char tc[2];
	tc[1] = '\0';
	string outstr = "";
	
	string::size_type pos = 0;
	while (pos < /*(int)*/ instr.length())
	{
		if (instr.c_str()[pos] <= 90 && instr.c_str()[pos] >= 65)
		{
			tc[0] = instr.c_str()[pos] + 32;
			string tstr = tc;
			outstr = outstr + tc;
		}
		else
			outstr = outstr + instr.substr(pos, 1);
		
		pos++;
	}
	
	return outstr;
}

string CONFIGFILE::LCase(string instr)
{
	CONFIGVARIABLE lcaser;
	string outstr = lcaser.strLCase(instr);
	return outstr;
}

bool CONFIGFILE::SetParam(string param, int invar)
{
	char tc[256];
	
	sprintf(tc, "%i", invar);
	
	string tstr = tc;
	
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, bool invar)
{
	//char tc[256];
	
	//sprintf(tc, "%i", invar);
	
	string tstr = "false";
	
	if (invar)
		tstr = "true";
	
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, float invar)
{
	char tc[256];
	
	sprintf(tc, "%f", invar);
	
	string tstr = tc;
	
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, float * invar)
{
	char tc[256];
	
	sprintf(tc, "%f,%f,%f", invar[0], invar[1], invar[2]);
	
	string tstr = tc;
	
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, string invar)
{
	CONFIGVARIABLE newvar;
	
	newvar.name = param;
	newvar.section = "";
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		newvar.section = param.substr(0, ppos);
		ppos++;
		newvar.name = param.substr(ppos, param.length() - ppos);
	}
	
	newvar.Set(invar);
	
	Add(param, newvar);
	
	return true;
}

bool CONFIGFILE::Write(bool with_brackets)
{
	return Write(with_brackets, filename);
}

bool CONFIGFILE::Write(bool with_brackets, string save_as)
{
	ofstream f;
	f.open(save_as.c_str());
	
	if (f)
	{
		//clear out written flags
		/*CONFIGVARIABLE * cur = vars;
		while (cur != NULL)
		{
			cur->written = false;
			cur = cur->next;
		}
		
		//write non-section variables first
		bool nosection = false;
		cur = vars;
		while (cur != NULL)
		{
			if (cur->section == "")
			{
				f << cur->name << " = " << cur->val_s << endl;
				
				nosection = true;
				
				cur->written = true;
			}
			
			cur = cur->next;
		}
		
		if (nosection)
			f << endl;
		
		//write variables by section
		cur = vars;
		while (cur != NULL)
		{
			if (!cur->written)
			{
				if (with_brackets)
					f << "[ " << cur->section << " ]" << endl;
				else
					f << cur->section << endl;
				
				CONFIGVARIABLE * sub = vars;
				while (sub != NULL)
				{
					if (!sub->written && cur->section == sub->section)
					{
						f << sub->name << " = " << sub->val_s << endl;
						
						sub->written = true;
					}
					
					sub = sub->next;
				}
				
				f << endl;
				
				cur->written = true;
			}
			
			cur = cur->next;
		}
		*/
		
		list <CONFIGVARIABLE> vlist;
	
		for (bucketed_hashmap <std::string, CONFIGVARIABLE>::iterator i = variables.begin(); i != variables.end(); ++i)
		{
			//cout << incsuccess << endl;
			//variables.IteratorGet()->DebugPrint();
			vlist.push_back(*i);
		}
		
		vlist.sort();
		
		string cursection = "";
		for (list <CONFIGVARIABLE>::iterator cur = vlist.begin(); cur != vlist.end(); ++cur)
		{
			if (cur->section == "")
			{
				f << cur->name << " = " << cur->val_s << endl;
			}
			else
			{
				if (cur->section != cursection)
				{
					f << endl;
					cursection = cur->section;
					
					if (with_brackets)
						f << "[ " << cur->section << " ]" << endl;
					else
						f << cur->section << endl;
				}
				
				f << cur->name << " = " << cur->val_s << endl;
			}
		}
		
		f.close();
		return true;
	}
	else
		return false;
}

bool CONFIGFILE::Write()
{
	return Write(true);
}

bool CONFIGFILE::ClearParam(string param)
{
	return variables.Erase(param);
}

void CONFIGFILE::GetSectionList(list <string> & sectionlistoutput) const
{
	sectionlistoutput.clear();
	map <string, bool> templist;
	for (bucketed_hashmap <std::string, CONFIGVARIABLE>::const_iterator i = variables.begin(); i != variables.end(); ++i)
	{
		templist[i->section] = true;
	}
	
	for (map <string, bool>::iterator i = templist.begin(); i != templist.end(); ++i)
	{
		sectionlistoutput.push_back(i->first);
	}
}

void CONFIGFILE::GetParamList(list <string> & paramlistoutput, string sel_section) const
{
	bool all = false;
	if (sel_section == "")
		all = true;
	
	//cout << "++++++++++" << variables.GetTotalObjects() << endl;
	
	paramlistoutput.clear();
	map <string, bool> templist;
	for (bucketed_hashmap <std::string, CONFIGVARIABLE>::const_iterator i = variables.begin(); i != variables.end(); ++i)
	{
		if (all)
			templist[i->section+"."+i->name] = true;
		else if (i->section == sel_section)
		{
			templist[i->name] = true;
		}
		
		/*if (sel_section == "engine")
		{
			cout << "++++++" << variables.IteratorGet()->section << endl;
		}*/
	}
	
	for (map <string, bool>::iterator i = templist.begin(); i != templist.end(); ++i)
	{
		paramlistoutput.push_back(i->first);
	}
}

CONFIGVARIABLE & CONFIGVARIABLE::CopyFrom(const CONFIGVARIABLE & other)
{
	/*
	string section;
	string name;

	string val_s;
	int val_i;
	float val_f;
	float val_v[3];
	bool val_b;
	*/
	
	section = other.section;
	name = other.name;
	val_s = other.val_s;
	val_i = other.val_i;
	val_f = other.val_f;
	val_b = other.val_b;
	
	for (int i = 0; i < 3; i++)
		val_v[i] = other.val_v[i];
	
	return *this;
}

bool CONFIGVARIABLE::operator<(const CONFIGVARIABLE & other)
{
	//return GetFullName() < other.GetFullName();
	return (section + "." + name < other.section + "." + other.name);
}

void CONFIGFILE::ChangeSectionName(std::string oldname, std::string newname)
{
	std::list <std::string> paramlist;
	GetParamList(paramlist, oldname);
	
	for (std::list <std::string>::iterator i = paramlist.begin(); i != paramlist.end(); ++i)
	{
		std::string value;
		bool success = GetParam(oldname+"."+*i, value);
		assert(success);
		SetParam(newname+"."+*i, value);
		success = ClearParam(oldname+"."+*i);
		assert(success);
	}
}

