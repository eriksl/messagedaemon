#ifndef _device_lcd_h_
#define _device_lcd_h_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
using std::string;

#include "device.h"

class DeviceLcd : public Device
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

		FT_Library	_ft_lib;
		FT_Face		_ft_face;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

		void	_plot(char * fb, int y, int x, bool on = true) throw(string);

	public:

				DeviceLcd() throw(string);
				~DeviceLcd();
		int		width() const;
		int		height() const;
};

#endif
