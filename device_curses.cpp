/* Copyright Erik Slagter, GPLv2 is applicable */

#include <ncurses.h>
#undef standout

#include "device.h"
#include "device_curses.h"
#include "syslog.h"

DeviceCurses::DeviceCurses() throw(string)
{
	_root = _border = _content = 0;
	_screen = 0;
}

DeviceCurses::~DeviceCurses()
{
	__close();
}

void DeviceCurses::__open() throw(string)
{
	if(_screen || _root || _border || _content)
		throw(string("DeviceCurses::__open:: device not closed"));

	_screen = newterm(0, stdout, stdin);
	set_term(_screen);

	_root = ::newwin(4, 20, 0, 0);
	start_color();
	::scrollok(_root, 0);
	_border = ::newwin(6, 22, 0, 0);
	::scrollok(_border, 0);
	::wborder(_border, 0, 0, 0, 0, 0, 0, 0, 0);
	::wrefresh(_border);
	_content = ::newwin(4, 20, 1, 1);
	::wrefresh(_content);
	setcolour();
}

void DeviceCurses::__close()
{
	if(_content)
		::delwin(_content);

	if(_border)
		::delwin(_border);

	if(_root)
		::delwin(_root);

	if(_root || _border || _content)
		::endwin();

	_root = _border = _content = 0;

	if(_screen)
		::delscreen(_screen);

	_screen = 0;
}

int DeviceCurses::width() const
{
	int xx, yy;

	getmaxyx(_content, yy, xx);
	return(xx);
}

int DeviceCurses::height() const
{
	int xx, yy;

	getmaxyx(_content, yy, xx);
	return(yy);
}

int DeviceCurses::brightness_to_colour(unsigned int b)
{
	int b2c[] = {
		COLOR_BLACK,
		COLOR_GREEN,
		COLOR_CYAN,
		COLOR_YELLOW,
		COLOR_WHITE };

	if(b >= (sizeof(b2c) / sizeof(*b2c)))
		return(COLOR_WHITE);

	return(b2c[b]);
}

void DeviceCurses::setcolour() throw(string)
{
	int	fg_colour;
	int	bg_colour;

	if(!_root || !_border || !_content)
		throw(string("DeviceCurses::__setcolour:: device not open"));

	fg_colour = COLOR_BLACK;
	bg_colour = brightness_to_colour(_brightness);

	if(::init_pair(1, fg_colour, bg_colour) == ERR)
		throw(string("init_pair"));

	if(::wcolor_set(_border, 1, 0) == ERR)
		throw(string("wcolor_set(_border)"));

	if(::wcolor_set(_content, 1, 0) == ERR)
		throw(string("wcolor_set(_content)"));

	if(::redrawwin(_content) == ERR)
		throw(string("redrawwin"));
}

void DeviceCurses::brightness(int percentage) throw(string)
{
	if(!_root || !_border || !_content)
		throw(string("DeviceCurses::brightness:: device not open"));

	Device::brightness(percentage);
	setcolour();
}

void DeviceCurses::standout(bool onoff) throw(string)
{
	if(!_root || !_border || !_content)
		throw(string("DeviceCurses::standout:: device not open"));

	Device::standout(onoff);

	if(_standout)
		::wattron(_content, A_REVERSE);
	else
		::wattroff(_content, A_REVERSE);
}

void DeviceCurses::__update() throw(string)
{
	char *	current;
	int		yy;

	if(!_root || !_border || !_content)
		throw(string("DeviceCurses::__update:: device not open"));

	for(yy = 0; yy < height(); yy++)
	{
		current = _textbuffer + (width() * yy);
		mvwaddnstr(_content, yy, 0, current, width());
	}

	::wrefresh(_content);
	::wrefresh(_border);
}
