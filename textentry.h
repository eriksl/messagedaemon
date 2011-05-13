/* Copyright Erik Slagter, GPLv2 is applicable */

#ifndef __textentry_h__
#define __textentry_h__

#include <string>
using std::string;

#include <set>
using std::set;

#include <map>
using std::map;
using std::pair;

#include <sys/types.h>
#include <pthread.h>

typedef set<string> TextEntryAttributes;

class TextEntry
{
	public:

		bool				valid;
		string				id;
		TextEntryAttributes	attributes;
		time_t				expire;
		string				text;

		TextEntry() throw(string);
		TextEntry(const string & id, const string & text, int expire = 60) throw(string);
		TextEntry & operator =(const TextEntry &) throw(string);
};

class TextEntries
{
	private:

		typedef map<string,TextEntry>	TextEntriesMap;

		pthread_rwlock_t				rwlock;
		string							freeze_id;
		time_t							freeze_timeout;
		TextEntriesMap					data;

	public:

		TextEntry	get(const string &) throw(string);
		TextEntry	get_next(const string &, bool & last) throw(string);
		void		put(const TextEntry &) throw(string);
		void		erase(const string &) throw(string);
		void		freeze(const string &) throw(string);
		time_t		get_freeze_timeout();
		string		get_freeze_id();
		bool		frozen();
		int			size();
		
		TextEntries() throw(string);
		~TextEntries() throw(string);

};

#endif
