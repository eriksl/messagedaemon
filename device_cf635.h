/* Copyright Erik Slagter, GPLv2 is applicable */

#ifndef _device_cf635_h_
#define _device_cf635_h_

#include <string>
using std::string;

#include "device.h"

class DeviceCF635 : public Device
{
	private:

		int		_fd;
		int		_flashcycle;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

		void	_initserial() throw(string);
		void	_command(int command,
						size_t data_length, const unsigned char * data,
						int & received_command,
						size_t & received_data_length, size_t received_buffer_length, unsigned char * received_buffer)
							throw(string);
		void	_sendline(int y, int x, size_t data_length, const char * data) throw(string);
		void	_drain(int timeout) throw(string);
		void	_initgpio() throw(string);
		void	_initkeyreporting() throw(string);
		void	_setcontrast(int value) throw(string);
		void	_setled(int led, bool red, bool green) throw(string);
		void	_readkeys(int & down, int & pressed, int & released) throw(string);
		void	_setbright(int value) throw(string);
		void	_ping() throw(string);
		void	poll() throw(string);

		static unsigned int _crc32(size_t data_length, const unsigned char * data);

	public:

		DeviceCF635(string) throw(string);
		~DeviceCF635();

		int width() const;
		int	height() const;
};

#endif
