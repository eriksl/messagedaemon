#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <string>
using std::string;

#include "device.h"
#include "http_server.h"
#include "syslog.h"

#if defined(DEVICE_TTY)
#include "device_tty.h"
#endif

#if defined(DEVICE_CURSES)
#include "device_curses.h"
#endif

#if defined(DEVICE_GTK)
#include "device_gtk.h"
#endif

#if defined(DEVICE_MATRIX)
#include "device_matrix.h"
#endif

#if defined(DEVICE_MUIN)
#include "device_muin.h"
#endif

#if defined(DEVICE_CF634)
#include "device_cf634.h"
#endif

#if defined(DEVICE_CF635)
#include "device_cf635.h"
#endif

#if defined(DEVICE_SURE)
#include "device_sure.h"
#endif

#if defined(DEVICE_LCD)
#include "device_lcd.h"
#endif

#include "textentry.h"

static bool quit = false;

static void sigint(int)
{
	signal(SIGINT, SIG_DFL);
	quit = true;
}

static void sigterm(int)
{
	signal(SIGTERM, SIG_DFL);
	quit = true;
}

static void msgd_internal_throw(const string & error)
{
	string errormessage = string("error caught, message = ") + error;

	if(errno != 0)
	{
		errormessage += string(", system error = ");
		errormessage += strerror(errno);
	}

	vlog("%s\n", errormessage.c_str());
}

static Device * new_device(string devicetype, string devicenode, __attribute__((unused)) bool & foreground)
{
	Device * dev = 0;

#ifdef DEVICE_TTY
	if(devicetype == "tty")
	{
		foreground = true;
		dev = new DeviceTty();
	}
#endif
#ifdef DEVICE_CURSES
	if(devicetype == "curses")
	{
		foreground = true;
		dev = new DeviceCurses();
	}
#endif
#ifdef DEVICE_GTK
	if(devicetype == "gtk")
		dev = new DeviceGtk();
#endif
#ifdef DEVICE_MATRIX
	if(devicetype == "matrix")
		dev = new DeviceMatrix(devicenode);
#endif
#ifdef DEVICE_MUIN
	if(devicetype == "muin")
		dev = new DeviceMuin(devicenode);
#endif
#ifdef DEVICE_CF634
	if(devicetype == "cf634")
		dev = new DeviceCF634(devicenode);
#endif
#ifdef DEVICE_CF635
	if(devicetype == "cf635")
		dev = new DeviceCF635(devicenode);
#endif
#ifdef DEVICE_SURE
	if(devicetype == "sure")
		dev = new DeviceSure(devicenode);
#endif
#ifdef DEVICE_LCD
	if(devicetype == "lcd")
		dev = new DeviceLcd(devicenode);
#endif
	return(dev);
}

int main(int argc, char ** argv)
{
	int					opt;
	bool				foreground = false;
	string				keyscript;
	Device *			device = (Device *)0;

	try
	{
		string	devices;
		string	devicetype;
		string	devicenode = "";
		string	deviceid;

#if defined(DEVICE_TTY)
	devices += " tty";
#endif

#if defined(DEVICE_CURSES)
	devices += " curses";
#endif

#if defined(DEVICE_GTK)
	devices += " gtk";
#endif

#if defined(DEVICE_MATRIX)
	devices += " matrix";
#endif

#if defined(DEVICE_MUIN)
	devices += " muin";
#endif

#if defined(DEVICE_CF634)
	devices += " cf634";
#endif

#if defined(DEVICE_CF635)
	devices += " cf635";
#endif

#if defined(DEVICE_SURE)
	devices += " sure";
#endif

#if defined(DEVICE_LCD)
	devices += " lcd";
#endif

		while((opt = getopt(argc, argv, "fd:D:k:")) != -1)
		{
			switch(opt)
			{
				case('f'):
				{
					foreground = true;
					break;
				}

				case('d'):
				{
					devicetype = optarg;
					break;
				}

				case('D'):
				{
					devicenode = optarg;
					break;
				}

				case('k'):
				{
					keyscript = string(optarg);
					break;
				}

				default:
				{
					errno = 0;
					throw(string("usage: msgd -f -d type [-D device] [-k keyscript], type =") + devices + string(", device = /dev/*, -f = foreground"));
				}
			}
		}

		if(devicetype == "help" || devicetype == "")
		{
			fprintf(stderr, "available devices:%s\n", devices.c_str());
			exit(-1);
		}

		device = new_device(devicetype, devicenode, foreground);

		if(!device)
		{
			fprintf(stderr, "unknown device, available devices:%s\n", devices.c_str());
			exit(-1);
		}

		device->open();

		deviceid = device->identify();

		signal(SIGINT, sigint);
		signal(SIGTERM, sigterm);

		if(!foreground)
			daemon(0, 0);

		setresuid(65534, 65534, 65534);

		TextEntries text_entries;

#ifdef MHD_mode_multithread
		HttpServer * http_server = new HttpServer(text_entries, device, 8888, true);
#else
#ifdef MHD_mode_singlethread
		HttpServer * http_server = new HttpServer(text_entries, device, 8888, false);
#else
#error "Either MHD_mode_singlethread or MHD_mode_multithread should be set"
#endif
#endif
		TextEntry current;
		string current_id = "";

		while(!quit)
		{
			try
			{
next_id:
				if(text_entries.size() == 0)
				{
					char datebuf[32];
					time_t ticks;
					ticks = time(0);
					struct tm tm;
					localtime_r(&ticks, &tm);
					strftime(datebuf, sizeof(datebuf), "%Y %b %d %H:%M:%S\n", &tm);
					text_entries.put(TextEntry("boot", string(datebuf) + "\n" + deviceid));
					current_id = "boot";
				}

				try
				{
					if(text_entries.frozen())
					{
						current_id	= text_entries.get_freeze_id();
						current		= text_entries.get(current_id);
					}
					else
					{
						bool last;
						current = text_entries.get_next(current_id, last);
						current_id = current.id;
					}
				}
				catch(string e)
				{
					current_id = "";
					continue;
				}

				if((current.expire > 0) && (current.expire < time(0)))
				{
					try
					{
						bool last;
						current = text_entries.get_next(current_id, last);
						text_entries.erase(current_id);
						current_id = current.id;
					}
					catch(string e)
					{
						current_id = "";
					}

					continue;
				}

				int		progress;
				string	output;
				char	buffer[128];

				snprintf(buffer, sizeof(buffer), "%-14.14s", current.id.c_str());
				output = buffer;

				device->lock();
				device->clear();

				if((current.attributes.find("standout") != current.attributes.end()) || text_entries.frozen())
					device->standout(true);
				else
					device->standout(false);

				device->move(0, 6);
				device->print(output);
				device->move(1, 0);
				device->print(current.text);
				device->unlock();

				for(progress = 0; !quit && (progress < 5); progress++)
				{
					time_t ticks;
					struct tm tm;

					time(&ticks);
					localtime_r(&ticks, &tm);
					char datebuf[32];

					switch(progress)
					{
						case(0):
						case(3):
						{
							strftime(datebuf, sizeof(datebuf), "%H:%M", &tm);
							break;
						}

						case(1):
						case(4):
						{
							strftime(datebuf, sizeof(datebuf), "%d/%m", &tm);
							break;
						}

						case(2):
						{
							snprintf(datebuf, sizeof(datebuf), "[%d]   ", (int)text_entries.size());
							break;
						}
					}

					datebuf[5] = 0;
					output = datebuf;

					device->lock();
					device->progress(100 - (progress * 25));
					device->move(0, 0);
					device->print(output);
					device->update();
					device->unlock();

					time(&ticks);

					while(!quit && (time(0) - ticks) < 2)
					{
						if(text_entries.frozen() && (current_id != text_entries.get_freeze_id()))
							goto next_id;

						device->lock();
						device->poll();
						device->unlock();

						if(keyscript != "")
						{
							Device::keyevent_t	key;
							string	cmdline;
							char	val2string[8];
							bool	ok;

							for(;;)
							{
								device->lock();
								key = device->getkey(ok);
								device->unlock();

								if(!ok)
									break;

								snprintf(val2string, sizeof(val2string), "%d", key.key);

								vlog("readkey: %s,%d\n", val2string, key.status);

								cmdline = keyscript + " " + val2string + " " + ((key.status == Device::keyevent_t::key_pressed) ? "PRESSED" : "RELEASED");

								system(cmdline.c_str());
							}
						}

						http_server->poll(200000);
					}
				}
			}
			catch(string e)
			{
				int retry;

				for(retry = 0; !quit; retry++)
				{
					if(retry != 0)
						sleep(1);

					vlog("device exception: %s, retry %d\n", e.c_str(), retry);

					try
					{
						device->close();
						device->open();
						device->unlock();
						vlog("device OK, resuming\n");
						break;
					}
					catch(string e2)
					{
						continue;
					}
				}

				if(quit)
					break;

				continue;
			}
		}

		delete(http_server);
		http_server = 0;
		delete(device);
		device = 0;
	}
	catch(const string & error)
	{
		msgd_internal_throw(error);
		exit(-1);
	}
	catch(const char * error)
	{
		msgd_internal_throw(string(error));
		exit(-1);
	}
	catch(...)
	{
		msgd_internal_throw(string("caught unknown error"));
		exit(-1);
	}

	if(quit)
		vlog("interrupt\n");

	exit(0);
}
