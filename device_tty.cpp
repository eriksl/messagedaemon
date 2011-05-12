#include "device.h"
#include "device_tty.h"
#include "syslog.h"

DeviceTty::DeviceTty() throw(string)
{
}

DeviceTty::~DeviceTty()
{
	__close();
}

void DeviceTty::__open() throw(string)
{
}

void DeviceTty::__close()
{
}

int DeviceTty::width() const
{
	return(20);
}

int DeviceTty::height() const
{
	return(4);
}

void DeviceTty::__update() throw(string)
{
	char *	current;
	int		yy;

	for(yy = 0; yy < height(); yy++)
	{
		current = _textbuffer + (width() * yy);
		printf("%.20s\n", current);
	}

	printf("\n");
}
