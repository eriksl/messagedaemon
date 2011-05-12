#include <time.h>
#include <stdlib.h>

#include "textentry.h"
#include "syslog.h"

TextEntry::TextEntry() throw(string)
{
}

TextEntry::TextEntry(const string & id_in, const string & text_in, int expiry) throw(string)
{
	id		= id_in;
	text	= text_in;
	valid	= true;

	if(expiry > 0)
		expire = time(0) + expiry;
	else
		expire = expiry;
}

TextEntry & TextEntry::operator =(const TextEntry & in) throw(string)
{
	id				= in.id;
	attributes		= in.attributes;
	expire			= in.expire;
	text			= in.text;
	valid			= in.valid;

	return(*this);
}

TextEntries::TextEntries() throw(string)
	: freeze_id(""), freeze_timeout(0)
{
	pthread_rwlock_init(&rwlock, 0);
}

TextEntries::~TextEntries() throw(string)
{
	pthread_rwlock_destroy(&rwlock);
}

TextEntry TextEntries::get(const string & key) throw(string)
{
	TextEntriesMap::iterator	it;
	TextEntry					text_entry;

	pthread_rwlock_rdlock(&rwlock);

	if((it = data.find(key)) == data.end())
	{
		pthread_rwlock_unlock(&rwlock);
		throw(string("TextEntries::get: key not found"));
	}

	text_entry = it->second;

	pthread_rwlock_unlock(&rwlock);

	return(it->second);
}

TextEntry TextEntries::get_next(const string & key, bool & last) throw(string)
{
	TextEntriesMap::iterator	it;
	TextEntry					text_entry;

	pthread_rwlock_rdlock(&rwlock);

	if((it = data.find(key)) == data.end())
		it = data.begin();
	else
	{
		if(++it == data.end())
			it = data.begin();
	}

	if(it == data.end())
	{
		pthread_rwlock_unlock(&rwlock);
		throw(string("TextEntries::get_next: set empty"));
	}

	text_entry = it->second;

	if(++it == data.end())
		last = true;
	else
		last = false;

	pthread_rwlock_unlock(&rwlock);

	if(!text_entry.valid)
		throw(string("TextEntries::getnext: invalid TextEntry"));

	return(text_entry);
}

int TextEntries::size()
{
	int rv;

	pthread_rwlock_rdlock(&rwlock);
	rv = data.size();
	pthread_rwlock_unlock(&rwlock);

	return(rv);
}

void TextEntries::put(const TextEntry & text_entry) throw(string)
{
	pthread_rwlock_wrlock(&rwlock);
	data[text_entry.id] = text_entry;
	pthread_rwlock_unlock(&rwlock);
}

void TextEntries::erase(const string & key) throw(string)
{
	pthread_rwlock_wrlock(&rwlock);

	if(data.erase(key) == 0)
	{
		pthread_rwlock_unlock(&rwlock);
		throw(string("key not in map"));
	}

	pthread_rwlock_unlock(&rwlock);
}

void TextEntries::freeze(const string & id) throw(string)
{
	pthread_rwlock_rdlock(&rwlock);

	if(data.find(id) == data.end())
	{
		pthread_rwlock_unlock(&rwlock);
		throw(string("TextEntries::freeze: id not found"));
	}

	freeze_id = id;
	freeze_timeout = time(0) + 20;

	pthread_rwlock_unlock(&rwlock);
}

time_t TextEntries::get_freeze_timeout()
{
	time_t timeout;

	pthread_rwlock_rdlock(&rwlock);
	timeout	= freeze_timeout;
	pthread_rwlock_unlock(&rwlock);

	return(timeout);
}

string TextEntries::get_freeze_id()
{
	string id;

	pthread_rwlock_rdlock(&rwlock);
	id = freeze_id;
	pthread_rwlock_unlock(&rwlock);

	return(id);
}

bool TextEntries::frozen()
{
	bool rv;

	pthread_rwlock_rdlock(&rwlock);
	rv = freeze_timeout > time(0);
	pthread_rwlock_unlock(&rwlock);

	return(rv);
}
