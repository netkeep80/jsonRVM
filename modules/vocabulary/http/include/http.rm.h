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
Copyright (c) 2016-2021 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
#include "vm.rm.h"
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


	void jsonToXML(vm& rmvm, vm_rel& $)
	{
		$.sub = json2xml($.obj);
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
		struct GET
		{
			static inline char* name = "GET";
			static void add(Server& svr, const char* pattern, Server::Handler handler) { svr.Get(pattern, handler); }
		};
		struct POST
		{
			static inline char* name = "POST";
			static void add(Server& svr, const char* pattern, Server::Handler handler) { svr.Post(pattern, handler); }
		};
		struct PUT
		{
			static inline char* name = "PUT";
			static void add(Server& svr, const char* pattern, Server::Handler handler) { svr.Put(pattern, handler); }
		};
		struct DEL
		{
			static inline char* name = "DEL";
			static void add(Server& svr, const char* pattern, Server::Handler handler) { svr.Delete(pattern, handler); }
		};
		struct PATCH
		{
			static inline char* name = "PATCH";
			static void add(Server& svr, const char* pattern, Server::Handler handler) { svr.Patch(pattern, handler); }
		};
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
	void  HTTP_METHOD(vm& rmvm, vm_rel& $)
	{
		if (!$.obj.is_object())
			$.throw_json(__func__, ": $obj must be object"s);

		try
		{
			string base_uri, uri_path;
			json const& uri = $.obj.count("URI") ? $.obj["URI"] : ""s;

			if (uri.is_string())
			{
				const static std::regex re(R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)");
				std::cmatch components;

				if (std::regex_match(uri.get_ref<string const&>().c_str(), components, re)) {

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

			if ($.obj.count("timeout"))
			{
				//client_config.set_timeout(std::chrono::microseconds($.obj["timeout"].get<json::number_unsigned_t>()));
				//cli.set_connection_timeout(0, 300000); // 300 milliseconds
				//cli.set_read_timeout(5, 0); // 5 seconds
				//cli.set_write_timeout(5, 0); // 5 seconds
			}

			if ($.obj.count("username") && $.obj.count("password"))
			{
				//client_config.set_credentials(credentials(utf8_to_wstring($.obj["username"]), utf8_to_wstring($.obj["password"])));
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

			if ($.obj.count("header"))
				if ($.obj["header"].is_object())
					for (auto& it : $.obj["header"].items())
						requestObj.set_header(it.key().c_str(), it.value().is_string() ? it.value().get_ref<string const&>().c_str() : it.value().dump().c_str());

			if ($.obj.count("body"))
				_convert::from_json($.obj["body"], requestObj.body);

			if (auto res = cli.send(requestObj))
			{
				$.sub = json::object();
				$.sub["status"] = res->status;
				$.sub["base_uri"] = base_uri;
				$.sub["uri_path"] = uri_path;
				$.sub["header"] = json::object();
				$.sub["body"] = json();
				if (res->body.length())
					_convert::to_json($.sub["body"], res->body);

				if (!res->headers.empty())
					for (auto& it : res->headers)
						$.sub["header"][it.first] = it.second;
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
		catch (json& j) { $.throw_json(__func__, j); }
		catch (json::exception& e) { $.throw_json(__func__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__func__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__func__, "unknown exception"s); }
	}


	inline std::string http_dump_headers(const Headers& headers) {
		std::string s;
		char buf[BUFSIZ];

		for (auto it = headers.begin(); it != headers.end(); ++it) {
			const auto& x = *it;
			snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
			s += buf;
		}

		return s;
	}


	inline std::string http_log(const Request& req, const Response& res) {
		std::string s;
		char buf[BUFSIZ];

		s += "================================\n";

		snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
			req.version.c_str(), req.path.c_str());
		s += buf;

		std::string query;
		for (auto it = req.params.begin(); it != req.params.end(); ++it) {
			const auto& x = *it;
			snprintf(buf, sizeof(buf), "%c%s=%s",
				(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
				x.second.c_str());
			query += buf;
		}
		snprintf(buf, sizeof(buf), "%s\n", query.c_str());
		s += buf;

		s += http_dump_headers(req.headers);

		s += "--------------------------------\n";

		snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
		s += buf;
		s += http_dump_headers(res.headers);
		s += "\n";

		if (!res.body.empty()) { s += res.body; }

		s += "\n";

		return s;
	}


	inline unsigned get_unsigned(const json& obj, const string& field, unsigned default = unsigned())
	{
		if (obj.count(field))
		{
			const json& val = obj[field];
			switch (val.type())
			{
			case json::value_t::number_float:   return unsigned(val.get<json::number_float_t>());
			case json::value_t::number_integer: return unsigned(val.get<json::number_integer_t>());
			case json::value_t::number_unsigned:return unsigned(val.get<json::number_unsigned_t>());
			case json::value_t::boolean:        return unsigned(val.get<json::boolean_t>() ? 1 : 0);
			case json::value_t::string:         return unsigned(std::stoul(val.get_ref<const json::string_t&>()));
			default: break;
			}
		}

		return default;
	}

	template<typename method>
	void	http_add_methods(vm& rmvm, vm_rel& $, Server& svr, json& api)
	{
		if (!api.is_object())
			$.throw_json(__func__, ": $obj/GET must be object"s);

		for (auto& http_method : api.items())
		{
			if (!http_method.value().is_object())
				$.throw_json(__func__, ": $obj/GET/"s + http_method.key() + " must be object"s);

			method::add(svr, http_method.key().c_str(), [&, http_method](const Request& req, Response& res)
				{
					json	rel;
					json&	ent = http_method.value()["$ent"];
					using _convert = application_json;

					if (req.body.length())
						_convert::to_json(rel, req.body);

					try
					{
						rmvm.objectify(ent, rel);
						res.status = 200;
					}
					catch (json& j)
					{
						rel = j;
						res.status = 400;
					}					
					
					res.set_content(rel.dump(2).c_str(), "application/json");
				}
			);
		}
	}


	inline void  http_service(vm& rmvm, vm_rel& $)
	{
		//typename _convert;
		method mtd;

		if (!$.obj.is_object())
			$.throw_json(__func__, ": $obj must be object"s);

		string	host = "0.0.0.0"s;

		if ($.obj.count("host"))
		{
			if (!$.obj["host"].is_string())
				$.throw_json(__func__, ": $obj/host must be string"s);

			host = $.obj["host"].get<json::string_t>();
		}

		unsigned port = get_unsigned($.obj, "port", 80);

		try
		{
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
			SSLServer svr(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
#else
			Server svr;
#endif

			if (!svr.is_valid()) {
				printf("server has an error...\n");
				return;
			}

			svr.Get("/", [=](const Request& /*req*/, Response& res) {
				const char* fmt = R"(
	          R
	       S__|__O
	     O   _|_   S
	  R__|__/_|_\__|__R  rmvm [Version %s]
	     |  \_|_/  |     json Relations (Model) Virtual Machine
	     S    |    O     https://github.com/netkeep80/jsonRVM
	        __|__
	       /  |  \
	      /___|___\
	Fractal Triune Entity

	Licensed under the MIT License <http://opensource.org/licenses/MIT>.
	Copyright (c) 2016-2021 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.
				)";
				char buf[BUFSIZ];
				snprintf(buf, sizeof(buf), fmt, vm::version.c_str());
				res.set_content(buf, "text/plain");
				});

			svr.Get("/stop",
				[&](const Request& /*req*/, Response& /*res*/) { svr.stop(); });

			svr.set_logger([](const Request& req, const Response& res) {
				printf("%s", http_log(req, res).c_str());
				});

			if ($.obj.count(methods::GET::name))
				http_add_methods<methods::GET>(rmvm, $, svr, $.obj[methods::GET::name]);

			if ($.obj.count(methods::POST::name))
				http_add_methods<methods::POST>(rmvm, $, svr, $.obj[methods::POST::name]);

			if ($.obj.count(methods::PUT::name))
				http_add_methods<methods::PUT>(rmvm, $, svr, $.obj[methods::PUT::name]);

			if ($.obj.count(methods::DEL::name))
				http_add_methods<methods::DEL>(rmvm, $, svr, $.obj[methods::DEL::name]);

			if ($.obj.count(methods::PATCH::name))
				http_add_methods<methods::PATCH>(rmvm, $, svr, $.obj[methods::PATCH::name]);

			svr.listen(host.c_str(), port);
			
			/*
			if ($.obj.count("header"))
				if ($.obj["header"].is_object())
					for (auto& it : $.obj["header"].items())
						requestObj.set_header(it.key().c_str(), it.value().is_string() ? it.value().get_ref<string&>().c_str() : it.value().dump().c_str());

			if ($.obj.count("body"))
				_convert::from_json($.obj["body"], requestObj.body);

			if (auto res = cli.send(requestObj))
			{
				$.rel = res->status;
				$.sub = json::object();
				$.sub["base_uri"] = base_uri;
				$.sub["uri_path"] = uri_path;
				$.sub["header"] = json::object();
				$.sub["body"] = json();
				if (res->body.length())
					_convert::to_json($.sub["body"], res->body);

				if (!res->headers.empty())
					for (auto& it : res->headers)
						$.sub["header"][it.first] = it.second;
			}
			else
			{
				throw json("error code: "s + to_string(static_cast<std::underlying_type<Error>::type>(res.error())));
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
				auto result = cli.get_openssl_verify_result();
				if (result)
					throw json("verify error: "s + X509_verify_cert_error_string(result));;
#endif
			}*/

		}
		catch (json& j) { $.throw_json(__func__, j); }
		catch (json::exception& e) { $.throw_json(__func__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__func__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__func__, "unknown exception"s); }
	}


#define add_http_entity(method, converter_type)																					\
	rmvm.add_base_entity(																											\
		rmvm["http"][ methods::##method::name ],																									\
		string(#converter_type),																								\
		HTTP_METHOD<application_##converter_type, methods::##method::name>,															\
		"Calls http webapi "s + methods::##method::name + " method with "s + application_##converter_type::content_type<string>() + " content_type"s		\
	);

#define add_http_entites(converter_type)	\
	add_http_entity(GET, converter_type);	\
	add_http_entity(POST, converter_type);	\
	add_http_entity(PUT, converter_type);	\
	add_http_entity(DEL, converter_type);	\
	add_http_entity(PATCH, converter_type);

	const string& import_relations_model_to(vm& rmvm)
	{
		rmvm.add_base_entity(rmvm, "ToXML"s, jsonToXML, "");
		//	rmvm.add_base_entity(rmvm, "html"s, json2html, "Entity that uses JSON templates to convert JSON objects into HTML");

		add_http_entites(urlencoded);
		add_http_entites(json);
		add_http_entites(xml);

		rmvm.add_base_entity(rmvm["http"], "service"s, http_service, "");

		return vm::version;
	}
}
