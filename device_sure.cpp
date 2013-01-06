/* Copyright Erik Slagter, GPLv2 is applicable */

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
using boost::bad_lexical_cast;

#include "device.h"
#include "device_sure.h"
#include "syslog.h"

DeviceSure::DeviceSure(string node) throw(string)
{
	_fd			= -1;
	_flashcycle = 0;

	if(node == "")
		_device_node = "/dev/ttyUSB0";
	else
		_device_node = node;
}

DeviceSure::~DeviceSure()
{
	__close();
}

void DeviceSure::__open() throw(string)
{
	if(_fd != -1)
		throw(string("DeviceSure::__open: device not closed"));

	if((_fd = ::open(_device_node.c_str(), O_RDWR | O_NOCTTY | O_EXCL, 0)) < 0)
		throw(string("DeviceSure::DeviceSure::open"));

	ioctl(_fd, TIOCEXCL, 1);

	_initserial();
	_init();
}

void DeviceSure::__close()
{
	if(_fd == -1)
		return;

	try
	{
		clear();
		__update();
		_setbright(0);
		usleep(200000);
	}
	catch(...)
	{
	}
	
	::close(_fd);
	_fd = -1;
}

int DeviceSure::width() const
{
	return(20);
}

int DeviceSure::height() const

{
	return(4);
}

size_t DeviceSure::_pollread(size_t size, uint8_t * buffer) const throw(string)
{
	int				rv;
	struct pollfd	pfd = { _fd, POLLIN, 0 };

	rv = ::poll(&pfd, 1, 5000);

	if(rv == 0)
		throw(string("DeviceSure::_pollread: no response from device"));

	if(rv < 0)
		throw(string("DeviceSure::_pollread: error in reading from device (1)"));

	rv = ::read(_fd, buffer, size);

	if(rv == 0)
		throw(string("DeviceSure::_pollread: device returns EOF"));

	if(rv < 0)
		throw(string("DeviceSure::_pollread: error in reading from device (2)"));

	if(rv != (int)size)
		throw(string("DeviceSure::_pollread: error in reading from device (3)"));

	return(rv);
}

size_t DeviceSure::_pollwrite(size_t size, const uint8_t * buffer) const throw(string)
{
	int				rv;
	struct pollfd	pfd = { _fd, POLLOUT, 0 };

	rv = ::poll(&pfd, 1, 5000);

	if(rv == 0)
		throw(string("DeviceSure::_pollwrite: no response from device"));

	if(rv < 0)
		throw(string("DeviceSure::_pollwrite: error in writing to device (1)"));

	rv = ::write(_fd, buffer, size);

	if(rv == 0)
		throw(string("DeviceSure::_pollwrite: device returns EOF"));

	if(rv < 0)
		throw(string("DeviceSure::_pollwrite: error in writing to device (2)"));

	if(rv != (int)size)
		throw(string("DeviceSure::_pollwrite: error in writing to device (3)"));

	return(rv);
}

void DeviceSure::_initserial() throw(string)
{
	int				result;
	struct termios	tio;

	if(_fd == -1)
		throw(string("DeviceSure::_initserial: device not open"));

	if(ioctl(this->_fd, TIOCMGET, &result))
		throw(string("DeviceSure::DeviceSure::ioctl(TIOCMGET)"));

	result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(this->_fd, TIOCMSET, &result))
		throw(string("DeviceSure::DeviceSure::ioctl(TIOCMSET)"));

	result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(this->_fd, TIOCMSET, &result))
		throw(string("DeviceSure::DeviceSure::ioctl(TIOCMSET)"));

	if(tcgetattr(this->_fd, &tio) == 1)
		throw(string("DeviceSure::DeviceSure::tcgetattr"));

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

	cfsetispeed(&tio, B9600);
	cfsetospeed(&tio, B9600);

	if(tcsetattr(this->_fd, TCSANOW, &tio) == 1)
		throw(string("DeviceSure::DeviceSure::tcsetattr"));
}

void DeviceSure::_init() throw(string)
{
	uint8_t command[32] = { 0xfe, 0x53, 0x75, 0x72, 0x65 }; // establish communication

	if(_fd == -1)
		throw(string("DeviceSure::_init: device not open"));

	_pollwrite(5, command);

	usleep(50000);

	command[1] = 0x50;	//	contrast = 0
	command[2] = 0;

	_pollwrite(3, command);

	usleep(50000);

	command[1] = 0x57;	// temperature = celsius
	command[2] = 0x43;

	_pollwrite(3, command);
}

string DeviceSure::identify() const throw()
{
#if 0
	char command[32] = { 0xfe, 0x76 }; 

	_pollwrite(2, command);
	usleep(50000);
	_pollread(11, command);

	command[11] = 0;
	string rrr = command + 8;

	command[8] = 0;
	string t = command + 7;

	command[7] = 0;
	string l = command + 6;

	command[6] = 0;
	string e = command + 5;

	command[5] = 0;
	string p = command + 4;

	command[4] = 0;
	string abcd = command;

	string rv = string("identity: ") + abcd + ", rx8025: " + p + ", eprom size: " + e + ", photo resistor: " + l + ", thermal resistor: " + t + ", reserved: " + rrr;

	return(rv);
#endif
	return(string("Sure 2004 USB LCD"));
}

void DeviceSure::_setbright(int value) throw(string)
{
	static const int value2brightness[] =
	{
		0,		//	not used, off
		16,		//	very low
		64,		//	low
		128,	//	normal
		253		//	bright
	};

	uint8_t command[16];
	int length = 0;

	if(_fd == -1)
		throw(string("DeviceSure::_setbright: device not open"));

	command[length++] = 0xfe;

	if(_brightness == 0)
	{
		command[length++] = 0x98;
		command[length++] = 0;
		command[length++] = 0xfe;
		command[length++] = 0x46;
	}
	else
	{
		command[length++] = 0x42;
		command[length++] = 0x00;
		command[length++] = 0xfe;
		command[length++] = 0x98;
		command[length++] = value2brightness[value];
	}

	_pollwrite(length, command);
}

void DeviceSure::__update() throw(string)
{
	static const int bright2standout[] = { 0, 2, 3, 4, 4 };
	int		xx, yy;
	uint8_t	current;
	uint8_t	line[width() + 8];

	if(_fd == -1)
		throw(string("DeviceSure::__update: device not open"));

	if(_standout)
		_setbright(bright2standout[_brightness]);
	else
		_setbright(_brightness);

	for(yy = 0; yy < height(); yy++)
	{
		line[0] = 0xfe;
		line[1] = 0x47;
		line[2] = 0x01;
		line[3] = yy + 1;

		for(xx = 0; xx < width(); xx++)
		{
			current = _textbuffer[xx + (yy * width())];
			line[xx + 4] = current;
		}

		_pollwrite(width() + 4, line);
	}

	_setbright(_brightness);
}

int DeviceSure::analog_inputs() const throw()
{
	return(1);
}

int DeviceSure::max_analog() const throw()
{
	return(100);
}

int DeviceSure::__read_analog(int) throw(string)
{
	uint8_t	command[16];
	uint8_t *	cptr;
	int		tries;
	bool	done;
	int		rv;

	for(done = false, tries = 10; !done && (tries > 0); tries--)
	{
		try
		{
			command[0] = 0xfe;
			command[1] = 0x77;

			usleep(200000);
			_pollwrite(2, command);
			usleep(200000);
			_pollread(5, command);

		}
		catch(const string & e)
		{
		}

		done = true;
	}

	if(!done)
		throw(string("DeviceSure::__read_analog::timeout"));

	command[3] = '\0';

	cptr = command;

	if(*cptr == ' ')
		cptr++;

	try
	{
		rv = lexical_cast<int>(cptr);
	}
	catch(bad_lexical_cast)
	{
		rv = -1;
	}

	return(rv);
}
