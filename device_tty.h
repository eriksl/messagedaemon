#ifndef _device_tty_h_
#define _device_tty_h_

#include <string>
using std::string;

#include "device.h"

class DeviceTty : public Device
{
	private:

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

	public:

				DeviceTty() throw(string);
				~DeviceTty();
		int		width() const;
		int		height() const;
};

#endif
