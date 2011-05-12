#include <fcntl.h>
#include <sys/ioctl.h>

#include "device.h"
#include "device_lcd.h"

#include "fp.h"
#include "lcd-ks0713.h"

#include "syslog.h"

DeviceLcd::DeviceLcd(string font) throw(string) :
{
	_lcd_fd	= -1;
	_fp_fd	= -1
}

DeviceLcd::~DeviceLcd()
{
	__close();
}

void DeviceLcd::__open() throw(string)
{
	int v;
	int error;

	if((_lcd_fd != -1) || (_fp_fd != -1))
		throw(string("DeviceLcd::__open: device not closed"));

	if(font == "")
		font = "/share/fonts/DejaVuSansMono-Bold.ttf";

	if((_fp_fd = ::open("/dev/dbox/fp0", O_RDWR, 0)) < 0)
		throw(string("DeviceLcd::DeviceLcd: cannot open front processor device"));

	if((_lcd_fd = ::open("/dev/dbox/lcd0", O_RDWR, 0)) < 0)
		throw(string("DeviceLcd::DeviceLcd: cannot open lcd device"));

	if(ioctl(_lcd_fd, LCD_IOCTL_INIT))
		throw(string("DeviceLcd::DeviceLcd: LCD_IOCTL_INIT"));

	v = 40;

	if(ioctl(_lcd_fd, LCD_IOCTL_SRV, &v))
		throw(string("DeviceLcd::DeviceLcd:: cannot set contrast"));

	if(!!(error = FT_Init_FreeType(&_ft_lib)))
		throw(string("DeviceLcd::DeviceLcd: FT_Init_FreeType"));

	if(!!(error = FT_New_Face(_ft_lib, font.c_str(), 0, &_ft_face)))
		throw(string("DeviceLcd::DeviceLcd: FT_New_Face"));

	if(!!(error = FT_Set_Pixel_Sizes(_ft_face, 8, 18)))
		throw(string("DeviceLcd::DeviceLcd:: FT_Set_Pixel_Sizes"));
}

void DeviceLcd::__close()
{
	if((_lcd_fd != -1) && (_fp_fd != -1))
	{
		try
		{
			clear();
		}
		catch(...)
		{
		}
	}

	if(_lcd_fd != -1)
	{
		::close(_lcd_fd);
		_lcd_fd = -1;
	}

	if(_fp_fd != -1)
	{
		::close(_fp_fd);
		_fp_fd  = -1;
	}
}

int DeviceLcd::width() const
{
	return(20);
}

int DeviceLcd::height() const
{
	return(4);
}

void DeviceLcd::_plot(char * fb, int y, int x, bool on) throw(string)
{
	size_t byte, bit;

	if((_lcd_fd == -1) && (_fp_fd == -1))
		throw(string("DeviceLcd::_plot: device not open"));

	if(y > _size_y)
		return;

	if(x > _size_x)
		return;

	byte = (y >> 3) * _size_x + x;
	bit  = (y  & 7);

	if(bit > 7)
		throw(string("DeviceLcd::_plot: bit > 7"));

	if(on)
		fb[byte] |= (1 << bit);
	else
		fb[byte] &= ~(1 << bit);
}

void DeviceLcd::__update() throw(string)
{
	char * 			framebuffer;
	ssize_t			framebuffer_size;
	int				char_x, char_y;
	int				pointx, pointy;
	unsigned char *	fontptr;
	FT_GlyphSlot	slot = _ft_face->glyph;
	int				error;
	int				y, x, bytepos, byte, bit, bitpos, offsetx, offsety;
	int				v;
	char			ch;

	if((_lcd_fd == -1) && (_fp_fd == -1))
		throw(string("DeviceLcd::__update: device not open"));

	v = _standout ? 255 : 92;

	if(ioctl(_fp_fd, FP_IOCTL_LCD_DIMM, &v))
		throw(string("DeviceLcd::__update: cannot set brightness"));

	framebuffer_size = _size_x * _size_y / 8;

	if(!(framebuffer = (char *)malloc(framebuffer_size)))
		throw(string("DeviceLcd::__update::malloc"));

	memset(framebuffer, 0, framebuffer_size);

	for(char_y = 0; char_y < height(); char_y++)
	{
		for(char_x = 0; char_x < width(); char_x++)
		{
			pointx	= (char_x * _font_width);
			pointy	= (char_y * _font_height) - 1;
			ch		= _textbuffer[char_x + (char_y * width())];

			if(!!(error = FT_Load_Char(_ft_face, ch, FT_LOAD_RENDER | FT_LOAD_MONOCHROME)))
				throw(string("DeviceLcd::__update::FT_Load_Char"));

			if(slot->bitmap.pixel_mode != ft_pixel_mode_mono)
				throw(string("DeviceLcd::__update: bit mode wrong"));

			fontptr = static_cast<typeof(fontptr)>(slot->bitmap.buffer);

			for(y = 0; y < slot->bitmap.rows; y++)
			{
				for(x = 0; x < slot->bitmap.pitch; x++)
				{
					bytepos = (y * slot->bitmap.pitch) + x;
					byte	= fontptr[bytepos];

					for(bitpos = 0; bitpos < 8; bitpos++)
					{
						bit	= byte & (1 << bitpos);

						offsety = _font_height - slot->bitmap_top;
						offsetx = 1 - slot->bitmap_left;

						if(offsety < 0)
							offsety = 0;

						if(offsetx < 0)
							offsetx = 0;

						if(bit)
							_plot(framebuffer, pointy + y + offsety,
									pointx + x + offsetx + (7 - bitpos), true);
					}
				}
			}
		}
	}

	if(::write(_lcd_fd, framebuffer, framebuffer_size) != framebuffer_size)
		throw(string("DeviceLcd::__update::write"));

	free(framebuffer);
}
