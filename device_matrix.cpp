/* Copyright Erik Slagter, GPLv2 is applicable */

#include "device_matrix.h"

DeviceMatrix::DeviceMatrix(string node) throw()
{
	if(node == "")
		_device_node = "/dev/ttyS0";
	else
		_device_node = node;
}

DeviceMatrix::~DeviceMatrix() throw()
{
	__close();
}

void DeviceMatrix::__init() throw(string)
{
	_command('D');	// auto line wrapping off
	_command('R');	// auto scroll off
	_command('T');	// cursor blink off
	_command('K');	// cursor off
}

void DeviceMatrix::__reinit() throw(string)
{
	__init();
}

void DeviceMatrix::__setbright(int value) throw(string)
{
	static const int value2brightness[] =
	{
		3,	//  off
		3,	//  very low
		2,	//  low
		1,	//  normal
		0	//  bright
    };

	if(value == 0)
		_command('F');		// display off
	else
		_command('B', 0x05); // display on for five minutes

	_command('Y', value2brightness[value]);
}

string DeviceMatrix::identify() const throw()
{
	return(string("Matrix Orbital VFD or LCD RS232C display"));
}
