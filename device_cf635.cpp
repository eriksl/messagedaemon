#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>

#include "device.h"
#include "device_cf635.h"
#include "syslog.h"

const static int debug = -1;

DeviceCF635::DeviceCF635(string node) throw(string)
{
	_fd			= -1;
	_flashcycle	= 0;

	if(node == "")
		_device_node = "/dev/ttyUSB0";
	else
		_device_node = node;
}

DeviceCF635::~DeviceCF635()
{
	__close();
}

void DeviceCF635::__open() throw(string)
{
	if(_fd != -1)
		throw(string("DeviceCf635::__open: device not closed"));

	if((_fd = ::open(_device_node.c_str(), O_RDWR | O_NOCTTY, 0)) < 0)
		throw(string("DeviceCF635::DeviceCF635::__open"));

	_initserial();
	_ping();
	_initgpio();
	_initkeyreporting();
}

void DeviceCF635::__close()
{
	if(_fd == -1)
		return;

	try
	{
		_setbright(0);
		_setled(0, 0, 0);
		_setled(1, 0, 0);
		_setled(2, 0, 0);
		_setled(3, 0, 0);
	}
	catch(...)
	{
	}

	::close(_fd);
	_fd = -1;
}

void DeviceCF635::_initserial() throw(string)
{
	int				result;
	struct termios	tio;

	if(!_fd == -1)
		throw(string("DeviceCF635::_initserial: device not open"));

	if(ioctl(_fd, TIOCMGET, &result))
		throw(string("DeviceCF635::DeviceCF635::ioctl(TIOCMGET)"));

	result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(_fd, TIOCMSET, &result))
		throw(string("DeviceCF635::DeviceCF635::ioctl(TIOCMSET)"));

	result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(_fd, TIOCMSET, &result))
		throw(string("DeviceCF635::DeviceCF635::ioctl(TIOCMSET)"));

	if(tcgetattr(_fd, &tio) == 1)
		throw(string("DeviceCF635::DeviceCF635::tcgetattr"));

	tio.c_iflag &=	~(BRKINT|PARMRK|INPCK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF|IUCLC|IUTF8);
	tio.c_iflag |=	IGNPAR | IGNBRK;

	tio.c_oflag &=	~(OPOST|ONLCR|OCRNL|ONOCR|ONLRET|OFILL|OFDEL|NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY|OLCUC);
	tio.c_oflag |=	0;

	tio.c_cflag &=	~(CSIZE|PARENB|PARODD|CRTSCTS|CSTOPB);
	tio.c_cflag |=	CREAD|CS8|CLOCAL|HUPCL;

	tio.c_lflag &=	~(ISIG|ICANON|IEXTEN|ECHO|TOSTOP);
	tio.c_lflag |=	NOFLSH;

	cfsetispeed(&tio, B115200);
	cfsetospeed(&tio, B115200);

	if(tcsetattr(_fd, TCSANOW, &tio) == 1)
		throw(string("DeviceCF635::DeviceCF635::tcsetattr"));
}

unsigned int DeviceCF635::_crc32(size_t data_length, const unsigned char * data)
{
	static const unsigned int crc_values[256] =
	{
		0x00000,0x01189,0x02312,0x0329b,0x04624,0x057ad,0x06536,0x074bf,
		0x08c48,0x09dc1,0x0af5a,0x0bed3,0x0ca6c,0x0dbe5,0x0e97e,0x0f8f7,
		0x01081,0x00108,0x03393,0x0221a,0x056a5,0x0472c,0x075b7,0x0643e,
		0x09cc9,0x08d40,0x0bfdb,0x0ae52,0x0daed,0x0cb64,0x0f9ff,0x0e876,
		0x02102,0x0308b,0x00210,0x01399,0x06726,0x076af,0x04434,0x055bd,
		0x0ad4a,0x0bcc3,0x08e58,0x09fd1,0x0eb6e,0x0fae7,0x0c87c,0x0d9f5,
		0x03183,0x0200a,0x01291,0x00318,0x077a7,0x0662e,0x054b5,0x0453c,
		0x0bdcb,0x0ac42,0x09ed9,0x08f50,0x0fbef,0x0ea66,0x0d8fd,0x0c974,
		0x04204,0x0538d,0x06116,0x0709f,0x00420,0x015a9,0x02732,0x036bb,
		0x0ce4c,0x0dfc5,0x0ed5e,0x0fcd7,0x08868,0x099e1,0x0ab7a,0x0baf3,
		0x05285,0x0430c,0x07197,0x0601e,0x014a1,0x00528,0x037b3,0x0263a,
		0x0decd,0x0cf44,0x0fddf,0x0ec56,0x098e9,0x08960,0x0bbfb,0x0aa72,
		0x06306,0x0728f,0x04014,0x0519d,0x02522,0x034ab,0x00630,0x017b9,
		0x0ef4e,0x0fec7,0x0cc5c,0x0ddd5,0x0a96a,0x0b8e3,0x08a78,0x09bf1,
		0x07387,0x0620e,0x05095,0x0411c,0x035a3,0x0242a,0x016b1,0x00738,
		0x0ffcf,0x0ee46,0x0dcdd,0x0cd54,0x0b9eb,0x0a862,0x09af9,0x08b70,
		0x08408,0x09581,0x0a71a,0x0b693,0x0c22c,0x0d3a5,0x0e13e,0x0f0b7,
		0x00840,0x019c9,0x02b52,0x03adb,0x04e64,0x05fed,0x06d76,0x07cff,
		0x09489,0x08500,0x0b79b,0x0a612,0x0d2ad,0x0c324,0x0f1bf,0x0e036,
		0x018c1,0x00948,0x03bd3,0x02a5a,0x05ee5,0x04f6c,0x07df7,0x06c7e,
		0x0a50a,0x0b483,0x08618,0x09791,0x0e32e,0x0f2a7,0x0c03c,0x0d1b5,
		0x02942,0x038cb,0x00a50,0x01bd9,0x06f66,0x07eef,0x04c74,0x05dfd,
		0x0b58b,0x0a402,0x09699,0x08710,0x0f3af,0x0e226,0x0d0bd,0x0c134,
		0x039c3,0x0284a,0x01ad1,0x00b58,0x07fe7,0x06e6e,0x05cf5,0x04d7c,
		0x0c60c,0x0d785,0x0e51e,0x0f497,0x08028,0x091a1,0x0a33a,0x0b2b3,
		0x04a44,0x05bcd,0x06956,0x078df,0x00c60,0x01de9,0x02f72,0x03efb,
		0x0d68d,0x0c704,0x0f59f,0x0e416,0x090a9,0x08120,0x0b3bb,0x0a232,
		0x05ac5,0x04b4c,0x079d7,0x0685e,0x01ce1,0x00d68,0x03ff3,0x02e7a,
		0x0e70e,0x0f687,0x0c41c,0x0d595,0x0a12a,0x0b0a3,0x08238,0x093b1,
		0x06b46,0x07acf,0x04854,0x059dd,0x02d62,0x03ceb,0x00e70,0x01ff9,
		0x0f78f,0x0e606,0x0d49d,0x0c514,0x0b1ab,0x0a022,0x092b9,0x08330,
		0x07bc7,0x06a4e,0x058d5,0x0495c,0x03de3,0x02c6a,0x01ef1,0x00f78
	};

	unsigned int crc = 0xffff;
	unsigned int lookup_value;

	while(data_length--)
	{
		crc &= 0xffff;
		lookup_value = (crc_values[(crc ^ *(data++)) & 0xff]) & 0xffff;
		crc = (crc >> 8) ^ lookup_value;
	}

	return((~crc) & 0xffff);
}

void DeviceCF635::_drain(int timeout) throw(string)
{
	unsigned char	received_packet[32];
	ssize_t			received;
	struct pollfd	pfd;
	int				pollresult;

	if(!_fd == -1)
		throw(string("DeviceCF635::_drain: device not open"));

	if(debug == 1)
		vlog("drain(%d)\n", timeout);

	usleep(timeout);

	for(;;)
	{
		pfd.fd = _fd;
		pfd.events = POLLIN;

		if((pollresult = ::poll(&pfd, 1, 10)) < 0)
		{
			if(errno == EINTR)
				continue;
			else
				throw(string("DeviceCF635::_command::_drain poll error"));
		}

		if(pollresult == 0)
			break;

		if((received = ::read(_fd, received_packet, sizeof(received_packet))) < 0)
			throw(string("DeviceCF635::drain::read"));

		if(received == 0)
			break;

		if(debug == 1)
			vlog("drained %d bytes\n", (int)received);
	}
}

void DeviceCF635::_command(int command,
		size_t data_length, const unsigned char * data,
		int & received_command,
		size_t & received_data_length, size_t received_buffer_length, unsigned char * received_buffer) throw(string)
{
	unsigned char	send_packet[32];
	unsigned char	received_packet[32];
	unsigned int	crc, received_crc;
	int				type;
	struct pollfd	pfd;
	int				pollresult;
	int				attempt;
	size_t			received;

	if(!_fd == -1)
		throw(string("DeviceCF635::_command: device not open"));

	if(data_length > 22)
		throw(string("DeviceCF635::_command: data_length > 22"));

	send_packet[0] = command & 0x3f;
	send_packet[1] = data_length;

	memcpy(send_packet + 2, data, data_length);

	crc = _crc32(data_length + 2, send_packet);

	if(debug == 1)
		vlog("crc = %x\n", _crc32(data_length + 2, send_packet));

	send_packet[data_length + 2 + 0] = (crc >> 0) & 0xff;
	send_packet[data_length + 2 + 1] = (crc >> 8) & 0xff;

	if(debug == 1)
	{
		vlog("sending send_packet[%d]: ", (int)data_length + 4);
		size_t ix;
		for(ix = 0; ix < data_length + 4; ix++)
			vlog("%02x ", (int)send_packet[ix]);
		vlog("\n\n");
	}

	for(attempt = 0; attempt < 5; attempt++)
	{
		if(debug == 1)
			vlog("attempt %d\n", attempt);

		_drain(150000 * attempt);

		if(debug == 1)
			vlog("write\n");

		if((size_t)::write(_fd, send_packet, data_length + 4) != (data_length + 4))
			throw(string("DeviceCF635::command::write"));

		usleep(10);

		if(debug == 1)
			vlog("written\n");

		pfd.fd = _fd;
		pfd.events = POLLIN;

		if((pollresult = ::poll(&pfd, 1, 10)) < 0)
		{
			if(errno == EINTR)
				continue;
			else
				throw(string("DeviceCF635::_command::read poll error"));
		}

		if(pollresult == 0)
			continue;

		if((received = ::read(_fd, received_packet, sizeof(received_packet))) <= 0)
			throw(string("DeviceCF635::command::read"));

		if(debug == 1)
			vlog("received %d bytes, attempt %d\n", (int)received, attempt);

		type = (received_packet[0] & 0xc0) >> 6;

		switch(type)
		{
			case(0):
			{
				if(debug == 1)
					vlog("DeviceCF635::command::read type = 00 (command), discarding packet\n");
				continue;
				break;
			}

			case(1):
			{
				break;
			}

			case(2):
			{
				if(debug == 1)
					vlog("DeviceCF635::command::read type = 10 (report), discarding packet\n");
				continue;
				break;
			}

			case(3):
			{
				if(debug == 1)
					vlog("DeviceCF635::command::read type = 11 (error), report error\n");
				throw(string("DeviceCF635::command::read: cf635 reports error"));
			}
		}

		received_command		= received_packet[0] & 0x3f;
		received_data_length	= received_packet[1];

		if(received_data_length > 22)
		{
			if(debug == 1)
				vlog("DeviceCF635::command::read received_data_length(%d) > 22, discarding packet\n", (int)received_data_length);
			continue;
		}

		if(received_data_length > received_buffer_length)
		{
			if(debug == 1)
				vlog("DeviceCF635::command::read received_data_length(%d) > received_buffer_length(%d), discarding packet\n",
					(int)received_data_length, (int)received_buffer_length);
			continue;
		}

		memcpy(received_buffer, received_packet + 2, received_data_length);

		crc				= _crc32(received_data_length + 2, received_packet);
		received_crc	= (received_packet[received_data_length + 2 + 0] >> 0) | (received_packet[received_data_length + 2 + 1] << 8);

		if((debug) && (attempt == 9))
		{
			vlog("received packet[%d]: ", (int)received_data_length + 4);
			size_t ix;
			for(ix = 0; ix < received_data_length + 4; ix++)
				vlog("%02x ", (int)received_packet[ix]);

			vlog("\ncrc = %x, received_crc = %x\n\n", crc, received_crc);
		}

		if(crc != received_crc)
		{
			if(debug == 1)
				vlog("DeviceCF635::command::read crc does not match, discarding packet\n");
		}
		else
			return;
	}

	throw(string("DeviceCF635::command::read: timeout"));
}

void DeviceCF635::_sendline(int yy, int xx, size_t data_length, const char * data) throw(string)
{
	unsigned char	send_buffer[16];
	int				received_command;
	size_t			received_buffer_length;
	unsigned char	received_buffer[32];
	char			print_buffer[32];

	if(!_fd == -1)
		throw(string("DeviceCF635::_sendline: device not open"));

	if((xx < 0) || (xx > 19))
		throw(string("DeviceCF635::_sendline: x out of range"));

	if((yy < 0) || (yy > 3))
		throw(string("DeviceCF635::_sendline: y out of range"));

	if(data_length > 20)
		throw(string("DeviceCF635::_sendline: data length out of range"));

	send_buffer[0] = xx;
	send_buffer[1] = yy;

	memcpy(send_buffer + 2, data, data_length);

	_command(0x1f, data_length + 2, send_buffer, received_command, received_buffer_length, sizeof(received_buffer), received_buffer);

	if(received_command != 0x1f)
	{
		snprintf(print_buffer, sizeof(print_buffer), "0x%x", received_command);
		throw(string("DeviceCF635::_sendline: invalid reply (command=") + print_buffer + ")");
	}

	if(received_buffer_length != 0)
	{
		snprintf(print_buffer, sizeof(print_buffer), "%d", (int)received_buffer_length);
		throw(string("DeviceCF635::_sendline: invalid reply (length=") + print_buffer + ")");
	}
}

void DeviceCF635::_setbright(int value) throw(string)
{
	unsigned char	send_buffer;
	int				received_command;
	size_t			received_buffer_length;
	unsigned char	received_buffer[32];
	char			print_buffer[32];

	static const int value2brightness[] =
	{
		0,		//  off
		1,		//  very low
		6,		//  low
		12,		//  normal
		64		//  bright
	};

	static const int value2contrast[] =
	{
		0,		//  off
		120,	//  very low
		120,	//  low
		120,	//  normal
		110		//  bright
	};

	if((value < 0) | (value > 4))
		throw(string("DeviceCF635::_setbright: value out of range\n"));

	if(!_fd == -1)
		throw(string("DeviceCF635::_setbright: device not open"));

	send_buffer = value2brightness[value];

	_command(0x0e, 1, &send_buffer, received_command, received_buffer_length, sizeof(received_buffer), received_buffer);

	if(received_command != 0x0e)
	{
		snprintf(print_buffer, sizeof(print_buffer), "0x%x", received_command);
		throw(string("DeviceCF635::_setbright: invalid reply (command=") + print_buffer + ")");
	}

	if(received_buffer_length != 0)
	{
		snprintf(print_buffer, sizeof(print_buffer), "%d", (int)received_buffer_length);
		throw(string("DeviceCF635::_setbright: invalid reply (length=") + print_buffer + ")");
	}

	send_buffer	= value2contrast[value];

	_command(0x0d, 1, &send_buffer, received_command, received_buffer_length, sizeof(received_buffer), received_buffer);

	if(received_command != 0x0d)
	{
		snprintf(print_buffer, sizeof(print_buffer), "0x%x", received_command);
		throw(string("DeviceCF635::_setbright: invalid reply (command=") + print_buffer + ")");
	}

	if(received_buffer_length != 0)
	{
		snprintf(print_buffer, sizeof(print_buffer), "%d", (int)received_buffer_length);
		throw(string("DeviceCF635::_setbright: invalid reply (length=") + print_buffer + ")");
	}
}

void DeviceCF635::_setled(int led, bool red, bool green) throw(string)
{
	unsigned char	send_data[8];
	int				received_command;
	size_t			received_data_length;
	unsigned char	received_data[32];

	static const int brightness2led[] =
	{
		0,		//  off
		1,		//  very low
		4,		//  low
		8,		//  normal
		100		//  bright
	};

	if(!_fd == -1)
		throw(string("DeviceCF635::_setled: device not open"));

	if((led < 0) || (led > 3))
		throw(string("DeviceCF635::command::_setled led index out of range"));

	led = 11 - (led << 1);

	send_data[0] = led;
	send_data[1] = green ? brightness2led[_brightness] : 0;

	_command(0x22, 2, send_data, received_command, received_data_length, sizeof(received_data), received_data);

	if(received_command != 0x22)
		throw(string("DeviceCF635::_setled: reply command does not match"));

	if(received_data_length != 0)
		throw(string("DeviceCF635::_setled: received data size does not match"));

	send_data[0] = led + 1;
	send_data[1] = red ? brightness2led[_brightness] : 0;

	_command(0x22, 2, send_data, received_command, received_data_length, sizeof(received_data), received_data);

	if(received_command != 0x22)
		throw(string("DeviceCF635::_setled: reply command does not match"));

	if(received_data_length != 0)
		throw(string("DeviceCF635::_setled: received data size does not match"));
}

void DeviceCF635::_ping() throw(string)
{
	unsigned char	send_buffer[16];
	size_t			ix;
	int				received_command;
	size_t			received_buffer_length;
	unsigned char	received_buffer[32];

	if(!_fd == -1)
		throw(string("DeviceCF635::_ping: device not open"));

	for(ix = 0; ix < sizeof(send_buffer); ix++)
		send_buffer[ix] = (120 + ix) & 0xff;

	_command(0x00, 16, send_buffer, received_command, received_buffer_length, sizeof(received_buffer), received_buffer);

	if(received_command != 0)
		throw(string("DeviceCF635::command::_ping reply command does not match"));

	if(memcmp(send_buffer, received_buffer, 16))
		throw(string("DeviceCF635::command::_ping data does not match"));
}

void DeviceCF635::_initgpio() throw(string)
{
	unsigned char	send_data[8];
	size_t			ix;
	int				received_command;
	size_t			received_data_length;
	unsigned char	received_data[32];

	if(!_fd == -1)
		throw(string("DeviceCF635::_initgpio: device not open"));

	for(ix = 12; ix > 4; ix--)
	{
		send_data[0] = ix;
		send_data[1] = 0;
		send_data[2] = 0x09; // 00001001 fast strong drive up, fast strong drive down

		_command(0x22, 3, send_data, received_command, received_data_length, sizeof(received_data), received_data);

		if(received_command != 0x22)
			throw(string("DeviceCF635::command::_initgpio reply command does not match"));

		if(received_data_length != 0)
			throw(string("DeviceCF635::command::_initgpio received data size does not match"));
	}
}

void DeviceCF635::_initkeyreporting() throw(string)
{
	unsigned char	send_data[2];
	int				received_command;
	size_t			received_data_length;
	unsigned char	received_data[32];

	if(!_fd == -1)
		throw(string("DeviceCF635::_initkeyreporting: device not open"));

	send_data[0] = send_data[1] = 0;

	_command(0x17, 2, send_data, received_command, received_data_length, sizeof(received_data), received_data);

	if(received_command != 0x17)
		throw(string("DeviceCF635::command::_initkeyreporting reply command does not match"));

	if(received_data_length != 0)
		throw(string("DeviceCF635::command::_initkeyreporting received data size does not match"));
}

void DeviceCF635::_readkeys(int & down, int & pressed, int & released) throw(string)
{
	unsigned char	send_data[1];
	int				received_command;
	size_t			received_data_length;
	unsigned char	received_data[32];

	if(!_fd == -1)
		throw(string("DeviceCF635::_readkeys: device not open"));

	_command(0x18, 0, send_data, received_command, received_data_length, sizeof(received_data), received_data);

	if(received_command != 0x18)
		throw(string("DeviceCF635::command::_readkeys reply command does not match"));

	if(received_data_length != 3)
		throw(string("DeviceCF635::command::_readkeys received data size does not match"));

	down		= received_data[0];
	pressed		= received_data[1];
	released	= received_data[2];
}

int DeviceCF635::width() const
{
	return(20);
}

int DeviceCF635::height() const
{
	return(4);
}

void DeviceCF635::poll() throw(string)
{
	int			down, pressed, released;
	int			bit;

	if(!_fd == -1)
		throw(string("DeviceCF635::poll: device not open"));

	_readkeys(down, pressed, released);

	for(bit = 0; bit < 7; bit++)
	{
		if(pressed & 0x01)
		{
			if(bit == 2) // OK
			{
				if(_brightness > 0)
				{
					_brightness--;
					_setbright(_brightness);
				}
			}
			else
				if(bit == 1) // cancel
				{
					if(_brightness < 4)
					{
						_brightness++;
						_setbright(_brightness);
					}
				}
				else
					_addkey(bit, true);
		}

		pressed >>= 1;

		if(released & 0x01)
			if((bit != 1) && (bit != 2))
				_addkey(bit, false);

		released >>= 1;
	}
}

void DeviceCF635::__update() throw(string)
{
	char line[width()];
	int yy;
	int xx;
	int led;
	int max;
	int bright;

	if(!_fd == -1)
		throw(string("DeviceCF635::__update: device not open"));

	bright = _brightness;

	if(_standout && (_brightness > 0))
	{
		bright = bright + 1;

		if(bright > 4)
			bright = 2;
	}

	_setbright(bright);

	max = _progress / 20;

	if(max > 4)
		max = 4;

	for(led = 0; led < 4; led++)
		if(led < max)
			_setled(3 - led, !!_standout, !_standout);
		else
			_setled(3 - led, 0, 0);

	for(yy = 0; yy < height(); yy++)
	{
		memcpy(line, &_textbuffer[yy * width()], width());

		for(xx = 0; xx < width(); xx++)
		{
			switch(line[xx])
			{
				case('@'): line[xx] = 160; break;
				case('['): line[xx] = 250; break;
				case(']'): line[xx] = 252; break;
			}
		}

		_sendline(yy, 0, width(), line);
	}

	_setbright(_brightness);
}
