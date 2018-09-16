#include <cmath>
#include <cpprest/json.h> // JSON library 
#include <jsoncpp/json/config.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include "RestHandler.h"
#include "Configuration.h"
#include "Utility.h"

RestHandler::RestHandler(int port)
{
	const static char fname[] = "RestHandler::RestHandler() ";

	// init
	std::string address = "http://0.0.0.0:";
	address += std::to_string(port);

	m_listener = std::make_shared<http_listener>(address);
	m_listener->support(methods::GET, std::bind(&RestHandler::handle_get, this, std::placeholders::_1));
	m_listener->support(methods::PUT, std::bind(&RestHandler::handle_put, this, std::placeholders::_1));
	m_listener->support(methods::POST, std::bind(&RestHandler::handle_post, this, std::placeholders::_1));
	m_listener->support(methods::DEL, std::bind(&RestHandler::handle_delete, this, std::placeholders::_1));

	this->open();

	LOG(INFO) << fname << "Listening for requests at:" << address << std::endl;
}

RestHandler::~RestHandler()
{
	this->close();
}

void RestHandler::open()
{
	m_listener->open().wait();
}

void RestHandler::close()
{
	m_listener->close();// .wait();
}

void RestHandler::handle_get(http_request message)
{
	try
	{
		//LOG(INFO) << L"get request:" << message.to_string() << endl;
		auto path = GET_STD_STRING(message.relative_uri().path());

		if (path == string("/view"))
		{
			message.reply(status_codes::OK, Configuration::instance()->getApplicationJson());
		}
		else if (path == "/config")
		{
			message.reply(status_codes::OK, Configuration::prettyJson(GET_STD_STRING(Configuration::instance()->getConfigContentStr())));
		}
		else
		{
			throw std::invalid_argument("No such path");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
}

void RestHandler::handle_put(http_request message)
{
	try
	{
		LOG(INFO) << "put request:" << message.to_string() << endl;
		checkToken(getToken(message));
		auto path = GET_STD_STRING(message.relative_uri().path());
		auto jsonApp = message.extract_json(true).get();
		if (jsonApp.is_null())
		{
			throw std::invalid_argument("invalid json format");
		}

		string regPrefix = "/reg";
		if (path.find(regPrefix) == 0)
		{
			auto app = Configuration::instance()->addApp(jsonApp);
			message.reply(status_codes::OK, Configuration::prettyJson(GET_STD_STRING(app->AsJson(true).serialize())));
		}
		else
		{
			message.reply(status_codes::ServiceUnavailable, "No such path");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
	return;
}

void RestHandler::handle_post(http_request message)
{
	try
	{
		LOG(INFO) << "post request:" << message.to_string() << endl;
		checkToken(getToken(message));
		auto path = GET_STD_STRING(message.relative_uri().path());
		auto jsonApp = message.extract_json(true).get();

		string appName;
		if (!jsonApp.is_null())
		{
			if (HAS_JSON_FIELD(jsonApp.as_object(), "name"))
			{
				auto name = GET_JSON_STR_VALUE(jsonApp.as_object(), "name");
				if (name != "all")
				{
					appName = name;
				}
			}
			else
			{
				throw std::invalid_argument("invalid application name");
			}
		}
		else
		{
			throw std::invalid_argument("invalid json format");
		}

		string stopPrefix = "/stop";
		string startPrefix = "/start";
		if (path.find(stopPrefix) == 0)
		{
			std::vector<std::string> result;
			if (appName.length() > 0)
			{
				Configuration::instance()->stopApp(appName);
				result.push_back(appName);
			}
			else
			{
				result = Configuration::instance()->stopAllApp();
			}
			string respMsg;
			if (result.size() > 0)
			{
				respMsg = "Application <";
				for (auto app : result)
				{
					respMsg += app;
					respMsg += ",";
				}
				respMsg += "> stopped.";
			}
			else
			{
				respMsg = "No application stopped.";
			}
			message.reply(status_codes::OK, respMsg);
		}
		else if (path.find(startPrefix) == 0)
		{
			std::vector<std::string> result;
			if (appName.length() > 0)
			{
				Configuration::instance()->startApp(appName);
				result.push_back(appName);
			}
			else
			{
				result = Configuration::instance()->startAllApp();
			}
			string respMsg;
			if (result.size() > 0)
			{
				respMsg = "Application <";
				for (auto app : result)
				{
					respMsg += app;
					respMsg += ",";
				}
				respMsg += "> started.";
			}
			else
			{
				respMsg = "No application started.";
			}
			message.reply(status_codes::OK, respMsg);
		}
		else
		{
			message.reply(status_codes::ServiceUnavailable, "No such path");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
}

void RestHandler::handle_delete(http_request message)
{
	try
	{
		LOG(INFO) << "delete request:" <<  message.to_string() << endl;
		checkToken(getToken(message));
		auto path = GET_STD_STRING(message.relative_uri().path());
		auto jsonApp = message.extract_json(true).get();

		string appName;
		if (!jsonApp.is_null())
		{
			if (HAS_JSON_FIELD(jsonApp.as_object(), "name"))
			{
				appName = GET_JSON_STR_VALUE(jsonApp.as_object(), "name");				
			}
			else
			{
				throw std::invalid_argument("invalid application name");
			}
		}
		else
		{
			throw std::invalid_argument("invalid json format");
		}
		string unregPrefix = "/unreg";
		if (path.find(unregPrefix) == 0)
		{
			std::vector<std::string> result;
			Configuration::instance()->removeApp(appName);
			result.push_back(appName);
			string respMsg;
			if (result.size() > 0)
			{
				respMsg = "Application <";
				for (auto app : result)
				{
					respMsg += app;
					respMsg += ",";
				}
				respMsg += "> stopped and unregisted.";
			}
			else
			{
				respMsg = "No application unregisted.";
			}
			message.reply(status_codes::OK, respMsg);
		}
		else
		{
			message.reply(status_codes::ServiceUnavailable, "No such path");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
	return;
}

void RestHandler::handle_error(pplx::task<void>& t)
{
	const static char fname[] = "Configuration::handle_error() ";

	try
	{
		t.get();
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << fname << "ERROR:" << e.what() << endl;
	}
	catch (...)
	{
		LOG(ERROR) << fname << "ERROR:" << "unknown exception" << endl;
	}
}


bool RestHandler::checkToken(const std::string& token)
{
	auto clientTime = Utility::convertStr2Time(token);
	auto diff = std::abs(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - clientTime).count());
	if (diff > 15)
	{
		throw std::invalid_argument("token invalid: untrusted");
	}
	// TODO: token check here.
	return true;
}

std::string RestHandler::getToken(const http_request& message)
{
	std::string token;
	if (message.headers().has("token"))
	{
		auto tokenInHeader = message.headers().find("token");
		token = Utility::decode64(tokenInHeader->second);
	}
	
	if (token.empty())
	{
		throw std::invalid_argument("Access denied:must have token.");
	}
	return token;
}
