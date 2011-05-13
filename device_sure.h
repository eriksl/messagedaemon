/* Copyright Erik Slagter, GPLv2 is applicable */

#ifndef _device_sure_h_
#define _device_suer_h_

#include <string>
using std::string;

#include "device.h"

class DeviceSure : public Device
{
	private:

		int		_fd;
		int		_flashcycle;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

		void	_initserial() throw(string);
		void	_init() throw(string);
		void	_setbright(int value) throw(string);

		size_t	_pollread(size_t size, char * buffer) const throw(string);
		size_t	_pollwrite(size_t size, const char * buffer) const throw(string);

	protected:

		int		__read_analog(int) throw(string);

	public:

				DeviceSure(string device) throw(string);
				~DeviceSure();

		int		width()			const;
		int		height()		const;
		string	identify()		const throw();
		int		analog_inputs()	const throw();
		int		max_analog()	const throw();
};

#endif
