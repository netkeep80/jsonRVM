/*        R
	   S__|__O
	 O   _|_   S
  R__|__/_|_\__|__R  jsonRVM
	 |  \_|_/  |     json Relations (Model) Virtual Machine
	 S    |    O     https://github.com/netkeep80/jsonRVM
		__|__
	   /  |  \
	  /___|___\
Fractal Triune Entity

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright � 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include "httplib.h"
#include <iostream>
#include "string_utils.h"
#include "jsonRVM.h"
#include "xml2json.hpp"

namespace rm
{
	using namespace std;
	using namespace httplib;

	inline string json_string2xml(const std::string& tag, const json& Value)
	{
		return "<"s + tag + ">" + Value.get<json::string_t>() + "</"s + tag + ">";
	}


	inline string json_float2xml(const std::string& tag, const json& Value)
	{
		return "<"s + tag + ">" + std::to_string(Value.get<json::number_float_t>()) + "</"s + tag + ">";
	}


	inline string json_integer2xml(const std::string& tag, const json& Value)
	{
		return "<"s + tag + ">" + std::to_string(Value.get<json::number_integer_t>()) + "</"s + tag + ">";
	}


	inline string json_unsigned2xml(const std::string& tag, const json& Value)
	{
		return "<"s + tag + ">" + std::to_string(Value.get<json::number_unsigned_t>()) + "</"s + tag + ">";
	}


	inline string json_bool2xml(const std::string& tag, const json& Value)
	{
		if (Value.get<json::boolean_t>()) return json_string2xml(tag, json("true"));
		else                              return json_string2xml(tag, json("false"));
	}


	inline string json_null2xml(const std::string& tag)
	{
		return "<"s + tag + "/>";
	}


	inline string json_type2xml(const std::string& tag, const json& Value)
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


	inline string json2xml(const json& Value)
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


	void jsonToXML(jsonRVM& rvm, EntContext& ec)
	{
		ec.sub = json2xml(ec.obj);
	}


	struct application_urlencoded
	{
		template<class _T>
		static auto content_type() { return _T("application/x-www-form-urlencoded"); }
		static void to_json(json& j, const wstring& data) { j = wstring_to_utf8(data); }
		static void to_json(json& j, const string& data) { j = data; }
		static void from_json(const json& j, string& data)
		{
			switch (j.type())
			{
			case json::value_t::string:
				data = j.get<string>();
				return;

			case json::value_t::object:
				data = ""s;

				for (auto& it : j.items())
				{
					if (data.length() == 0)
					{
						if (it.value().type() == json::value_t::string)
							data += it.key() + "="s + it.value().get<string>();
						else
							data += it.key() + "="s + it.value().dump();
					}
					else
					{
						if (it.value().type() == json::value_t::string)
							data += "&"s + it.key() + "="s + it.value().get<string>();
						else
							data += "&"s + it.key() + "="s + it.value().dump();
					}
				}
				return;

			default:
				throw json("Body of request must be string or object!");
			}
		}
		static void from_json(const json& j, wstring& data)
		{
			string _data;
			from_json(j, _data);
			data = utf8_to_wstring(_data);
		}
	};


	struct application_json
	{
		template<class _T>
		static auto content_type() { return _T("application/json"); }
		static void to_json(json& j, const wstring& data) { j = json::parse(wstring_to_utf8(data)); }
		static void to_json(json& j, const string& data) { j = json::parse(data); }
		static void from_json(const json& j, wstring& data) { data = utf8_to_wstring(j.dump()); }
		static void from_json(const json& j, string& data) { data = j.dump(); }
	};


	struct application_xml
	{
		template<class _T>
		static auto content_type() { return _T("application/xml"); }
		static void to_json(json& j, const wstring& data) { j = json::parse(xml2json(wstring_to_utf8(data).c_str())); }
		static void to_json(json& j, const string& data) { j = json::parse(xml2json(data.c_str())); }
		static void from_json(const json& j, wstring& data)
		{
			if (j.is_string())
				data = utf8_to_wstring(j.get<string>());
			else
				data = utf8_to_wstring(json2xml(j));
		}
		static void from_json(const json& j, string& data)
		{
			if (j.is_string())
				data = j.get<string>();
			else
				data = json2xml(j);
		}
	};


	using method = std::string;

	struct methods
	{
		static inline char* HEAD = "HEAD";
		static inline char* GET = "GET";
		static inline char* POST = "POST";
		static inline char* PUT = "PUT";
		static inline char* DEL = "DEL";
		static inline char* PATCH = "PATCH";
	};

	/*
	Berners-Lee, et al.         Standards Track                    [Page 50]
	RFC 3986                   URI Generic Syntax               January 2005

	Appendix B.  Parsing a URI Reference with a Regular Expression

	   As the "first-match-wins" algorithm is identical to the "greedy"
	   disambiguation method used by POSIX regular expressions, it is
	   natural and commonplace to use a regular expression for parsing the
	   potential five components of a URI reference.

	   The following line is the regular expression for breaking-down a
	   well-formed URI reference into its components.

		  ^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?
		   12            3  4          5       6  7        8 9

	   The numbers in the second line above are only to assist readability;
	   they indicate the reference points for each subexpression (i.e., each
	   paired parenthesis).  We refer to the value matched for subexpression
	   <n> as $<n>.  For example, matching the above expression to

		  http://www.ics.uci.edu/pub/ietf/uri/#Related

	   results in the following subexpression matches:

		  $1 = http:
		  $2 = http
		  $3 = //www.ics.uci.edu
		  $4 = www.ics.uci.edu
		  $5 = /pub/ietf/uri/
		  $6 = <undefined>
		  $7 = <undefined>
		  $8 = #Related
		  $9 = Related

	   where <undefined> indicates that the component is not present, as is
	   the case for the query component in the above example.  Therefore, we
	   can determine the value of the five components as

		  scheme    = $2
		  authority = $4
		  path      = $5
		  query     = $7
		  fragment  = $9

	   Going in the opposite direction, we can recreate a URI reference from
	   its components by using the algorithm of Section 5.3.


	The following are two example URIs and their component parts :
	$2 ://    $4          $5         ?    $7     # $9
	foo://example.com:8042/over/there?name=ferret#nose
	\_/   \_____________ / \________/ \_________/ \__/
	 |           |              |          |       |
	scheme   authority         path      query   fragment

	authority   = [ userinfo "@" ] host [ ":" port ]
	userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )

	5.3.  Component Recomposition

	   Parsed URI components can be recomposed to obtain the corresponding
	   URI reference string.  Using pseudocode, this would be:

		  result = ""

		  if defined(scheme) then
			 append scheme to result;
			 append ":" to result;
		  endif;

		  if defined(authority) then
			 append "//" to result;
			 append authority to result;
		  endif;

		  append path to result;

		  if defined(query) then
			 append "?" to result;
			 append query to result;
		  endif;

		  if defined(fragment) then
			 append "#" to result;
			 append fragment to result;
		  endif;

		  return result;

	   Note that we are careful to preserve the distinction between a
	   component that is undefined, meaning that its separator was not
	   present in the reference, and a component that is empty, meaning that
	   the separator was present and was immediately followed by the next
	   component separator or the end of the reference.

	*/

#define CA_CERT_FILE "./ca-bundle.crt"

	template<typename _convert, const method& mtd>
	void  HTTP_METHOD(jsonRVM& rvm, EntContext& ec)
	{
		if (!ec.obj.is_object())
			ec.throw_json(__FUNCTION__, ": $obj must be object"s);

		try
		{
			string base_uri, uri_path;
			if (!ec.obj.count("URI")) ec.obj["URI"] = ""s;
			json& uri = ec.obj["URI"];

			if (uri.is_string())
			{
				const static std::regex re(R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)");
				std::cmatch components;

				if (std::regex_match(uri.get_ref<string&>().c_str(), components, re)) {

					auto scheme = components[2].str();
					auto authority = components[4].str();

					if (scheme.size())
						base_uri = scheme + "://";

					if (authority.size())
						base_uri += authority;

					auto path = components[5].str();
					auto query = components[7].str();
					auto fragment = components[9].str();

					uri_path = path;

					if (query.size())
						uri_path += "?" + query;

					if (fragment.size())
						uri_path += "#" + fragment;
				}
			}
			else if (uri.is_object())
			{
				if (uri.count("scheme"))
					base_uri = uri["scheme"] + "://";

				if (uri.count("authority"))
					base_uri += uri["authority"];
				else
				{	//	authority   = [ userinfo "@" ] host [ ":" port ]
					if (uri.count("user_info"))
						base_uri += uri["user_info"] + "@";
					if (uri.count("host"))
						base_uri += uri["host"];
					if (uri.count("port"))
						base_uri += ":" + uri["port"];
				}

				if (uri.count("path"))
					uri_path = uri["path"];

				if (uri.count("query"))
					uri_path += "?" + uri["query"];

				if (uri.count("fragment"))
					uri_path += "#" + uri["fragment"];
			}

			if (ec.obj.count("timeout"))
			{
				//client_config.set_timeout(std::chrono::microseconds(ec.obj["timeout"].get<json::number_unsigned_t>()));
				//cli.set_connection_timeout(0, 300000); // 300 milliseconds
				//cli.set_read_timeout(5, 0); // 5 seconds
				//cli.set_write_timeout(5, 0); // 5 seconds
			}

			if (ec.obj.count("username") && ec.obj.count("password"))
			{
				//client_config.set_credentials(credentials(utf8_to_wstring(ec.obj["username"]), utf8_to_wstring(ec.obj["password"])));
				// Basic Authentication
				//cli.set_basic_auth("user", "pass");
				// Digest Authentication
				//cli.set_digest_auth("user", "pass");
				// Bearer Token Authentication
				//cli.set_bearer_token_auth("token");
			}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
			httplib::SSLClient cli(base_uri);
			cli.set_ca_cert_path(CA_CERT_FILE);
			cli.enable_server_certificate_verification(true);
#else
			httplib::Client cli(base_uri.c_str());
#endif

			httplib::Request	requestObj;
			httplib::Progress	progress;

			requestObj.method = mtd;
			requestObj.path = uri_path;
			requestObj.progress = std::move(progress);

			if (ec.obj.count("header"))
				if (ec.obj["header"].is_object())
					for (auto& it : ec.obj["header"].items())
						requestObj.set_header(it.key().c_str(), it.value().is_string() ? it.value().get_ref<string&>().c_str() : it.value().dump().c_str());

			if (ec.obj.count("body"))
				_convert::from_json(ec.obj["body"], requestObj.body);

			if (auto res = cli.send(requestObj))
			{
				ec.res = res->status;
				ec.sub = json::object();
				ec.sub["base_uri"] = base_uri;
				ec.sub["uri_path"] = uri_path;
				ec.sub["header"] = json::object();
				ec.sub["body"] = json();
				if (res->body.length())
					_convert::to_json(ec.sub["body"], res->body);

				if (!res->headers.empty())
					for (auto& it : res->headers)
						ec.sub["header"][it.first] = it.second;
			}
			else
			{
				throw json("error code: "s + to_string(static_cast<std::underlying_type<Error>::type>(res.error())));
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
				auto result = cli.get_openssl_verify_result();
				if (result)
					throw json("verify error: "s + X509_verify_cert_error_string(result));;
#endif
			}
		}
		catch (json& j) { ec.throw_json(__FUNCTION__, j); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
	}


#define add_http_entity(method, converter_type)																					\
	rvm.AddBaseEntity(																											\
		rvm["http"][ #method ],																									\
		string(#converter_type),																								\
		HTTP_METHOD<application_##converter_type, methods::##method>,															\
		"Calls http webapi "s + #method + " method with "s + application_##converter_type::content_type<string>() + " content_type"s		\
	);

#define add_http_entites(converter_type)	\
	add_http_entity(HEAD, converter_type);	\
	add_http_entity(GET, converter_type);	\
	add_http_entity(POST, converter_type);	\
	add_http_entity(PUT, converter_type);	\
	add_http_entity(DEL, converter_type);	\
	add_http_entity(PATCH, converter_type);

	const string& ImportRelationsModel(jsonRVM& rvm)
	{
		rvm.AddBaseEntity(rvm, "ToXML"s, jsonToXML, "");
		//	rvm.AddBaseEntity(rvm, "html"s, json2html, "Entity that uses JSON templates to convert JSON objects into HTML");

		add_http_entites(urlencoded);
		add_http_entites(json);
		add_http_entites(xml);

		return rmvm_version;
	}
}
