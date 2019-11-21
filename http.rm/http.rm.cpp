// http.rm.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "cpprest\json.h"
#include "cpprest\http_client.h"
#include <iomanip>
#include "xml2json.hpp"

using namespace std;
using namespace utility;
using namespace pplx;
//using namespace web;
using namespace web::details;
using namespace web::http;
using namespace web::http::oauth2;
using namespace web::http::oauth2::experimental;
using namespace web::http::client;

/*! \page errors Introduction
(HTTP) response status codes
-# 1xx Informational
-# 2xx Success
-# 3xx Redirection
-# 4xx Client Error
-# 5xx Server Error
*/
#define	HTTP_CODE_UNKNOWN						0
#define HTTP_CODE_CONTINUE						100
#define HTTP_CODE_PROCESSING					102
#define HTTP_CODE_OK							200
#define HTTP_CODE_CREATED						201
#define HTTP_CODE_SWITCHINGPROTOCOLS			101
#define HTTP_CODE_ACCEPTED						202
#define HTTP_CODE_NONAUTHORITATIVEINFORMATION	203
#define HTTP_CODE_NOCONTENT						204
#define HTTP_CODE_RESETCONTENT					205
#define HTTP_CODE_PARTIALCONTENT				206
#define HTTP_CODE_MULTIPLECHOICES				300
#define HTTP_CODE_MOVEDPERMANENTLY				301
#define HTTP_CODE_FOUND							302
#define HTTP_CODE_SEEOTHER						303
#define HTTP_CODE_NOTMODIFIED					304
#define HTTP_CODE_USEPROXY						305
#define HTTP_CODE_TEMPORARYREDIRECT				307
#define HTTP_CODE_BADREQUEST					400
#define HTTP_CODE_UNAUTHORIZED					401
#define HTTP_CODE_PAYMENTREQUIRED				402
#define HTTP_CODE_FORBIDDEN						403
#define HTTP_CODE_NOTFOUND						404
#define HTTP_CODE_METHODNOTALLOWED				405
#define HTTP_CODE_NOTACCEPTABLE					406
#define HTTP_CODE_PROXYAUTHREQUIRED				407
#define HTTP_CODE_REQUESTTIMEOUT				408
#define HTTP_CODE_CONFLICT						409
#define HTTP_CODE_GONE							410
#define HTTP_CODE_LENGTHREQUIRED				411
#define HTTP_CODE_PRECONDITIONFAILED			412
#define HTTP_CODE_REQUESTENTITYTOOLARGE			413
#define HTTP_CODE_REQUESTURITOOLARGE			414
#define HTTP_CODE_UNSUPPORTEDMEDIATYPE			415
#define HTTP_CODE_RANGENOTSATISFIABLE			416
#define HTTP_CODE_EXPECTATIONFAILED				417
#define HTTP_CODE_INTERNALERROR					500
#define HTTP_CODE_NOTIMPLEMENTED				501
#define HTTP_CODE_SERVICEUNAVAILABLE			503
#define HTTP_CODE_GATEWAYTIMEOUT				504
#define HTTP_CODE_HTTPVERSIONNOTSUPPORTED		505


template<UINT CodePage>
wstring	_to_wstring(const string &data)
{
	size_t			nLengthW = data.length() + 1;
	wchar_t*	str = new wchar_t[nLengthW];
	memset(str, 0, nLengthW * sizeof(wchar_t));
	MultiByteToWideChar(CodePage, 0, data.c_str(), -1, str, (int)nLengthW);
	wstring	res(str);
	delete[] str;
	return res;
}

template<UINT CodePage>
string	wstring_to_(const wstring &data)
{
	size_t	size = data.length();
	if (size)
	{
		char*		str = new char[2 * size + 1];	//	for russian utf8 string case
		memset(str, 0, 2 * size + 1);
		WideCharToMultiByte(CodePage, 0, data.c_str(), -1, str, 2 * (int)size + 1, NULL, NULL);
		string		res(str);
		delete[] str;
		return res;
	}
	else
		return string();
}

inline wstring cp1251_to_wstring(const string &data) { return _to_wstring<CP_ACP>(data); }
inline wstring oem_to_wstring(const string &data) { return _to_wstring<CP_OEMCP>(data); }
inline wstring utf8_to_wstring(const string &data) { return _to_wstring<CP_UTF8>(data); }
inline string  wstring_to_cp1251(const wstring &data) { return wstring_to_<CP_ACP>(data); }
inline string  wstring_to_oem(const wstring &data) { return wstring_to_<CP_OEMCP>(data); }
inline string  wstring_to_utf8(const wstring &data) { return wstring_to_<CP_UTF8>(data); }

inline string cp1251_to_oem(const string &data) { return wstring_to_oem(_to_wstring<CP_ACP>(data)); }
inline string cp1251_to_utf8(const string &data) { return wstring_to_utf8(_to_wstring<CP_ACP>(data)); }
inline string oem_to_cp1251(const string &data) { return wstring_to_cp1251(_to_wstring<CP_OEMCP>(data)); }
inline string oem_to_utf8(const string &data) { return wstring_to_utf8(_to_wstring<CP_OEMCP>(data)); }
inline string utf8_to_cp1251(const string &data) { return wstring_to_cp1251(_to_wstring<CP_UTF8>(data)); }
inline string utf8_to_oem(const string &data) { return wstring_to_oem(_to_wstring<CP_UTF8>(data)); }


bool				WebAPIUseBearer = false;
wstring				WebAPIURI, WebAPIStringAnswer, WebAPIAccessToken;
http_headers		WebAPIResponceHeaders;
http_client_config	WebAPIconfig;
task<http_response>	WebAPI_last_task;
http_exception		WebAPI_last_http_exception(0);


int	IsAsyncRequestDone()
{
	task<string_t> previousTask;
	int		WebAPILastResponseStatusCode = HTTP_CODE_UNKNOWN;
	WebAPIResponceHeaders.clear();
	WebAPIStringAnswer = L"";

	try
	{
		WebAPI_last_task.wait();
	}
	catch (...)
	{
		return HTTP_CODE_REQUESTTIMEOUT;
	}

	try
	{
		http_response response = WebAPI_last_task.get();
		WebAPIResponceHeaders = response.headers();
		WebAPILastResponseStatusCode = response.status_code();
		switch (WebAPILastResponseStatusCode)
		{
		case status_codes::OK:
		case status_codes::BadRequest:
		case status_codes::InternalError:
		case status_codes::NotFound:
		case status_codes::BadGateway:
		{
			previousTask = response.extract_string();
		}
		break;
		case status_codes::Unauthorized: // TODO need to check
			WebAPIAccessToken = L"";
			previousTask = task_from_result(string_t());
			break;
		default:
		{
			// Handle error cases, for now return empty json value...
			previousTask = task_from_result(string_t());
		}
		break;
		}
	}
	catch (...)
	{
		previousTask = task_from_result(string_t());
	}

	previousTask.wait();

	try
	{
		WebAPIStringAnswer = previousTask.get();
	}
	catch (const http_exception& e)
	{
		WebAPI_last_http_exception = e;
	}
	catch (...)
	{
	}

	return WebAPILastResponseStatusCode;
}


string_t WebAPISetAccessToken()
{
	if (0 == WebAPIAccessToken.length())
	{
		credentials cr = WebAPIconfig.credentials();
		string_t data;
		data = U("grant_type=password&username=") + cr.username() + U("&password=") + cr.password();

		http_client client(WebAPIURI);
		WebAPI_last_task = client.request(methods::POST, U("token"), data, U("application/json"));

		switch (IsAsyncRequestDone())
		{
		case 200:
		{
			json jsonValue;
			jsonValue = json::parse(wstring_to_utf8(WebAPIStringAnswer));
			jsonValue = jsonValue["access_token"];
			WebAPIAccessToken = utf8_to_wstring(jsonValue.get<string>());
			break;
		}

		default:
			WebAPIAccessToken = L"";
			break;
		}
	}
	return WebAPIAccessToken;
}

///////////////////////////////////////////////////////////////////////////////


std::string json_string2xml(const std::string& tag, const json &Value)
{
	return "<"s + tag + ">" + Value.get<json::string_t>() + "</"s + tag + ">";
}


std::string json_float2xml(const std::string& tag, const json &Value)
{
	return "<"s + tag + ">" + std::to_string(Value.get<json::number_float_t>()) + "</"s + tag + ">";
}


std::string json_integer2xml(const std::string& tag, const json &Value)
{
	return "<"s + tag + ">" + std::to_string(Value.get<json::number_integer_t>()) + "</"s + tag + ">";
}


std::string json_unsigned2xml(const std::string& tag, const json &Value)
{
	return "<"s + tag + ">" + std::to_string(Value.get<json::number_unsigned_t>()) + "</"s + tag + ">";
}


std::string json_bool2xml(const std::string& tag, const json &Value)
{
	if (Value.get<json::boolean_t>()) return json_string2xml(tag, json("true"));
	else                              return json_string2xml(tag, json("false"));
}


std::string json_null2xml(const std::string& tag)
{
	return "<"s + tag + "/>";
}


std::string json_type2xml(const std::string& tag, const json &Value)
{
	std::string res = "";

	switch (Value.type())
	{
	case json::value_t::object:
	{
		res += "<"s + tag + ">";
		for (auto& it : Value.items())
		{
			res += json_type2xml(it.key(), it.value());
		}
		res += "</"s + tag + ">";
		return res;
	}

	case json::value_t::array:
	{
		for (auto& it : Value)
		{
			res += json_type2xml(tag, it);
		}
		return res;
	}

	case json::value_t::number_float:
		return json_float2xml(tag, Value);

	case json::value_t::number_integer:
		return json_integer2xml(tag, Value);

	case json::value_t::number_unsigned:
		return json_unsigned2xml(tag, Value);

	case json::value_t::string:
		return json_string2xml(tag, Value);

	case json::value_t::boolean:
		return json_bool2xml(tag, Value);

	default:	//	null
		return json_null2xml(tag);
	}
}


std::string json2xml(const json &Value)
{
	std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s,
				tag = "root"s;
	
	switch (Value.type())
	{
	case json::value_t::object:
	{
		if (1 == Value.size())
			res += json_type2xml(Value.begin().key(), Value.cbegin().value());
		else
		{
			res += "<"s + tag + ">";
			for (auto& it : Value.items())
			{
				res += json_type2xml(it.key(), it.value());
			}
			res += "</"s + tag + ">";
		}
		return res;
	}

	case json::value_t::array:
	{
		if (1 == Value.size())
			res += json_type2xml(tag, Value.cbegin().value());
		else
		{
			res += "<"s + tag + ">";
			for (auto& it : Value)
			{
				res += json_type2xml("element"s, it);
			}
			res += "</"s + tag + ">";
		}
		return res;
	}

	case json::value_t::number_float:
		return res + json_float2xml(tag, Value);

	case json::value_t::number_integer:
		return res + json_integer2xml(tag, Value);

	case json::value_t::number_unsigned:
		return res + json_unsigned2xml(tag, Value);

	case json::value_t::string:
		return res + json_string2xml(tag, Value);

	case json::value_t::boolean:
		return res + json_bool2xml(tag, Value);

	default:	//	null
		return res + json_string2xml(tag, json(""));
	}
}

void jsonToXML(EntContext& ec)
{
	ec.sub = json2xml(ec.obj);
}


struct application_json
{
	static wstring content_type() { return L"application/json"; }
	static void to_json(json& j, const wstring& data) { j = json::parse(wstring_to_utf8(data)); }
	static void from_json(const json& j, wstring& data) { data = utf8_to_wstring(j.dump()); }
};

struct application_xml
{
	static wstring content_type() { return L"application/xml"; }
	static void to_json(json& j, const wstring& data) { j = json::parse(xml2json(wstring_to_utf8(data).c_str())); }
	static void from_json(const json& j, wstring& data)
	{
		if(j.is_string())
			data = utf8_to_wstring(j.get<string>());
		else
			data = utf8_to_wstring(json2xml(j));
	}
};


/*
void	HTTP_METHOD_json(json &EV, const method &mtd)
{
	json &ec.val = val2ref(EV[""]);	//	текущее значение json проекции
	json& ec.sub = jref(EV["$sub"]);
	ViewEntity(jref(EV["ctx"]), jref(EV["$obj"]), ec.val);

	if (ec.val.is_object())
	{
		try
		{
			if (!ec.sub.count("URL")) ec.sub["URL"] = "s";
			if (!ec.sub.count("method")) ec.sub["method"] = ""s;
			if (!ec.sub.count("UserName")) ec.sub["UserName"] = ""s;
			if (!ec.sub.count("Password")) ec.sub["Password"] = ""s;

			WebAPIURI = utf8_to_wstring(ec.sub["URL"].get<string>());

			if (ec.sub[L"UserName"].as_string() != L"")
				WebAPIconfig.set_credentials(credentials(ec.sub[L"UserName"].as_string(), ec.sub[L"Password"].as_string()));

			if (WebAPIUseBearer) // Bearer
			{
				if (0 == WebAPIAccessToken.length())
				{
					WebAPISetAccessToken();
				}
				http_client	client(WebAPIURI);
				http_request requestObj(methods::POST);
				requestObj.headers().add(L"Authorization", L"Bearer " + WebAPIAccessToken);
				requestObj.set_request_uri(ec.sub[L"method"].as_string());
				if (!ec.obj.is_null()) requestObj.set_body(ec.obj.serialize(), U("application/json"));
				WebAPI_last_task = client.request(requestObj);
			}
			else
			{
				http_client client(WebAPIURI, WebAPIconfig);
				if (!ec.obj.is_null())
					WebAPI_last_task = client.request(mtd, ec.sub[L"method"].as_string(), ec.obj.serialize(), U("application/json"));
				else
					WebAPI_last_task = client.request(mtd, ec.sub[L"method"].as_string());
			}

			ec.val = value::object();
			ec.val[L"status_code"] = value(IsAsyncRequestDone());

			if (WebAPIStringAnswer.length())
				ec.val[L"result"] = value::parse(WebAPIStringAnswer);
			else
				ec.val[L"result"] = value();
		}
		catch (const exception& e)
		{
			ec.val = value::object();
			ec.val[L"exception"] = value(utf8_to_wstring(e.what()));
		}
		catch (...)
		{
			ec.val = value::object();
			ec.val[L"exception"] = value(L"...");
		}
	}
	else
	{
		ec.val = value::object();
		ec.val[L"error"] = value(L"$sub must be object");
	}
}
*/

/*
The following are two example URIs and their component parts :

foo://example.com:8042/over/there?name=ferret#nose
\_/  \______________ / \________/ \_________/ \__/
 |           |              |          |       |
scheme   authority         path      query   fragment

authority   = [ userinfo "@" ] host [ ":" port ]
userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
*/
template<typename _convert>
void  HTTP_METHOD(EntContext& ec, const method &mtd)
{
	if (!ec.obj.is_object())
		throw json({ { __FUNCTION__, ": $sub must be object"s} });

	try
	{
		uri base_uri;
		http_client_config	client_config;
		if (!ec.obj.count("URI")) ec.obj["URI"] = ""s;
		json& uri = ec.obj["URI"];

		if (uri.is_string()) base_uri = utf8_to_wstring(uri);
		else if (uri.is_object())
		{
			/*uri_components components;
			if (uri.count("scheme"))	components.m_scheme = utf8_to_wstring(uri["scheme"]);
			if (uri.count("host"))		components.m_host = utf8_to_wstring(uri["host"]);
			if (uri.count("user_info"))	components.m_user_info = utf8_to_wstring(uri["user_info"]);
			if (uri.count("path"))		components.m_path = utf8_to_wstring(uri["path"]);
			if (uri.count("query"))		components.m_query = utf8_to_wstring(uri["query"]);
			if (uri.count("fragment"))	components.m_fragment = utf8_to_wstring(uri["fragment"]);*/

			uri_builder components;
			if (uri.count("scheme"))	components.set_scheme( utf8_to_wstring(uri["scheme"]));
			if (uri.count("host"))		components.set_host( utf8_to_wstring(uri["host"]));
			if (uri.count("user_info"))	components.set_user_info( utf8_to_wstring(uri["user_info"]));
			if (uri.count("path"))		components.set_path( utf8_to_wstring(uri["path"]));
			if (uri.count("query"))		components.set_query( utf8_to_wstring(uri["query"]));
			if (uri.count("fragment"))	components.set_fragment( utf8_to_wstring(uri["fragment"]));
			base_uri = components.to_uri();
		}

		if (ec.obj.count("timeout"))
			client_config.set_timeout(std::chrono::microseconds(ec.obj["timeout"].get<json::number_unsigned_t>()));

		if (ec.obj.count("username") && ec.obj.count("password"))
			client_config.set_credentials(credentials(utf8_to_wstring(ec.obj["username"]), utf8_to_wstring(ec.obj["password"])));

		http_client client(base_uri, client_config);
		http_request requestObj(mtd);

		if (ec.obj.count("header"))
			if (ec.obj["header"].is_object())
				for (auto& it : ec.obj["header"].items())
					requestObj.headers().add(utf8_to_wstring(it.key()), utf8_to_wstring(it.value().is_string() ? it.value().get<string>() : it.value().dump()));

		if (ec.obj.count("body"))
		{
			wstring	body;
			_convert::from_json(ec.obj["body"], body);
			requestObj.set_body(body, _convert::content_type());
		}

		WebAPI_last_task = client.request(requestObj);
		ec.val = IsAsyncRequestDone();
		ec.sub = json::object();
		ec.sub["header"] = json::object();
		ec.sub["body"] = json();

		if (!WebAPIResponceHeaders.empty())
			for (auto& it : WebAPIResponceHeaders)
				ec.sub["header"][wstring_to_utf8(it.first)] = wstring_to_utf8(it.second);

		if (WebAPIStringAnswer.length())
			_convert::to_json(ec.sub["body"], WebAPIStringAnswer);
	}
	catch (json& j) { throw json({ { __FUNCTION__, j } }); }
	catch (json::exception& e) { throw json({ { __FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id) } }); }
	catch (std::exception& e) { throw json({ { __FUNCTION__, "std::exception: "s + e.what() } }); }
	catch (...) { throw json({ { __FUNCTION__, "unknown exception"s } }); }
}

///////////////////////////////////////////////////////////////////////////////

void  HTTP_GET_json(EntContext& ec)    { HTTP_METHOD<application_json>(ec, methods::GET); }
void  HTTP_POST_json(EntContext& ec)   { HTTP_METHOD<application_json>(ec, methods::POST); }
void  HTTP_PUT_json(EntContext& ec)    { HTTP_METHOD<application_json>(ec, methods::PUT); }
void  HTTP_DELETE_json(EntContext& ec) { HTTP_METHOD<application_json>(ec, methods::DEL); }
void  HTTP_GET_xml(EntContext& ec)     { HTTP_METHOD<application_xml>(ec, methods::GET); }
void  HTTP_POST_xml(EntContext& ec)    { HTTP_METHOD<application_xml>(ec, methods::POST); }
void  HTTP_PUT_xml(EntContext& ec)     { HTTP_METHOD<application_xml>(ec, methods::PUT); }
void  HTTP_DELETE_xml(EntContext& ec)  { HTTP_METHOD<application_xml>(ec, methods::DEL); }

/*
void json2html(EntContext& ec)
{
	json &ec.val = val2ref(EV[""]);	//	текущее значение json проекции
	json& ec.sub = val2ref(EV["$sub"]);
	json& ec.obj = val2ref(EV["$obj"]);
	string	result = "";
	
	if (ec.val.is_object())
	{
		if (!ec.val.count("URI")) ec.val["URI"] = ""s;
		
		for (auto& it : Ent.items())
		{
			json	key = it.key();
			CSPush(key.get<string>());	//	debug
										//	проецируем в текущем контексте
			json&	subRef = ReferEntity(EV, key, ec.val);
			json&	objRef = ReferEntity(EV, it.value(), ec.val);
			ViewEntity(EV, objRef, subRef);
		}

		ec.sub = result;
	}
	else
	{
		ec.sub = result;
		throw(__FUNCTION__ + ": $sub must be object"s);
	}
}
*/

__declspec(dllexport) void ImportRelationsModel(json &Ent)
{
	Ent["HTTP"]["RVM_version"] = RVM_version;
	Addx86Entity(Ent, "ToXML"s, jsonToXML, "");
	Addx86Entity(Ent["HTTP"]["GET"], "json"s, HTTP_GET_json,    "");
	Addx86Entity(Ent["HTTP"]["POST"],"json"s, HTTP_POST_json,   "");
	Addx86Entity(Ent["HTTP"]["PUT"], "json"s, HTTP_PUT_json,    "");
	Addx86Entity(Ent["HTTP"]["DEL"], "json"s, HTTP_DELETE_json, "");
	Addx86Entity(Ent["HTTP"]["GET"], "xml"s,  HTTP_GET_xml,     "");
	Addx86Entity(Ent["HTTP"]["POST"],"xml"s,  HTTP_POST_xml,    "");
	Addx86Entity(Ent["HTTP"]["PUT"], "xml"s,  HTTP_PUT_xml,     "");
	Addx86Entity(Ent["HTTP"]["DEL"], "xml"s,  HTTP_DELETE_xml,  "");
//	Addx86Entity(Ent, "html"s, json2html, "Entity that uses JSON templates to convert JSON objects into HTML");
}
