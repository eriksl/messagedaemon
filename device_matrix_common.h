#ifndef _device_matrix_common_h_
#define _device_matrix_common_h_

#include <stdint.h>

#include <string>
using std::string;

#include "device.h"

class DeviceMatrixCommon : public Device
{
	private:

				int		_fd;

				void	_initserial()			throw(string);
				void	_setbright(int value)	throw(string);

		virtual	void	__init()				throw(string) = 0;

		virtual	void	__reinit()				throw(string);
		virtual	void	__setbright(int value)	throw(string);

	protected:

				void	_command(int a, int b = -1, int c = -1, int d = -1) throw(string);
				void	_read_reply(size_t length, uint8_t * buffer) throw(string);
				void	__update()				throw(string);
				void	__open()				throw(string);
				void	__close();

	public:

						DeviceMatrixCommon() throw(string);
		virtual			~DeviceMatrixCommon();
				int		width() const;
				int		height() const;
};

#endif
