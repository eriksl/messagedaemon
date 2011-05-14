/* Copyright Erik Slagter, GPLv2 is applicable */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
using boost::bad_lexical_cast;

#include "http_server.h"
#include "syslog.h"

HttpServer::HttpServer(TextEntries & te_in, Device * dev, int tcp_port, bool multithread_in) throw(string)
	: device(dev), text_entries(te_in), multithread(multithread_in)
{
	int multithread_option = multithread ? MHD_USE_THREAD_PER_CONNECTION : 0;

	page_dispatcher_map["/"]				=  &HttpServer::page_dispatcher_root;
	page_dispatcher_map["/debug"]			=  &HttpServer::page_dispatcher_debug;
	page_dispatcher_map["/display"]			=  &HttpServer::page_dispatcher_display;
	page_dispatcher_map["/remove"]			=  &HttpServer::page_dispatcher_remove;
	page_dispatcher_map["/standout"]		=  &HttpServer::page_dispatcher_standout;
	page_dispatcher_map["/insert"]			=  &HttpServer::page_dispatcher_insert;
	page_dispatcher_map["/brightness"]		=  &HttpServer::page_dispatcher_brightness;
	page_dispatcher_map["/beep"]			=  &HttpServer::page_dispatcher_beep;
	page_dispatcher_map["/read_analog"]		=  &HttpServer::page_dispatcher_read_analog;

	daemon = MHD_start_daemon(multithread_option | MHD_USE_IPv6 | MHD_USE_DEBUG,
			tcp_port, 0, 0, &HttpServer::access_handler_callback, this,
			MHD_OPTION_NOTIFY_COMPLETED, &HttpServer::callback_request_completed, this,
			MHD_OPTION_END);

	if(daemon == 0)
		throw(string("Cannot start http daemon"));
}

HttpServer::~HttpServer() throw(string)
{
	MHD_stop_daemon(daemon);
	daemon = 0;
}

void HttpServer::poll(int timeout) throw(string)
{
	if(multithread)
	{
		if(timeout < 0)
			for(;;)
				sleep(65536);
		else
			usleep(timeout);
	}
	else
	{
		fd_set				read_fd_set, write_fd_set, except_fd_set;
		int					max_fd = 0;
		struct timeval		tv;
		struct timeval *	tvp;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&except_fd_set);

		if(MHD_get_fdset(daemon, &read_fd_set, &write_fd_set, &except_fd_set, &max_fd) == MHD_NO)
			throw(string("error in MHD_get_fdset"));

		if(timeout >= 0)
		{
			tv.tv_sec	= timeout / 1000000;
			tv.tv_usec	= (timeout % 1000000);
			tvp = &tv;
		}
		else
			tvp = 0;

		if(select(max_fd + 1, &read_fd_set, &write_fd_set, &except_fd_set, tvp) != 0)
			MHD_run(daemon);
	}
}

string HttpServer::html_header(const string & title, int reload, string reload_url, string cssurl)
{
	string			refresh_string;
	string			refresh_header;

	if(reload)
	{
		try
		{
			refresh_string = lexical_cast<string>(reload);
		}
		catch(bad_lexical_cast)
		{
			reload = 0;
		}
	}

	if(reload)
	{
		refresh_header = "        <meta http-equiv=\"Refresh\" content=\"" + refresh_string;

		if(reload_url.size() != 0)
			refresh_header += ";url=" + reload_url;

		refresh_header += "\"/>\n";
	}

	if(cssurl != "")
		cssurl = "<link rel=\"stylesheet\" href=\"/style.css\"/>";

	return(string("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n") +
				"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n" +
				"    <head>\n" +
				"        <meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\"/>\n" +
				cssurl +
				refresh_header +
				"        <title>" + title + "</title>\n" + 
				"    </head>\n" +
				"    <body>\n");
}

string HttpServer::html_footer()
{
	return(string("    </body>\n") +
				"</html>\n");
}

int HttpServer::send_raw(MHD_Connection * connection, int http_code,
			const string & data, const string & data_mime,
			const string & cookie_id, const string & cookie_value) const throw(string)
{
	int						rv;
	struct MHD_Response	*	response;
	char *					buffer;

	buffer = (char *)alloca(data.size() + 1);
	strcpy(buffer, data.c_str());

	response = MHD_create_response_from_data(data.size(), buffer, MHD_NO, MHD_YES);
	MHD_add_response_header(response, "Content-Type", data_mime.c_str());

	if(cookie_id.size())
	{
		string cookie = cookie_id + "=" + cookie_value + "; path=/;";
		MHD_add_response_header(response, "Set-Cookie", cookie.c_str());
	}

	rv = MHD_queue_response(connection, http_code, response);
	MHD_destroy_response(response);

	return(rv);
}

int HttpServer::send_html(MHD_Connection * connection, const string & title, int http_code,
			const string & message, int reload, const string & reload_url,
			const string & cookie_id, const string & cookie_value) const throw(string)
{
	string					data;

	data = html_header(title, reload, reload_url, "/style.css");
	data += message;
	data += html_footer();

	return(send_raw(connection, http_code, data, "text/html", cookie_id, cookie_value));
}

int HttpServer::http_error(MHD_Connection * connection, int http_code, const string & message) const throw(string)
{
	return(send_html(connection, "ERROR", http_code, string("<p>") + message + "</p>\n"));
}

string HttpServer::dev_text_to_html(string & id, const string & width, const string & height, string font_face, string font_size) const throw(string)
{
	string							data, text;
	TextEntry						te;
	bool							standout, last;
	size_t							ix;
	TextEntryAttributes::iterator	it;
	string							table_style;
	string							entry_style;

	for(;;)
	{
		try
		{
			te = text_entries.get_next(id, last);
			id = te.id;
		}
		catch(string e)
		{
			sleep(10);
			continue;
		}

		break;
	}

	standout = (te.attributes.find("standout") != te.attributes.end());
	text = te.text;

	table_style = "style=\"border: solid 1px blue; padding: 0px; margin: 0px; background: yellow;";

	if(width.size() != 0)
		table_style += " width: " + width + ";";

	if(height.size() != 0)
		table_style += " height: " + height + ";";

	table_style += "\"";

	entry_style = "style=\"font-family: " + font_face + "; font-size: " + font_size + "; font-weight: " + (standout ? "bold" : "normal") + ";\"";

	while((ix = text.find("\n")) != string::npos)
		text.replace(ix, 1, "</td></tr><tr><td " + entry_style + ">");

	data += "<table " + table_style + ">\n";
	data += "<tr valign=\"top\"><td " + entry_style + ">" + id + "</td></tr>\n";
	data += "<tr valign=\"top\"><td " + entry_style + ">" + text + "</td></tr>\n";
	data += "</table>\n";

	return(data);
}

string HttpServer::text_entries_to_options() const throw(string)
{
	string		data;
	TextEntry	te;
	bool		last;

	try
	{
		te = text_entries.get_next("", last);
	}
	catch(string e)
	{
		return("");
	}

	data += "<option value=\"" + te.id + "\">" + te.id + "</option>\n";

	while(!last)
	{
		try
		{
			te = text_entries.get_next(te.id, last);
		}
		catch(string e)
		{
			break;
		}

		data += "<option value=\"" + te.id + "\">" + te.id + "</option>\n";
	}

	return(data);
}

int HttpServer::access_handler_callback(void * void_http_server,
		struct MHD_Connection * connection,
		const char * url, const char * method, const char * version,
		const char * upload_data, size_t * upload_data_size,
		void ** con_cls)
{
	HttpServer * http_server = (HttpServer *)void_http_server;

	if(*con_cls == 0)
	{
		ConnectionData * ncd = new(ConnectionData);
		ncd->callback_count = 0;
		ncd->postprocessor	= MHD_create_post_processor(connection, 1024, callback_postdata_iterator, ncd);
		*con_cls = (void *)ncd;
	}
	else
		(**(ConnectionData **)con_cls).callback_count++;

	if(string(method) == "POST")
	{
		if((**(ConnectionData **)con_cls).callback_count == 0)
			return(MHD_YES);
	}

	if(*upload_data_size)
	{
		MHD_post_process((**(ConnectionData **)con_cls).postprocessor, upload_data, *upload_data_size);
		*upload_data_size = 0;
		return(MHD_YES);
	}

	return(http_server->access_handler(connection,
		url, method, version, *(ConnectionData **)con_cls, upload_data_size, upload_data));
};

int HttpServer::access_handler(struct MHD_Connection * connection,
		const string & url, const string & method, const string &,
		ConnectionData * con_cls, size_t *, const char *) const
{
	PageHandler::map_t::const_iterator	it;
	PageHandler::dispatcher_function_t	fn;

	for(it = page_dispatcher_map.begin(); it != page_dispatcher_map.end(); it++)
		if(it->first == string(url)) 
			break;

	if(it != page_dispatcher_map.end())
	{
		fn = it->second;

		StringStringMap variables;

		if((method != "GET") && (method != "POST"))
			return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

		if(method == "POST")
			variables = con_cls->values;

		StringStringMap get_arguments = get_http_values(connection, MHD_GET_ARGUMENT_KIND);
		variables.insert(get_arguments.begin(), get_arguments.end());

		return((this->*fn)(connection, method, con_cls, variables));
	}

	return(http_error(connection, MHD_HTTP_NOT_FOUND, string("URI ") + url + " not found"));
}

int HttpServer::callback_keyvalue_iterator(void * cls, enum MHD_ValueKind, const char * key, const char * value)
{
	StringStringMap * rv = (StringStringMap *)cls;

	(*rv)[string(key)] = string(value);

	return(MHD_YES);
}

StringStringMap HttpServer::get_http_values(struct MHD_Connection * connection, enum MHD_ValueKind kind) const
{
	StringStringMap rv;

	MHD_get_connection_values(connection, kind, callback_keyvalue_iterator, &rv);

	return(rv);
}

void * HttpServer::callback_request_completed(void *, struct MHD_Connection *, void ** con_cls, enum MHD_RequestTerminationCode)
{
	if(con_cls && *con_cls)
	{
		ConnectionData * cdp = (ConnectionData *)*con_cls;

		if(cdp->postprocessor)
		{
			MHD_destroy_post_processor(cdp->postprocessor);
			cdp->postprocessor = 0;
		}

		delete(cdp);
		*con_cls = 0;
	}

	return(0);
}

int HttpServer::callback_postdata_iterator(void * con_cls, enum MHD_ValueKind,
		const char * key, const char *, const char *,
		const char *, const char * data, uint64_t, size_t size)
{
	string mangle;
	ConnectionData * condata = (ConnectionData *)con_cls;

	mangle.append(data, size);
	condata->values[key] = mangle;
	return(MHD_YES);
}
