/* Copyright Erik Slagter, GPLv2 is applicable */

#ifndef _device_curses_h_
#define _device_curses_h_

#include <string>
using std::string;

#include "device.h"

#ifndef __NCURSES_H
typedef void SCREEN;
typedef void WINDOW;
#endif

class DeviceCurses : public Device
{
	private:

		SCREEN * _screen;
		WINDOW * _root;
		WINDOW * _border;
		WINDOW * _content;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

		static int brightness_to_colour(unsigned int);
		void setcolour() throw(string);

	public:

				DeviceCurses() throw(string);
				~DeviceCurses();
		int		width() const;
		int		height() const;

		void brightness(int value) throw(string);
		void standout(bool inoff) throw(string);
};

#endif
