/* Copyright Erik Slagter, GPLv2 is applicable */

#include <string.h>

#include <iomanip>
using std::right;
using std::left;
using std::setw;

#include "device.h"
#include "device_gtk.h"
#include "syslog.h"

DeviceGtk::DeviceGtk(string) throw(string)
{
	gtk_init(0, 0);
	_window = 0;
	_label  = 0;
}

DeviceGtk::~DeviceGtk()
{
	__close();
}

void DeviceGtk::__open() throw(string)
{
	GdkColor colour;

	if(_window)
		throw(string("DeviceGtk::__open: device not closed"));

	_ready			= false;
	_current_width	= 800;
	_current_height	= 330;

	_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(_window), "msgd");
	colour.red = colour.green = colour.blue = 65535;
	gtk_widget_modify_bg(GTK_WIDGET(_window), GTK_STATE_NORMAL, &colour);
	gtk_widget_show(_window);

	_label = gtk_label_new(0);
	gtk_container_add(GTK_CONTAINER(_window), _label);
	gtk_misc_set_alignment(GTK_MISC(_label), 0.5, 0.5);
	gtk_widget_show(_label);

	_ready = true;

	g_signal_connect(G_OBJECT(_window),
			"configure_event",
			G_CALLBACK(_resize_callback),
			this);

	gtk_widget_set_size_request(_label, _current_width, _current_height);
	gtk_window_maximize(GTK_WINDOW(_window));
}

void DeviceGtk::__close()
{
	if(_label)
	{
		gtk_widget_destroy(_label);
		_label = 0;
	}

	if(_window)
	{
		gtk_widget_destroy(_window);
		_window = 0;
	}
}

int DeviceGtk::width() const
{
	return(20);
}

int DeviceGtk::height() const
{
	return(4);
}

void DeviceGtk::__update() throw(string)
{
	int			yy;
	char		line[(width() + 1) * height() + 1];
	char *		line2;
	int			font_size;
	GdkColor	colour;

	if(!_window)
		throw(string("DeviceGtk::__update: device not open"));

	*line = '\0';

	for(yy = 0; yy < height(); yy++)
	{
		strncat(line, _textbuffer + (yy * width()), width());
		strcat(line, "\n");
	}

	font_size = _current_width / 15;

	colour.red = colour.green = colour.blue = 65535;

	if(_standout)
		colour.blue = 40000;

	gtk_widget_modify_bg(GTK_WIDGET(_window), GTK_STATE_NORMAL, &colour);

	line2 = g_markup_printf_escaped("<span font=\"Sans %d\">%s</span>", font_size, line);
	gtk_label_set_markup(GTK_LABEL(_label), line2);
	g_free(line2);

	while(g_main_context_iteration(0, FALSE));
}

void DeviceGtk::_resize_callback(GtkWidget *, GdkEventConfigure * event, DeviceGtk * thisptr)
{
	thisptr->resize(event->height, event->width);
}

void DeviceGtk::resize(int h, int w) throw(string)
{
	if(_ready)
	{
		vlog("resize event: %dx%d\n", w, h);
		_current_width = w;
		_current_height = h;
		_ready = false;
		gtk_widget_set_size_request(_label, w, h);
		_ready = true;
		__update();
	}
}
