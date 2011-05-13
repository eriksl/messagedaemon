#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "device.h"
#include "device_matrix_common.h"
#include "syslog.h"

DeviceMatrixCommon::DeviceMatrixCommon() throw(string)
{
	_fd = -1;
}

DeviceMatrixCommon::~DeviceMatrixCommon()
{
}

void DeviceMatrixCommon::__reinit() throw(string)
{
}

void DeviceMatrixCommon::__setbright(int) throw(string)
{
}

void DeviceMatrixCommon::__open() throw(string)
{
	if(_fd != -1)
		throw(string("DeviceMatrixCommon::__open: device not closed"));

	if((_fd = ::open(_device_node.c_str(), O_RDWR, 0)) < 0)
		throw(string("DeviceMatrixCommon::DeviceMatrixCommon::open(") + _device_node + ")");

	_initserial();
	__init();
}

void DeviceMatrixCommon::__close()
{
	if(_fd == -1)
		return;

	try
	{
		clear();
		_brightness = 0;
		__update();
	}
	catch(...)
	{
	}

	::close(_fd);
	_fd = -1;
}

void DeviceMatrixCommon::_initserial() throw(string)
{
	int				result;
	struct termios	tio;

	if(_fd == -1)
		throw(string("DeviceMatrixCommon::_initserial: device not open"));

	if(ioctl(_fd, TIOCMGET, &result))
		throw(string("DeviceMatrixCommon::DeviceMatrixCommon::ioctl(TIOCMGET)"));

	result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(_fd, TIOCMSET, &result))
		throw(string("DeviceMatrixCommon::DeviceMatrixCommon::ioctl(TIOCMSET)"));

	result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(_fd, TIOCMSET, &result))
		throw(string("DeviceMatrixCommon::DeviceMatrixCommon::ioctl(TIOCMSET)"));

	if(tcgetattr(_fd, &tio) == 1)
		throw(string("DeviceMatrixCommon::DeviceMatrixCommon::tcgetattr"));

	tio.c_iflag &= ~(BRKINT | INPCK	| INLCR | IGNCR	| IUCLC |
					IXON	| IXOFF	| IXANY | IMAXBEL | ISTRIP | ICRNL);
	tio.c_iflag |=	(IGNBRK | IGNPAR);

	tio.c_oflag &= ~(OPOST | OLCUC | OCRNL | ONOCR | ONLRET | OFILL | ONLCR);
	tio.c_oflag |=	0;

	tio.c_cflag &=	~(CSTOPB | PARENB | PARODD	| HUPCL);
	tio.c_cflag |=	(CS8	| CREAD	| CLOCAL | CRTSCTS);

	tio.c_lflag &= ~(ISIG	| ICANON	| XCASE	| ECHO	| ECHOE	| ECHOK |
					ECHONL | ECHOCTL	| ECHOPRT | ECHOKE | FLUSHO | TOSTOP |
					PENDIN | IEXTEN		| NOFLSH);
	tio.c_lflag |=	0;

	cfsetospeed(&tio, B19200);

	if(tcsetattr(_fd, TCSANOW, &tio) == 1)
		throw(string("DeviceMatrixCommon::DeviceMatrixCommon::tcsetattr"));
}

void DeviceMatrixCommon::_command(int a, int b, int c, int d) throw(string)
{
	char commandstr[8];
	ssize_t length = 2;

	if(_fd == -1)
		throw(string("DeviceMatrixCommon::_command: device not open"));

	commandstr[0] = 0xfe;
	commandstr[1] = a;

	if(b != -1)
	{
		commandstr[2] = b;
		length++;

		if(c != -1)
		{
			commandstr[3] = c;
			length++;

			if(d != -1)
			{
				commandstr[4] = d;
				length++;
			}
		}
	}

	if(::write(_fd, commandstr, length) != length)
		throw(string("DeviceMatrixCommon::command::write"));

	usleep(5000);
}

void DeviceMatrixCommon::_read_reply(size_t length, uint8_t * buffer) throw(string)
{
	int rv;

	if((rv = read(_fd, buffer, length)) != length)
		throw(string("DeviceMatrixCommon::_read_reply"));
}

int DeviceMatrixCommon::width() const
{
	return(20);
}

int DeviceMatrixCommon::height() const
{
	return(4);
}

void DeviceMatrixCommon::_setbright(int value) throw(string)
{
	if(_fd == -1)
		throw(string("DeviceMatrixCommon::_setbright: device not open"));

	if((value < 0) || (value > 4))
		throw(string("DeviceMatrixCommon::_setbright: value out of range"));

	__setbright(value);
}

void DeviceMatrixCommon::__update() throw(string)
{
	int yy;
	int bright;
	char * current;

	if(_fd == -1)
		throw(string("DeviceMatrixCommon::__update: device not open"));

	__reinit();

	bright = _brightness;

	if(_standout && (_brightness > 0))
	{
		bright = bright + 1;

		if(bright > 4)
			bright = 2;
	}

	_setbright(bright);

	_command('H'); // 0x48 cursor home

	for(yy = 0; yy < height(); yy++)
	{
		current = _textbuffer + (yy * width());

		if(::write(_fd, current, width()) != width())
			throw(string("DeviceMatrixCommon::Update::write"));
	}

	usleep(200000);

	_setbright(_brightness);
}
