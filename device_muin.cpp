#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "syslog.h"
#include "device_muin.h"

DeviceMuin::DeviceMuin(string node) throw()
{
	if(node == "")
		_device_node = "/dev/ttyACM0";
	else
		_device_node = node;
}

DeviceMuin::~DeviceMuin() throw()
{
	__close();
}

void DeviceMuin::__init() throw(string)
{
	_command(0x58);	// clear screen
	_command(0x42);	// display on
	_command(0x4b);	// underline cursor off
	_command(0x54);	// cursor doesn't blink
}

void DeviceMuin::__setbright(int value) throw(string)
{
vlog((string("* setbright ") + lexical_cast<string>(value) + "\n").c_str());
	static const int value2brightness[] =
	{
		0,	//  off
		25,	//  very low
		50,	//  low
		75,	//  normal
		100	//  bright
    };

	if(value == 0)
	{
		_command(0xfc);
	}
	else
	{
		_command(0xfb);
		_command(0xfd, value2brightness[value]);
	}
}

string DeviceMuin::identify() const throw()
{
	return(string("Droids MuIn USB LCD"));
}

int DeviceMuin::__read_analog(int input) throw(string)
{
	uint8_t		reply[32];
	uint32_t	rv;

	_command(0xfa);
	_command(0xf1);
	_read_reply(10, reply);

	rv =  ((uint32_t)reply[input * 2 + 0]) << 8;
	rv |= ((uint32_t)reply[input * 2 + 1]) << 0;

	return(rv);
}

void DeviceMuin::__beep(int pitch) throw(string)
{
	_command(0x73, pitch * 48, 1000);
	usleep(1000);
}

bool DeviceMuin::canbeep() const throw()
{
	return(true);
}

int DeviceMuin::analog_inputs() const throw()
{
	return(5);
}

int DeviceMuin::max_analog() const throw()
{
	return(1024);
}
