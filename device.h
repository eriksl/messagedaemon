/* Copyright Erik Slagter, GPLv2 is applicable */

#ifndef _device_h_
#define _device_h_

#include <pthread.h>

#include <string>
using std::string;

#include <deque>
using std::deque;

class Device
{
	public:

		typedef struct
		{
			int key;

			enum
			{
				key_pressed,
				key_released,
				key_undef
			}
			status;

		} keyevent_t;

	private:

		typedef				deque<keyevent_t> keyevents_t;
		keyevents_t			keyevents;

		bool				_clean;
		bool				_opened;
		pthread_mutex_t		_mutex;
		bool				_mutex_valid;

		virtual void		__update()					throw(string)	= 0;
		virtual void		__open()					throw(string)	= 0;
		virtual	void		__close()									= 0;
		virtual void		__beep(int)					throw(string);
		virtual int			__read_analog(int)			throw(string);

		void				_printline(string)			throw(string);

	protected:

		char *			_textbuffer;
		size_t			_textbuffer_size;
		int				_x;
		int				_y;
		bool			_standout;
		int				_brightness;
		int				_progress;
		string			_device_node;

		void			_addkey(int key, bool pressed);	
		void			_move(int y, int x);
		void			_clear();

	public:

						Device() throw(string);
		virtual			~Device();

		void			lock() throw(string);
		void			unlock() throw(string);

		int				x() const;
		int				y() const;

		void			open() throw(string);
		void			close();
		void			move(int y = -1, int x = -1);
		void			print(string text) throw(string);
		void			clear();
		void			update() throw(string);
		keyevent_t		getkey(bool & ok);

		virtual void	standout(bool);
		virtual void	brightness(int) throw(string);
		virtual void	progress(int) throw(string);
		virtual void	poll() throw(string);
		virtual int		width()	const = 0;
		virtual int		height() const = 0;
		virtual string	identify() const throw();
		virtual bool	canbeep() const throw();
		virtual void	beep(int) throw(string);
		virtual int		analog_inputs() const throw();
		virtual int		read_analog(int input) throw(string);
		virtual int		max_analog() const throw(string);
};

#endif
