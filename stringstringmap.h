#ifndef _stringstringmap_h_
#define _stringstringmap_h_

#include <map>
#include <string>

class StringStringMap : public std::map<std::string, std::string>
{
	public:

		std::string dump(bool html) const throw();
		std::string operator()(const std::string & key) const throw();
};

#endif
