#include "ArgumentParser.h"
#include <iostream>
#include <boost/program_options.hpp>
#include "../ApplicationManager/Utility.h"

using namespace std;

#define HELP_ARG_CHECK_WITH_RETURN if (m_commandLineVariables.count("help") > 0) {cout << desc << std::endl; return; }
#define RESPONSE_CHECK_WITH_RETURN if (response.status_code() != status_codes::OK) {cout << response.extract_utf8string(true).get() << std::endl; return;}

ArgumentParser::ArgumentParser(int argc, char* argv[], int listenPort)
	:m_listenPort(listenPort)
{
	po::options_description global("Global options");
	global.add_options()
		("help,h", "produce help message")
		("command", po::value<std::string>(), "command to execute")
		("subargs", po::value<std::vector<std::string> >(), "arguments for command");

	po::positional_options_description pos;
	pos.add("command", 1).
		add("subargs", -1);

	auto parsed = po::command_line_parser(argc, argv).options(global).positional(pos).allow_unregistered().run();
	m_pasrsedOptions = parsed.options;
	po::store(parsed, m_commandLineVariables);
	po::notify(m_commandLineVariables);
}


ArgumentParser::~ArgumentParser()
{
}

void ArgumentParser::parse()
{
	if (m_commandLineVariables.empty())
	{
		printMainHelp();
		return;
	}
	
	
	std::string cmd = m_commandLineVariables["command"].as<std::string>();
	if (cmd == "reg")
	{
		processReg();
	}
	else if (cmd == "unreg")
	{
		// DELETE /app/$app-name
		processUnReg();
	}
	else if (cmd == "view")
	{
		// VIEW /app/$app-name
		// VIEW /app-manager/applications
		processView();
	}
	else if (cmd == "config")
	{
		// VIEW /app-manager/config
		processConfig();
	}
	else if (cmd == "start")
	{
		// POST /app/$app-name?action=start
		processStartStop(true);
	}
	else if (cmd == "stop")
	{
		// POST /app/$app-name?action=stop
		processStartStop(false);
	}
}

void ArgumentParser::printMainHelp()
{
	std::cout << "Commands:" << std::endl;
	std::cout << "  view        List application[s]" << std::endl;
	std::cout << "  config      Display configuration infomration" << std::endl;
	std::cout << "  start       Start application[s]" << std::endl;
	std::cout << "  stop        Stop application[s]" << std::endl;
	std::cout << "  reg         Add a new application" << std::endl;
	std::cout << "  unreg       Remove an application" << std::endl;

	std::cout << std::endl;
	std::cout << "Run 'appc COMMAND --help' for more information on a command." << std::endl;

	std::cout << std::endl;
	std::cout << "Usage:  appmgc [COMMAND] [ARG...] [flags]" << std::endl;
}

void ArgumentParser::processReg()
{
	po::options_description desc("Register a new application:");
	desc.add_options()
		("name,n", po::value<std::string>(), "application name")
		("user,u", po::value<std::string>(), "application process running user name")
		("cmd,c", po::value<std::string>(), "full command line with arguments")
		("workdir,w", po::value<std::string>()->default_value("/tmp"), "working directory")
		("active,a", po::value<bool>()->default_value(true), "application active status (start is true, stop is false)")
		("time,t", po::value<std::string>(), "start date time for short running app (e.g., '2018-01-01 09:00:00')")
		("daily_start,s", po::value<std::string>(), "daily start time (e.g., '09:00:00')")
		("daily_end,d", po::value<std::string>(), "daily end time (e.g., '20:00:00')")
		("env,e", po::value<std::string>(), "environment variables (e.g., env1=value1:env2=value2)")
		("interval,i", po::value<int>(), "start interval seconds for short running app")
		("extraTime,x", po::value<int>(), "extra timeout for short running app,the value must less than interval  (default 0")
		("keep_running,k", po::value<bool>()->default_value(false), "monitor and keep running for short running app in start interval")
		("force,f", "force without confirm.")
		("help,h", "produce help message");

	moveForwardCommandLineVariables(desc);
	HELP_ARG_CHECK_WITH_RETURN;
	if (m_commandLineVariables.count("name") == 0
		|| m_commandLineVariables.count("user") == 0
		|| m_commandLineVariables.count("cmd") == 0
		|| m_commandLineVariables.count("workdir") == 0
		)
	{
		cout << desc << std::endl;
		return;
	}

	if (m_commandLineVariables.count("interval") > 0 && m_commandLineVariables.count("extraTime") >0)
	{
		if (m_commandLineVariables["interval"].as<int>() <= m_commandLineVariables["extraTime"].as<int>())
		{
			cout << "The extraTime seconds must less than interval." << std::endl;
			return;
		}
	}
	if (isAppExist(m_commandLineVariables["name"].as<string>()))
	{
		if (m_commandLineVariables.count("force") == 0)
		{
			if (!confirmInput("Application already exist, are you sure you want to update the application (y/n)?"))
			{
				return;
			}
		}
	}
	web::json::value jsobObj;
	jsobObj["name"] = web::json::value::string(m_commandLineVariables["name"].as<string>());
	jsobObj["command_line"] = web::json::value::string(m_commandLineVariables["cmd"].as<string>());
	jsobObj["run_as"] = web::json::value::string(m_commandLineVariables["user"].as<string>());
	jsobObj["working_dir"] = web::json::value::string(m_commandLineVariables["workdir"].as<string>());
	jsobObj["active"] = web::json::value::number(m_commandLineVariables["active"].as<bool>() ? 1 : 0);
	if (m_commandLineVariables.count("time") > 0)
	{
		jsobObj["start_time"] = web::json::value::string(m_commandLineVariables["time"].as<string>());
	}
	if (m_commandLineVariables.count("interval") > 0)
	{
		jsobObj["start_interval_seconds"] = web::json::value::number(m_commandLineVariables["interval"].as<int>());
	}

	if (m_commandLineVariables.count("extraTime") > 0)
	{
		jsobObj["start_interval_timeout"] = web::json::value::number(m_commandLineVariables["extraTime"].as<int>());
	}

	if (m_commandLineVariables.count("keep_running"))
	{
		jsobObj["keep_running"] = web::json::value::boolean(m_commandLineVariables["keep_running"].as<bool>());
	}

	if (m_commandLineVariables.count("daily_start") && m_commandLineVariables.count("daily_end"))
	{
		web::json::value objDailyLimitation = web::json::value::object();
		objDailyLimitation["daily_start"] = web::json::value::string(m_commandLineVariables["daily_start"].as<string>());
		objDailyLimitation["daily_end"] = web::json::value::string(m_commandLineVariables["daily_end"].as<string>());
		jsobObj["daily_limitation"] = objDailyLimitation;
	}


	if (m_commandLineVariables.count("env"))
	{
		std::vector<string> envs;
		Utility::splitString(m_commandLineVariables["env"].as<string>(), envs, ":");
		if (envs.size())
		{
			web::json::value objEnvs = web::json::value::object();
			std::for_each(envs.begin(), envs.end(), [&objEnvs](string env)
			{
				std::vector<string> envVec;
				Utility::splitString(env, envVec, "=");
				if (envVec.size() == 2)
				{
					objEnvs[GET_STRING_T(envVec.at(0))] = web::json::value::string(GET_STRING_T(envVec.at(1)));
				}
			});
			jsobObj["env"] = objEnvs;
		}
	}

	string restPath = string("/app/") + m_commandLineVariables["name"].as<string>();
	auto response = requestHttp(methods::PUT, restPath, jsobObj);
	RESPONSE_CHECK_WITH_RETURN;
	std::cout << GET_STD_STRING(response.extract_utf8string(true).get()) << std::endl;
}

void ArgumentParser::processUnReg()
{
	po::options_description desc("Unregister and remove an application:");
	desc.add_options()
		("help,h", "produce help message")
		("name,n", po::value<std::string>(), "remove application by name")
		("force,f", "force without confirm.");

	moveForwardCommandLineVariables(desc);
	HELP_ARG_CHECK_WITH_RETURN;

	if (isAppExist(m_commandLineVariables["name"].as<string>()))
	{
		if (m_commandLineVariables.count("force") == 0)
		{
			if (!confirmInput("Are you sure you want to remove the application (y/n)?"))
			{
				return;
			}
		}
		string restPath = string("/app/") + m_commandLineVariables["name"].as<string>();
		auto response = requestHttp(methods::DEL, restPath);
		RESPONSE_CHECK_WITH_RETURN;
		std::cout << GET_STD_STRING(response.extract_utf8string(true).get()) << std::endl;
	}
	else
	{
		throw std::invalid_argument("no such application");
	}
}

void ArgumentParser::processView()
{
	po::options_description desc("List application[s]:");
	desc.add_options()
		("help,h", "produce help message")
		("name,n", po::value<std::string>(), "view application by name.")
		;
	
	moveForwardCommandLineVariables(desc);
	HELP_ARG_CHECK_WITH_RETURN;

	if (m_commandLineVariables.empty())
	{
		string restPath = "/app-manager/applications";
		auto response = requestHttp(methods::GET, restPath);
		RESPONSE_CHECK_WITH_RETURN;
		printApps(response.extract_json(true).get());
	}
	else
	{
		if (m_commandLineVariables.count("name") > 0)
		{
			string restPath = string("/app/") + m_commandLineVariables["name"].as<string>();
			auto response = requestHttp(methods::GET, restPath);
			RESPONSE_CHECK_WITH_RETURN;
			auto arr = web::json::value::array(1);
			arr[0] = response.extract_json(true).get();
			printApps(arr);
		}
		else
		{
			std::cout << desc << std::endl;
		}
	}
}

void ArgumentParser::processConfig()
{
	string restPath = "/app-manager/config";
	auto bodyStr = requestHttp(methods::GET, restPath).extract_utf8string(true).get();
	std::cout << GET_STD_STRING(bodyStr) << std::endl;
}

void ArgumentParser::processStartStop(bool start)
{
	po::options_description desc("Start application:");
	desc.add_options()
		("help,h", "produce help message")
		("name,n", po::value<std::string>(), "start application by name. use 'all' to start all applications.")
		;
	
	moveForwardCommandLineVariables(desc);
	HELP_ARG_CHECK_WITH_RETURN;
	if (m_commandLineVariables.count("name") == 0) 
	{
		std::cout << desc << std::endl;
		return;
	}
	if (!isAppExist(m_commandLineVariables["name"].as<string>()))
	{
		throw std::invalid_argument("no such application");
	}

	std::map<string, string> query;
	query["action"] = start ? "start" : "stop";
	string restPath = string("/app/") + m_commandLineVariables["name"].as<string>();
	auto response = requestHttp(methods::POST, restPath, query);
	RESPONSE_CHECK_WITH_RETURN;
	std::cout << GET_STD_STRING(response.extract_utf8string(true).get()) << std::endl;
}

bool ArgumentParser::confirmInput(const char* msg)
{
	std::cout << msg << std::endl;
	std::string result;
	std::cin >> result;
	return result == "y";
}

http_response ArgumentParser::requestHttp(const method & mtd, string path)
{
	std::map<string, string> query;
	return std::move(requestHttp(mtd, path, query));
}

http_response ArgumentParser::requestHttp(const method & mtd, string path, web::json::value & body)
{
	std::map<string, string> query;
	return std::move(requestHttp(mtd, path, query, &body));
}

http_response ArgumentParser::requestHttp(const method & mtd, string path, std::map<string, string>& query, web::json::value * body)
{
	// Create http_client to send the request.
	auto restPath = (U("http://127.0.0.1:") + GET_STRING_T(std::to_string(m_listenPort)));
	http_client client(restPath);
	// Build request URI and start the request.
	uri_builder builder(GET_STRING_T(path));
	std::for_each(query.begin(), query.end(), [&builder](const std::pair<std::string, string>& pair)
	{
		builder.append_query(GET_STRING_T(pair.first), GET_STRING_T(pair.second));
	});
	
	http_request request(mtd);
	addHttpHeader(request);
	request.set_request_uri(builder.to_uri());
	if (body != nullptr)
	{
		request.set_body(*body);
	}
	http_response response = client.request(request).get();
	return std::move(response);
}

bool ArgumentParser::isAppExist(const std::string& appName)
{
	auto jsonValue = requestHttp(methods::GET, "/app-manager/applications").extract_json(true).get();
	auto arr = jsonValue.as_array();
	for (auto iter = arr.begin(); iter != arr.end(); iter++)
	{
		auto jobj = iter->as_object();
		if (GET_JSON_STR_VALUE(jobj, "name") == appName)
		{
			return true;
		}
	}
	return false;
}

void ArgumentParser::addHttpHeader(http_request & request)
{
	auto tokenPlain = Utility::convertTime2Str(std::chrono::system_clock::now());
	auto token = Utility::encode64(tokenPlain);
	request.headers().add("token", token);
}

void ArgumentParser::printApps(web::json::value json)
{
	// Title:
	std::cout << left;
	std::cout
		<< setw(3) << ("id")
		<< setw(6) << ("user")
		<< setw(7) << ("active")
		<< setw(6) << ("pid")
		<< setw(7) << ("return")
		<< setw(12) << ("name")
		<< ("command_line")
		<< endl;

	int index = 1;
	auto jsonArr = json.as_array();
	for_each(jsonArr.begin(), jsonArr.end(), [&index](web::json::value &x) {
		auto jobj = x.as_object();
		std::cout << setw(3) << index++;
		std::cout << setw(6) << GET_JSON_STR_VALUE(jobj, "run_as");
		std::cout << setw(7) << (GET_JSON_INT_VALUE(jobj, "active") == 1 ? "start" : "stop");
		std::cout << setw(6) << (GET_JSON_INT_VALUE(jobj, "pid") > 0 ? GET_JSON_INT_VALUE(jobj, "pid") : 0);
		std::cout << setw(7) << GET_JSON_INT_VALUE(jobj, "return");
		std::cout << setw(12) << GET_JSON_STR_VALUE(jobj, "name");
		std::cout << GET_JSON_STR_VALUE(jobj, "command_line");

		std::cout << std::endl;
	});
}

void ArgumentParser::moveForwardCommandLineVariables(po::options_description& desc)
{
	m_commandLineVariables.clear();
	std::vector<std::string> opts = po::collect_unrecognized(m_pasrsedOptions, po::include_positional);
	opts.erase(opts.begin());
	po::store(po::command_line_parser(opts).options(desc).run(), m_commandLineVariables);
	po::notify(m_commandLineVariables);
}
