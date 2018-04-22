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
//using namespace web::json;
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
	static void from_json(const json& j, wstring& data) { data = utf8_to_wstring(j.get<string>()); }
};


/*
void	HTTP_METHOD_json(Entity &EV, json &Value, const method &mtd)
{
	json& subview = *EV["->"];
	ViewEntity(EV.parent, *EV["<-"], Value);

	if (Value.is_object())
	{
		try
		{
			if (!subview.count("URL")) subview["URL"] = "s";
			if (!subview.count("method")) subview["method"] = ""s;
			if (!subview.count("UserName")) subview["UserName"] = ""s;
			if (!subview.count("Password")) subview["Password"] = ""s;

			WebAPIURI = utf8_to_wstring(subview["URL"].get<string>());

			if (subview[L"UserName"].as_string() != L"")
				WebAPIconfig.set_credentials(credentials(subview[L"UserName"].as_string(), subview[L"Password"].as_string()));

			if (WebAPIUseBearer) // Bearer
			{
				if (0 == WebAPIAccessToken.length())
				{
					WebAPISetAccessToken();
				}
				http_client	client(WebAPIURI);
				http_request requestObj(methods::POST);
				requestObj.headers().add(L"Authorization", L"Bearer " + WebAPIAccessToken);
				requestObj.set_request_uri(subview[L"method"].as_string());
				if (!objview.is_null()) requestObj.set_body(objview.serialize(), U("application/json"));
				WebAPI_last_task = client.request(requestObj);
			}
			else
			{
				http_client client(WebAPIURI, WebAPIconfig);
				if (!objview.is_null())
					WebAPI_last_task = client.request(mtd, subview[L"method"].as_string(), objview.serialize(), U("application/json"));
				else
					WebAPI_last_task = client.request(mtd, subview[L"method"].as_string());
			}

			Value = value::object();
			Value[L"status_code"] = value(IsAsyncRequestDone());

			if (WebAPIStringAnswer.length())
				Value[L"result"] = value::parse(WebAPIStringAnswer);
			else
				Value[L"result"] = value();
		}
		catch (const exception& e)
		{
			Value = value::object();
			Value[L"exception"] = value(utf8_to_wstring(e.what()));
		}
		catch (...)
		{
			Value = value::object();
			Value[L"exception"] = value(L"...");
		}
	}
	else
	{
		Value = value::object();
		Value[L"error"] = value(L"-> must be object");
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
void __fastcall HTTP_METHOD(Entity &EV, json &Value, const method &mtd)
{
	json& subview = *EV["->"];
	ViewEntity(EV.parent, *EV["<-"], Value);

	if (Value.is_object())
	{
		uri base_uri;
		http_client_config	client_config;
		if (!Value.count("URI")) Value["URI"] = ""s;
		json& uri = Value["URI"];

		if (uri.is_string()) base_uri = utf8_to_wstring(uri);
		else if (uri.is_object())
		{
			uri_components components;
			if (uri.count("scheme"))	components.m_scheme = utf8_to_wstring(uri["scheme"]);
			if (uri.count("host"))		components.m_host = utf8_to_wstring(uri["host"]);
			if (uri.count("user_info"))	components.m_user_info = utf8_to_wstring(uri["user_info"]);
			if (uri.count("path"))		components.m_path = utf8_to_wstring(uri["path"]);
			if (uri.count("query"))		components.m_query = utf8_to_wstring(uri["query"]);
			if (uri.count("fragment"))	components.m_fragment = utf8_to_wstring(uri["fragment"]);
			base_uri = components;
		}

		if (Value.count("timeout"))
			client_config.set_timeout(std::chrono::microseconds(Value["timeout"].get<json::number_unsigned_t>()));

		if (Value.count("username") && Value.count("password"))
			client_config.set_credentials(credentials(utf8_to_wstring(Value["username"]), utf8_to_wstring(Value["password"])));

		http_client client(base_uri, client_config);
		http_request requestObj(mtd);

		if (Value.count("header"))
			if (Value["header"].is_object())
				for (auto& it : Value["header"].items())
					requestObj.headers().add(utf8_to_wstring(it.key()), utf8_to_wstring(it.value().is_string() ? it.value().get<string>() : it.value().dump()));

		if (Value.count("body"))
		{
			wstring	body;
			_convert::from_json(Value["body"], body);
			requestObj.set_body(body, _convert::content_type());
		}
		
		WebAPI_last_task = client.request(requestObj);
		Value = IsAsyncRequestDone();
		subview = json::object();
		subview["header"] = json::object();
		subview["body"] = json();

		if (!WebAPIResponceHeaders.empty())
			for (auto& it : WebAPIResponceHeaders)
				subview["header"][wstring_to_utf8(it.first)] = wstring_to_utf8(it.second);

		if (WebAPIStringAnswer.length())
			_convert::to_json(subview["body"], WebAPIStringAnswer);
	}
	else
	{
		Value = HTTP_CODE_UNKNOWN;
		throw(__FUNCTION__ + ": ->/ must be object"s);
	}
}

///////////////////////////////////////////////////////////////////////////////

void __fastcall HTTP_GET_json(Entity &EV, json &Value)    { HTTP_METHOD<application_json>(EV, Value, methods::GET); }
void __fastcall HTTP_POST_json(Entity &EV, json &Value)   { HTTP_METHOD<application_json>(EV, Value, methods::POST); }
void __fastcall HTTP_PUT_json(Entity &EV, json &Value)    { HTTP_METHOD<application_json>(EV, Value, methods::PUT); }
void __fastcall HTTP_DELETE_json(Entity &EV, json &Value) { HTTP_METHOD<application_json>(EV, Value, methods::DEL); }
void __fastcall HTTP_GET_xml(Entity &EV, json &Value)     { HTTP_METHOD<application_xml>(EV, Value, methods::GET); }
void __fastcall HTTP_POST_xml(Entity &EV, json &Value)    { HTTP_METHOD<application_xml>(EV, Value, methods::POST); }
void __fastcall HTTP_PUT_xml(Entity &EV, json &Value)     { HTTP_METHOD<application_xml>(EV, Value, methods::PUT); }
void __fastcall HTTP_DELETE_xml(Entity &EV, json &Value)  { HTTP_METHOD<application_xml>(EV, Value, methods::DEL); }

__declspec(dllexport) void __fastcall ImportRelationsModel(json &Ent)
{
	Addx86Entity(Ent["HTTP"]["GET"], "json"s, HTTP_GET_json,    "");
	Addx86Entity(Ent["HTTP"]["POST"],"json"s, HTTP_POST_json,   "");
	Addx86Entity(Ent["HTTP"]["PUT"], "json"s, HTTP_PUT_json,    "");
	Addx86Entity(Ent["HTTP"]["DEL"], "json"s, HTTP_DELETE_json, "");
	Addx86Entity(Ent["HTTP"]["GET"], "xml"s,  HTTP_GET_xml,     "");
	Addx86Entity(Ent["HTTP"]["POST"],"xml"s,  HTTP_POST_xml,    "");
	Addx86Entity(Ent["HTTP"]["PUT"], "xml"s,  HTTP_PUT_xml,     "");
	Addx86Entity(Ent["HTTP"]["DEL"], "xml"s,  HTTP_DELETE_xml,  "");
}
