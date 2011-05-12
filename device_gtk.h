#ifndef _device_gtk_h_
#define _device_gtk_h_

#include <string>
using std::string;

#include "device.h"

#include <gtk/gtk.h>
#include <pthread.h>

class DeviceGtk : public Device
{
	private:

		GtkWidget * _window;
		GtkWidget * _label;

		void	__update() throw(string);
		void	__open() throw(string);
		void	__close();

		static void _resize_callback(GtkWidget * widget,
					GdkEventConfigure * event,
					DeviceGtk * thisptr);

		int _current_width;
		int _current_height;

		bool _ready;

	public:

		DeviceGtk() throw(string);
		~DeviceGtk();

		int		width() const;
		int		height() const;
		void	resize(int height, int width) throw(string);
};

#endif
