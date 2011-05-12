#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "device.h"
#include "device_cf634.h"
#include "syslog.h"

DeviceCF634::DeviceCF634(string node) throw(string)
{
	_fd = -1;

	if(node == "")
		_device_node = "/dev/ttyUSB0";
	else
		_device_node = node;
}

DeviceCF634::~DeviceCF634()
{
	__close();
}

void DeviceCF634::__open() throw(string)
{
	if(_fd != -1)
		throw(string("DeviceCF634::DeviceCF634::__open: fd already open"));

	if((_fd = ::open(_device_node.c_str(), O_RDWR | O_NOCTTY, 0)) < 0)
		throw(string("DeviceCF634::DeviceCF634::__open"));

	_initserial();
	_command(3);				// display on
	_command(20);				// scroll off
	_command(24);				// wrap off
	_command(4);				// cursor off
	_command(31);				// show system info
}

void DeviceCF634::__close()
{
	if(_fd == -1)
		return;

	try
	{
		clear();
		_command(2);			// display off
		__update();
	}
	catch(...)
	{
	}

	::close(_fd);
	_fd = -1;
}

void DeviceCF634::_initserial() throw(string)
{
	int				result;
	struct termios	tio;

	if(_fd == -1)
		throw(string("DeviceCF634::_initserial: fd not open"));

	if(ioctl(_fd, TIOCMGET, &result))
		throw(string("DeviceCF634::DeviceCF634::ioctl(TIOCMGET)"));

	result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(_fd, TIOCMSET, &result))
		throw(string("DeviceCF634::DeviceCF634::ioctl(TIOCMSET)"));

	result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(_fd, TIOCMSET, &result))
		throw(string("DeviceCF634::DeviceCF634::ioctl(TIOCMSET)"));

	if(tcgetattr(_fd, &tio) == 1)
		throw(string("DeviceCF634::DeviceCF634::tcgetattr"));

	tio.c_iflag &= ~(BRKINT | INPCK	| INLCR | IGNCR	| IUCLC |
					IXON	| IXOFF	| IXANY | IMAXBEL | ISTRIP | ICRNL);
	tio.c_iflag |=	(IGNBRK | IGNPAR);

	tio.c_oflag &= ~(OPOST | OLCUC | OCRNL | ONOCR | ONLRET | OFILL | ONLCR);
	tio.c_oflag |=	0;

	tio.c_cflag &=	~(CSIZE | PARENB | PARODD	| HUPCL | CRTSCTS);
	tio.c_cflag |=	(CREAD | CS8 | CSTOPB | CLOCAL);

	tio.c_lflag &= ~(ISIG	| ICANON	| XCASE	| ECHO	| ECHOE	| ECHOK |
					ECHONL | ECHOCTL	| ECHOPRT | ECHOKE | FLUSHO | TOSTOP |
					PENDIN | IEXTEN		| NOFLSH);
	tio.c_lflag |=	0;

	cfsetispeed(&tio, B19200);
	cfsetospeed(&tio, B19200);

	if(tcsetattr(_fd, TCSANOW, &tio) == 1)
		throw(string("DeviceCF634::DeviceCF634::tcsetattr"));
}

void DeviceCF634::_command(int a, int b, int c, int d) throw(string)
{
	char commandstr[8];
	ssize_t length = 1;

	if(_fd == -1)
		throw(string("DeviceCF634::_command: fd not open"));

	commandstr[0] = a;

	if(b != -1)
	{
		commandstr[1] = b;
		length++;

		if(c != -1)
		{
			commandstr[2] = c;
			length++;

			if(d != -1)
			{
				commandstr[3] = d;
				length++;
			}
		}
	}

	if(::write(_fd, commandstr, length) != length)
		throw(string("DeviceCF634::command::write"));
}

int DeviceCF634::width() const
{
	return(20);
}

int DeviceCF634::height() const
{
	return(4);
}

void DeviceCF634::__update() throw(string)
{
	int xx, yy;
	char current;

	if(_fd == -1)
		throw(string("DeviceCF634::__update: fd not open"));

	if(_brightness)
		_command(3);			// display on
	else
		_command(2);			// display off

	if(_standout)
		_command(15, 100);		// high contrast

	for(yy = 0; yy < height(); yy++)
	{
		_command(17, 0, yy); // cursor

		for(xx = 0; xx < width(); xx++)
		{
			current = _textbuffer[xx + (yy * width())];

			if(current == '_')
				current = 196;

			if(current == '[')
				current = 250;

			if(current == ']')
				current = 252;

			if(current == '@')
				current = 160;

			if(::write(_fd, &current, 1) != 1)
				throw(string("DeviceCF634::Update::write"));
		}
	}

	if(_standout)
		usleep(200000);

	_command(15, 55);			// medium contrast
}
