#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>

#include <sstream>
using std::stringstream;

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
using boost::bad_lexical_cast;

#include "http_server.h"
#include "syslog.h"

const char * HttpServer::id_cookie_name = "msgd.display.id";

int HttpServer::page_dispatcher_root(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	string					data, text, id;
	HttpServer::KeyValues	cookies;
	string					table1, table2, td1;
	bool					hastemp;
	string					temp;
	bool					canbeep;
	int						analog_inputs;
	int						max_analog;
	string					analog_values;

	table1	=	"<table style=\"margin: 4px 0px 0px 0px;\">";
	table2	=	"<table style=\"width: 100%; border: solid 1px black;\">";
	td1		=	"<td style=\"width: 10%;\">";

	if(method != "GET")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	if(con_cls->callback_count != 0)
		return(MHD_NO);

	cookies = get_http_values(connection, MHD_COOKIE_KIND);

	if(cookies.data.find(id_cookie_name) == cookies.data.end())
		id = "";
	else
		id = cookies.data[id_cookie_name];

	hastemp = false;
	device->lock();

	try
	{	
		stringstream ss;
		hastemp = device->hastemperature();
		ss << device->temperature();
		temp = ss.str();
	}
	catch(...)
	{
		hastemp = false;
	}

	try
	{	
		canbeep = device->canbeep();
	}
	catch(...)
	{
		canbeep = false;
	}

	try
	{	
		int input;

		analog_inputs	= device->analog_inputs();
		max_analog		= device->max_analog();

		analog_values = string("[") + lexical_cast<string>(analog_inputs) + "/" + lexical_cast<string>(max_analog) + "] ";

		for(input = 0; input < analog_inputs; input++)
			analog_values += lexical_cast<string>(device->read_analog(input)) + " ";
	}
	catch(...)
	{
		analog_inputs	= 0;
		max_analog		= 0;
	}

	device->unlock();

	text = dev_text_to_html(id, "200pt", "64pt");

	data = text + "\n";
	data += table1;
	data += "<tr><td><form method=\"post\" action=\"/display\">" + table2 + "\n<tr><td>&nbsp;</td>" + td1 + "<input type=\"submit\" value=\"display\"/></td></tr></table></form></td></tr>\n";
	data += "<tr><td><form method=\"post\" action=\"/debug\">" + table2 + "\n<tr><td>&nbsp;</td>" + td1 + "<input type=\"submit\" value=\"debug\"/></td></tr></table></form></td></tr>\n";
	data += "<tr><td><form method=\"post\" action=\"/insert\">\n";
	data += 	table2 + "<tr><td>id</td><td>value</td><td>expiry</td><td>freeze</td><td></td></tr><tr valign=\"top\">\n";
	data += 	"<td><textarea name=\"id\" cols=\"12\" rows=\"1\"></textarea></td>\n";
	data += 	"<td><textarea name=\"value\" cols=\"20\" rows=\"3\"></textarea></td>\n";
	data +=		"<td>\n";
	data += 		"<input type=\"radio\" name=\"expiry\" value=\"-1\"/>off<br/>\n";
	data += 		"<input type=\"radio\" name=\"expiry\" value=\"60\" checked=\"checked\"/>00:01:00<br/>\n";
	data += 		"<input type=\"radio\" name=\"expiry\" value=\"300\"/>00:05:00<br/>\n";
	data += 		"<input type=\"radio\" name=\"expiry\" value=\"1800\"/>00:30:00<br/>\n";
	data += 		"<input type=\"radio\" name=\"expiry\" value=\"3600\"/>00:60:00<br/>\n";
	data += 		"<input type=\"radio\" name=\"expiry\" value=\"86400\"/>24:00:0<br/>\n";
	data +=		"</td>\n";
	data +=		"<td>\n";
	data += 		"<input type=\"radio\" name=\"freeze\" value=\"off\" checked=\"checked\"/>off<br/>\n";
	data += 		"<input type=\"radio\" name=\"freeze\" value=\"on\"/>on<br/>\n";
	data +=		"</td>\n";
	data += 	"<td><input type=\"submit\" value=\"insert\"/></td></tr></table></form>\n";
	data += "</td></tr>\n";
	data += "<tr><td><form method=\"post\" action=\"/remove\">" + table2 + "<tr><td><select name=\"id\">" + text_entries_to_options() + "</select></td>" + td1 + "<input type=\"submit\" value=\"remove\"/></td></tr></table></form></td></tr>\n";
	data += "<tr><td><form method=\"post\" action=\"/standout\">" + table2 + "<tr><td><select name=\"id\">" + text_entries_to_options() + "</select></td><td><input type=\"radio\" name=\"value\" value=\"off\" checked=\"checked\"/>off<input type=\"radio\" name=\"value\" value=\"on\"/>on</td>" + td1 + "<input type=\"submit\" value=\"set standout\"/></td></tr></table></form></td></tr>\n";
	data += "<tr><td><form method=\"post\" action=\"/brightness\">" + table2 + "<tr><td><input type=\"radio\" name=\"value\" value=\"0\"/>off<input type=\"radio\" name=\"value\" value=\"1\"/>very dim<input type=\"radio\" name=\"value\" value=\"2\"/>dim<input type=\"radio\" name=\"value\" value=\"3\" checked=\"checked\"/>normal<input type=\"radio\" name=\"value\" value=\"4\"/>bright</td>" + td1 + "<input type=\"submit\" value=\"set brightness\"/></td></tr></table></form></td></tr>\n";

	if(hastemp)
		data += "<tr><td><form method=\"post\" action=\"/temperature\">" + table2 + "<tr><td>" + temp + "</td>" + td1 + "<input type=\"submit\" value=\"get temperature\"/></td></tr></table></form></td></tr>\n";

	if(canbeep)
		data += "<tr><td><form method=\"post\" action=\"/beep\">" + table2 + "<tr><td><input type=\"radio\" name=\"pitch\" value=\"0\"/>0%<input type=\"radio\" name=\"pitch\" value=\"1\"/>25%<input type=\"radio\" name=\"pitch\" value=\"2\"/>50%<input type=\"radio\" name=\"pitch\" value=\"3\" checked=\"checked\"/>75%<input type=\"radio\" name=\"pitch\" value=\"4\"/>100%</td>" + td1 + "<input type=\"submit\" value=\"beep\"/></td></tr></table></form></td></tr>\n";

	if(analog_inputs)
	{
		data += "<tr>";
		data += 	"<td>";
		data += 		"<form method=\"post\" action=\"/read_analog\">";
		data +=     		table2;
		data +=         		"<tr>";
		data +=						"<td>" + analog_values + "</td>";
		data += 					td1 + "<input type=\"text\" name=\"input\"</input></td>\n";
		data += 					td1 + "<input type=\"submit\" value=\"show analog input\"/></td>";
		data +=					"</tr>";
		data +=				"</table>";
		data +=			"</form>\n";
		data += 	"</td>\n";
		data += "</tr>\n";
	}

	data += "</table>\n";

	return(send_html(connection, "/", MHD_HTTP_OK, data, 30, "", id_cookie_name, id));
}

int HttpServer::page_dispatcher_debug(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	string data, text;
	bool last = false;
	string id = "";
	size_t ix;

	HttpServer::KeyValues	responses;
	HttpServer::KeyValues	headers;
	HttpServer::KeyValues	cookies;
	HttpServer::KeyValues	postdata;
	HttpServer::KeyValues	arguments;
	HttpServer::KeyValues	footer;

	TextEntry te;
    TextEntryAttributes::iterator it;

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	responses	= get_http_values(connection, MHD_RESPONSE_HEADER_KIND);
	headers		= get_http_values(connection, MHD_HEADER_KIND);
	cookies		= get_http_values(connection, MHD_COOKIE_KIND);
	postdata	= get_http_values(connection, MHD_POSTDATA_KIND);
	arguments	= get_http_values(connection, MHD_GET_ARGUMENT_KIND);
	footer		= get_http_values(connection, MHD_FOOTER_KIND);
	
	data += string("<p>method: ") + method + "</p>";
	data +=	"<p>responses";
	data += responses.dump(true);
	data +=	"</p>\n<p>headers";
	data += headers.dump(true);
	data +=	"</p>\n<p>cookies";
	data += cookies.dump(true);
	data +=	"</p>\n<p>postdata arguments";
	data += postdata.dump(true);
	data +=	"</p>\n<p>http footer";
	data += footer.dump(true);
	data +=	"</p>\n<p>GET arguments";
	data += arguments.dump(true);
	data += "</p>\n<p>post args";
	data +=	con_cls->values.dump(true);

	stringstream freeze_ss;
	freeze_ss << text_entries.get_freeze_timeout();

	data += "</p>\n<p>frozen: ";
	data += string(text_entries.frozen() ? "yes" : "no");
	data += ", id: " + text_entries.get_freeze_id();
	data += ", timeout: " + freeze_ss.str();
	data += "</p>\n";

	data += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"1\">\n";
	data += "<tr><th>key</th><th>id</th><th>attributes</th><th>expire</th><th>text</th></tr>\n";

	while(!last)
    {
		try
		{
			te = text_entries.get_next(id, last);
			id = te.id;
		}
		catch(string e)
		{
			break;
		}

		data += "<tr>\n";
		data += "<td>" + id + "</td>";
		data += "<td>" + te.id + "</td>";

		data += "<td>";

        for(it = te.attributes.begin(); it != te.attributes.end(); it++)
			data += *it + " ";
		data += "</td>";

		if(te.expire <= 0)
			data += "<td>none</td>";
		else
		{
			stringstream expire_ss;
			expire_ss << te.expire - time(0);
			data += "<td>" + expire_ss.str() + "</td>";
		}

		text = te.text;

		while((ix = text.find("\n")) != string::npos)
			text.replace(ix, 1, "<br/>");

		data += "<td>" + text + "</td>";
		data += "</tr>\n";
    }

	data += "</table>\n";

	string temp;
	bool hastemp = false;

	try
	{	
		stringstream ss;
		device->lock();
		hastemp = device->hastemperature();
		ss << device->temperature();
		temp = ss.str();
		device->unlock();
	}
	catch(string e)
	{
		device->unlock();
		temp = string("error: ") + e;
	}

	if(hastemp)
		data += string("<p>temperature = \"") + temp + "\"</p>\n";

	string ident;

	try
	{	
		device->lock();
		ident = device->identify();
		device->unlock();
	}
	catch(string e)
	{
		device->unlock();
		temp = string("error: ") + e;
	}

	data += string("<p>identification = \"") + ident + "\"</p>\n";

	return(send_html(connection, "debug", MHD_HTTP_OK, data, 5));
};

int HttpServer::page_dispatcher_display(MHD_Connection * connection, const string & method, ConnectionData *) const
{
	string							id;
	HttpServer::KeyValues			cookies;
    TextEntryAttributes::iterator	it;

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	cookies = get_http_values(connection, MHD_COOKIE_KIND);

	if(cookies.data.find(id_cookie_name) == cookies.data.end())
		id = "";
	else
		id = cookies.data[id_cookie_name];

	return(send_html(connection, "display", MHD_HTTP_OK, dev_text_to_html(id, "1000pt", "160pt", "monospace", "500%"), 5, "", id_cookie_name, id));
};

int HttpServer::page_dispatcher_remove(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	string	id;
	string	data;

	if(method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	if(con_cls->values.data.find("id") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: id"));

	id = con_cls->values.data["id"];

	try
	{
		text_entries.erase(id);
		data = "<p>entry " + id + " removed</p>\n";
	}
	catch(string e)
	{
		data = "<p>entry " + id + " not removed (" + e + ")</p>\n";
	}

	return(send_html(connection, "remove", MHD_HTTP_OK, data, 1, "/"));
};

int HttpServer::page_dispatcher_standout(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	string		id;
	string		value;
	string		data;
	TextEntry	te;

	if(method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	if(con_cls->values.data.find("id") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: id"));

	if(con_cls->values.data.find("value") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: value"));

	id = con_cls->values.data["id"];
	value = con_cls->values.data["value"];

	try
	{
		te = text_entries.get(id);
	}
	catch(string e)
	{
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, e));
	}

	data = "<p>entry " + id + " ";

	if(value == "on")
	{
		te.attributes.insert("standout");
		data += "added";
	}
	else
	{
		if(value == "off")
		{
			te.attributes.erase("standout");
			data += "removed";
		}
		else
			data += "ignored (unknown)";
	}


	data += " attribute</p>\n";

	text_entries.put(te);

	return(send_html(connection, "standout", MHD_HTTP_OK, data, 5, "/"));
};

int HttpServer::page_dispatcher_insert(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	string		id;
	string		value;
	string		data;
	string		expiry;
	string		freeze;

	if(method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	if(con_cls->values.data.find("id") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: id"));
	else
		id = con_cls->values.data["id"];

	if(con_cls->values.data.find("value") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: value"));
	else
		value = con_cls->values.data["value"];

	if(con_cls->values.data.find("expiry") == con_cls->values.data.end())
		expiry = "60";
	else
		expiry = con_cls->values.data["expiry"];

	if(con_cls->values.data.find("freeze") == con_cls->values.data.end())
		freeze = "off";
	else
		freeze = con_cls->values.data["freeze"];

	TextEntry te(id, value, strtol(expiry.c_str(), 0, 10));

	try
	{
		text_entries.put(te);

		if(freeze == "on")
			text_entries.freeze(id);
	}
	catch(string e)
	{
		return(http_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, e));
	}

	data = "<p>insert id=\"" + id + "\", value=\"" + value + "\", expiry=\"" + expiry + "\", freeze=\"" + freeze + "\" OK</p>\n";

	return(send_html(connection, "insert", MHD_HTTP_OK, data, 5, "/"));
};

int HttpServer::page_dispatcher_temperature(MHD_Connection * connection, const string & method, ConnectionData *) const
{
	if((method != "POST") && (method != "GET"))
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	string temperature;

	try
	{	
		stringstream ss;
		device->lock();
		ss << device->temperature();
		temperature = ss.str();
		device->unlock();
	}
	catch(string e)
	{
		device->unlock();
		temperature = string("error: ") + e;
	}

	string data = string("<p>temperature = \"") + temperature + "\"</p>\n";

	return(send_html(connection, "temperature", MHD_HTTP_OK, data, 5, "/"));
}

int HttpServer::page_dispatcher_brightness(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	if((method != "POST") && (method != "GET"))
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	string value;

	if(con_cls->values.data.find("value") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: value"));
	else
		value = con_cls->values.data["value"];

	string result;

	try
	{	
		device->lock();
		device->brightness(strtol(value.c_str(), 0, 10));
		result = "OK";
		device->unlock();
	}
	catch(string e)
	{
		device->unlock();
		result = string("Error: ") + e;
	}

	string data = string("<p>set brightness = \"") + result + "\"</p>\n";

	return(send_html(connection, "brightness", MHD_HTTP_OK, data, 5, "/"));
}

int HttpServer::page_dispatcher_beep(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	if(method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	string pitch;

	if(con_cls->values.data.find("pitch") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: pitch"));
	else
		pitch = con_cls->values.data["pitch"];

	string result;

	try
	{	
		device->lock();
		device->beep(lexical_cast<int>(pitch));
		result = "OK";
		device->unlock();
	}
	catch(string e)
	{
		device->unlock();
		result = string("Error: ") + e;
	}
	catch(bad_lexical_cast)
	{
		device->unlock();
		result = string("Error: invalid value");
	}

	string data = string("<p>beep = \"") + result + "\"</p>\n";

	return(send_html(connection, "beep", MHD_HTTP_OK, data, 5, "/"));
}

int HttpServer::page_dispatcher_read_analog(MHD_Connection * connection, const string & method, ConnectionData * con_cls) const
{
	if(method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	string input, value, error;

	if(con_cls->values.data.find("input") == con_cls->values.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "Missing POST-value: input"));
	else
		input = con_cls->values.data["input"];

	try
	{	
		device->lock();
		value = lexical_cast<string>(device->read_analog(lexical_cast<int>(input)));
		device->unlock();
		error = string("OK");
	}
	catch(string e)
	{
		device->unlock();
		value = string("ERROR");
		error = e;
	}
	catch(bad_lexical_cast)
	{
		device->unlock();
		value = string("ERROR");
		error = string("bad input");
	}

	string data = string("<p>[") + value + "]: " + error + "</p>\n";

	return(send_html(connection, "read_analog", MHD_HTTP_OK, data, 5, "/"));
}

