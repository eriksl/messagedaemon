/* Copyright Erik Slagter, GPLv2 is applicable */

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "msgd.h"
#include "device.h"
#include "syslog.h"

Device::Device() throw(string)
{
	_mutex_valid		= false;
	_opened				= false;
	_standout			= false;
	_x					= 0;
	_y					= 0;
	_brightness			= 3;
	_progress			= -1;
	_clean				= true;
	_textbuffer			= 0;
	_textbuffer_size	= 0;

	pthread_mutex_init(&_mutex, 0);
	_mutex_valid = true;
}

Device::~Device()
{
	if(_mutex_valid)
	{
		pthread_mutex_lock(&_mutex);

		if(_textbuffer)
		{
			free(_textbuffer);
			_textbuffer			= 0;
			_textbuffer_size	= 0;
		}

		pthread_mutex_unlock(&_mutex);

		pthread_mutex_destroy(&_mutex);
		_mutex_valid = false;
	}
}

void Device::open() throw(string)
{
	if(_opened)
		throw(string("Device::open:: device not closed"));

	__open();

	_textbuffer_size = width() * height();

	if(!(_textbuffer = (char *)malloc(_textbuffer_size)))
		throw(string("Device::open::malloc"));

	_opened	= true;
	clear();
}

void Device::close()
{
	__close();
	_opened = false;
}

string Device::identify() const throw()
{
	return(string("<unknown device>"));
}

int Device::x() const
{
	return(_x);
}

int Device::y() const
{
	return(_y);
}

void Device::_printline(string text) throw(string)
{
	size_t	ix;
	char	ch;
	size_t	length;

	if(!_opened)
		throw(string("Device::_printline: device not open"));

	if(_y >= height())
		return;

	length = width() - _x;

	if(length > text.length())
		length = string::npos;

	text = text.substr(0, length);

	for(ix = 0; ix < text.length(); ix++)
	{
		ch = text[ix];

		switch(ch)
		{
			case('\r'):
			{
				_move(-1, 0);
				break;
			}

			case('\n'):
			{
				_move(++_y, 0);
				break;
			}

			case('\f'):
			{
				_clear();
				break;
			}

			default:
			{
				size_t pos = (_y * width()) + _x;

				if(pos > _textbuffer_size)
					throw(string("Device::printline: position out of bounds\n"));

				_textbuffer[pos] = ch;
			
				if(++_x >= width())
				{
					_x = 0;
			
					if(++_y >= height())
					{
						_x = width() - 1;
						_y = height() - 1;
					}
			
					_move(-1, -1);
				}
            }
        }
    }
}

void Device::print(string text) throw(string)
{
	size_t	where;
	size_t	start = 0;
	string	line;

	if(!_opened)
		throw(string("Device::printline: device not open"));

	for(;;)
	{
		where = text.find_first_of('\n', start);

		if(where == string::npos)
		{
			if(start != text.length())
				_printline(text.substr(start));
			break;
		}

		_printline(text.substr(start, where - start + 1));
		start = where + 1;
	}

	_clean = false;
}

void Device::_move(int yy, int xx)
{
	if(!_opened)
		throw(string("Device::_move: device not open"));

	if(yy == -1)
		yy = _y;

	if(xx == -1)
		xx = _x;

	if(xx >= width())
		xx = width() - 1;

	if(yy >= height())
		yy = height() - 1;

	_x = xx;
	_y = yy;
}

void Device::move(int yy, int xx)
{
	_move(yy, xx);
}

void Device::_clear()
{
	if(!_opened)
		throw(string("Device::_clear: device not open"));

	memset(_textbuffer, ' ', _textbuffer_size);
	_x = _y = 0;
	_clean = false;
}

void Device::clear()
{
	_clear();
}

void Device::standout(bool onoff)
{
	if(!_opened)
		throw(string("Device::_standout: device not open"));

	_standout = onoff;
}

void Device::brightness(int value) throw(string)
{
	if((value < 0) || (value > 4))
		throw(string("Device::brightness: value out of range"));

	if(!_opened)
		throw(string("Device::brightness: device not open"));

	_brightness = value;
	__update();
}

void Device::progress(int percentage) throw(string)
{
	if(!_opened)
		throw(string("Device::progress: device not open"));

	if((percentage < -1) || (percentage > 100))
		throw(string("Device::progress: value out of range"));

	_progress = percentage;
}

void Device::update() throw(string)
{
	if(!_opened)
		throw(string("Device::update: device not open"));

	if(!_clean)
	{
		__update();
		_clean = true;
	}
}

void Device::poll() throw(string)
{
	if(!_opened)
		throw(string("Device::poll: device not open"));
}

void Device::lock() throw(string)
{
	if(_mutex_valid)
		pthread_mutex_lock(&_mutex);
	else
		throw(string("Device::lock:: mutex invalid"));
}

void Device::unlock() throw(string)
{
	if(_mutex_valid)
		pthread_mutex_unlock(&_mutex);
	else
		throw(string("Device::unlock:: mutex invalid"));
}

void Device::_addkey(int key, bool pressed)
{
	keyevent_t newkey;

	if(!_opened)
		throw(string("Device::_addkey: device not open"));

	newkey.key		= key;
	newkey.status	= pressed ? keyevent_t::key_pressed : keyevent_t::key_released;

	keyevents.push_front(newkey);
}

Device::keyevent_t Device::getkey(bool & ok)
{
	keyevent_t key;

	if(!_opened)
		throw(string("Device::getkey: device not open"));

	if(keyevents.size() > 0)
	{
		key	= keyevents.back();
		keyevents.pop_back();
		ok	= true;
	}
	else
	{
		key.key		= -1;
		key.status	= keyevent_t::key_undef;
		ok			= false;
	}

	return(key);
}

bool Device::canbeep() const throw()
{
	return(false);
}

void Device::__beep(int) throw(string)
{
}

void Device::beep(int pitch) throw(string)
{
	if(!_opened)
		throw(string("Device::beep: device not open"));

	if(pitch < 0 || pitch > 4)
		throw(string("Device::beep: pitch out of range"));

	__beep(pitch);
}

int Device::__read_analog(int) throw(string)
{
	throw(string("Device::__read_analog: function not implemented"));
}

int Device::analog_inputs() const throw()
{
	return(0);
}

int Device::read_analog(int input) throw(string)
{
	if(!_opened)
		throw(string("Device::read_analog: device not open"));

	if((input < 0) || (input >= analog_inputs()))
		throw(string("Device::read_analog: input out of range"));

	return(__read_analog(input));
}

int Device::max_analog() const throw(string)
{
	return(0);
}
