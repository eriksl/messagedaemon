#include "stringstringmap.h"

#include <string>
using std::string;

string StringStringMap::dump(bool html) const throw()
{
	string rv;
	StringStringMap::const_iterator it;

	if(html)
		rv = "<table border=\"1\" cellspacing=\"0\" cellpadding=\"0\">\n";

	for(it = begin(); it != end(); it++)
	{
		if(html)
			rv += "<tr><td>\n";

		rv += it->first;

		if(html)
			rv += "</td><td>\n";
		else
			rv += " = ";

		rv += it->second;

		if(html)
			rv += "</td></tr>\n";
		else
			rv += "\n";
	}

	if(html)
		rv += "</table>\n";

	return(rv);
}

string StringStringMap::operator() (const string & key) const throw()
{
	StringStringMap::const_iterator it;

	if((it = find(key)) == end())
		return(string(""));

	return(it->second);
}
