/* Copyright Erik Slagter, GPLv2 is applicable */

#include "device.h"
#include "device_esp.h"
#include "syslog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>

extern "C"
{
#include <libespif.h>
}

#include <boost/algorithm/string.hpp>

typedef vector<string> stringvector;

DeviceESP::DeviceESP(string devicenode_in) throw(string)
	: devicenode(devicenode_in)
{
}

DeviceESP::~DeviceESP()
{
	__close();
}

void DeviceESP::__open() throw(string)
{
}

void DeviceESP::__close()
{
	try
	{
		clear();
		__update();
	}
	catch(...)
	{
	}
}

int DeviceESP::width() const
{
	return(20);
}

int DeviceESP::height() const
{
	return(4);
}

void DeviceESP::__update() throw(string)
{
	stringvector tokens;
	stringvector::const_iterator it;
	string textrequest;

	char buffer[1024];

	espif_setup setup =
	{
		.verbose = 0,
		.connto = 2000,
		.conntr = 4,
		.recvto1 = 2000,
		.recvto2 = 100,
		.sendtr = 4,
	};

	textrequest = string("dhshw ");
	textrequest.append(_textbuffer, _textbuffer_size);
	textrequest.append("\r\n");

	split(tokens, devicenode, boost::is_any_of(","));

	for(it = tokens.begin(); it != tokens.end(); it++)
	{
		if(espif(&setup, it->c_str(), textrequest.c_str(), sizeof(buffer), buffer))
            vlog("ERROR: %s\n", buffer);
        else
            vlog("OK: %s\n", buffer);
	}
}

string DeviceESP::identify() const throw()
{
	return(string("IP device 4x20"));
}
