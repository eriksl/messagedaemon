#ifndef __http_server_h__
#define __http_server_h__

#include <sys/types.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <time.h>
#include <microhttpd.h>
#include <pthread.h>

#include <string>
using std::string;

#include <map>
using std::map;

#include "textentry.h"
#include "device.h"

class HttpServer
{
	private:

		struct MHD_Daemon * daemon;
		Device *			device;
		TextEntries &		text_entries;
		bool				multithread;
		static const char *	id_cookie_name;

		typedef map<string, string> string_string_map;

		struct KeyValues
		{
			string_string_map	data;
			string				dump(bool html) const;
		};

		struct ConnectionData
		{
			int							callback_count;
			struct MHD_PostProcessor *	postprocessor;
			KeyValues					values;
		};

		struct PageHandler
		{
			typedef int (HttpServer::*dispatcher_function_t)(MHD_Connection *, const string & method,
							ConnectionData * con_cls) const;
			typedef map<string, dispatcher_function_t> map_t;

			map_t data;
		};

		PageHandler::map_t	page_dispatcher_map;

		static string		html_header(const string & title = "", int reload = 0, string reload_url = "");
		static string		html_footer();

		int					send_html(MHD_Connection * connection, const string & title, int http_code,
									const string & data, int reload = 0, const string & reload_url = "",
									const string & cookie_id = "", const string & cookie_value = "") const throw(string);
		int					http_error(MHD_Connection * connection, int code,
									const string & message) const throw(string);
		string				dev_text_to_html(string & id, const string & width = "", const string & height = "",
									string font_face = "monospace", string font_size = "100%") const throw(string);
		string				text_entries_to_options() const throw(string);

		static int 			callback_keyvalue_iterator(void * cls, enum MHD_ValueKind kind, const char * key, const char * value);
		KeyValues			get_http_values(struct MHD_Connection * connection, enum MHD_ValueKind kind) const;

		static int			access_handler_callback(void * object,
								struct MHD_Connection * connection,
								const char * url, const char * method, const char * version,
								const char * upload_data, size_t * upload_data_size,
								void ** con_cls);

		int					access_handler(struct MHD_Connection * connection,
								const string & url, const string & method, const string & version,
								ConnectionData * con_cls, size_t * upload_data_size, const char * upload_data) const;

		static int			callback_postdata_iterator(void * cls, enum MHD_ValueKind kind,
								const char * key, const char * filename, const char * content_type,
								const char * transfer_encoding, const char * data, uint64_t off, size_t size);

		static void *		callback_request_completed(void * cls, struct MHD_Connection * connection,
								void ** con_cls, enum MHD_RequestTerminationCode toe);

		int page_dispatcher_root		(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_debug		(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_display		(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_remove		(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_standout	(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_insert		(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_brightness	(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_temperature	(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_beep		(MHD_Connection *, const string & method, ConnectionData * con_cls) const;
		int page_dispatcher_read_analog	(MHD_Connection *, const string & method, ConnectionData * con_cls) const;

	public:

		HttpServer(TextEntries &, Device *, int tcp_port = 8888, bool multithread = true) throw(string);
		~HttpServer() throw(string);
		void poll(int timeout) throw(string);
};

#endif
