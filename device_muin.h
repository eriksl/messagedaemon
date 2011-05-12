#ifndef _device_muin_h_
#define _device_muin_h_

#include <string>
using std::string;

#include "device_matrix_common.h"

class DeviceMuin : public DeviceMatrixCommon
{
	protected:

		void	__init() throw(string);
		void	__setbright(int) throw(string);
		void	__beep(int) throw(string);
		int		__read_analog(int) throw(string);

	public:

				DeviceMuin(string) throw();
		virtual	~DeviceMuin() throw();
		string	identify() const throw();
		bool	canbeep() const throw();
		int		analog_inputs() const throw();
		int		max_analog() const throw();
};

#endif
