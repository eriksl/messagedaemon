#ifndef _device_cf634_h_
#define _device_cf634_h_

#include <string>
using std::string;

#include "device.h"

class DeviceCF634 : public Device
{
	private:

		int		_fd;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

		void	_initserial() throw(string);
		void	_command(int a, int b = -1, int c = -1, int d = -1) throw(string);

	public:

		DeviceCF634(string) throw(string);
		~DeviceCF634();

		int	width() const;
		int	height() const;
};

#endif
