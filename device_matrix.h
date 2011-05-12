#ifndef _device_matrix_h_
#define _device_matrix_h_

#include <string>
using std::string;

#include "device_matrix_common.h"

class DeviceMatrix : public DeviceMatrixCommon
{
	protected:
		
		void	__init() throw(string);
		void	__reinit() throw(string);
		void	__setbright(int) throw(string);

	public:

				DeviceMatrix(string) throw();
		virtual	~DeviceMatrix() throw();

		string	identify() const throw();
};

#endif
