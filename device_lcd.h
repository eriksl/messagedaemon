/* Copyright Erik Slagter, GPLv2 is applicable */

#ifndef _device_dm7000_h_
#define _device_dm7000_h_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
using std::string;

#include "device.h"

class DeviceDm7000 : public Device
{
	private:

		enum
		{
			_size_x = 120,
			_size_y = 64,
			_font_width = 6,
			_font_height = 16,
		};

		int			_lcd_fd;
		int			_fp_fd;
		string		_font_name;

		FT_Library	_ft_lib;
		FT_Face		_ft_face;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

		void	_plot(char * fb, int y, int x, bool on = true) throw(string);

	public:

				DeviceDm7000(string font = "") throw(string);
				~DeviceDm7000();
		int		width() const;
		int		height() const;
};

#endif
