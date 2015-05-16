/* Copyright Erik Slagter, GPLv2 is applicable */

#ifndef _device_esp_h_
#define _device_esp_h_

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "device.h"

class DeviceESP : public Device
{
	private:

		string			devicenode;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

	public:

				DeviceESP(string devices) throw(string);
				~DeviceESP();
		int		width() const;
		int		height() const;
		string	identify() const throw();
};

#endif
