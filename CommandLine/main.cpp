
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <boost/program_options.hpp>

#include "../ApplicationManager/Utility.h"

using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using namespace std;
namespace po = boost::program_options;

enum COMMAND
{
	CMD_QUERY = 100,
	CMD_CONFIG,
	CMD_START,
	CMD_STOP,
	CMD_REG,
	CMD_UNREG
};

int preParseArgs(int argc, char* argv[], boost::program_options::variables_map& vmOut, int& cmdOut);
void printHelp();
int createToken(std::string& token);
bool appExist(const std::string& appName, const std::string& queryUrl);

int main(int argc, char * argv[])
{
	try
	{
		po::variables_map vm;
		int cmd = 0;
		int result = preParseArgs(argc, argv, vm, cmd);
		if (result != 0)
		{
			return result;
		}

		// Get listen port
		int port = DEFAULT_REST_LISTEN_PORT;
		web::json::value jsonValue;
		auto configPath = Utility::getSelfFullPath();
		configPath[configPath.length()] = '\0';
		ifstream jsonFile(configPath + ".json");
		if (jsonFile)
		{
			std::string str((std::istreambuf_iterator<char>(jsonFile)), std::istreambuf_iterator<char>());
			jsonValue = web::json::value::parse(GET_STRING_T(str));
			auto p = GET_JSON_INT_VALUE(jsonValue.as_object(), "RestListenPort");
			if (p > 1000 && p < 65534)
			{
				port = p;
			}
		}

		// Get sub argument
		string restPath;
		if (cmd == CMD_CONFIG)
		{
			restPath = "config";
			// Create http_client to send the request.
			http_client client(U("http://127.0.0.1:") + GET_STRING_T(std::to_string(port)) + U("/") + GET_STRING_T(restPath));
			http_response response = client.request(methods::GET).get();

			auto bodyStr = response.extract_utf8string(true).get();
			cout << "--------------------------------------------------------------------" << endl;
			std::cout << GET_STD_STRING(bodyStr) << std::endl;
			cout << "--------------------------------------------------------------------" << endl;
		}

		if (cmd == CMD_QUERY)
		{
			restPath = "view";
			// Create http_client to send the request.
			http_client client(U("http://127.0.0.1:") + GET_STRING_T(std::to_string(port)) + U("/") + GET_STRING_T(restPath));
			http_response response = client.request(methods::GET).get();


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
			cout << "--------------------------------------------------------------------" << endl;

			auto jsonValue = response.extract_json(true).get();
			auto arr = jsonValue.as_array();
			for_each(arr.begin(), arr.end(), [](web::json::value &x) {
				auto jobj = x.as_object();
				std::cout << setw(3) << GET_JSON_INT_VALUE(jobj, "index");
				std::cout << setw(6) << GET_JSON_STR_VALUE(jobj, "run_as");
				std::cout << setw(7) << (GET_JSON_INT_VALUE(jobj, "active") == 1 ? "start" : "stop");
				std::cout << setw(6) << (GET_JSON_INT_VALUE(jobj, "pid") > 0 ? GET_JSON_INT_VALUE(jobj, "pid") : 0);
				std::cout << setw(7) << GET_JSON_INT_VALUE(jobj, "return");
				std::cout << setw(12) << GET_JSON_STR_VALUE(jobj, "name");
				std::cout << GET_JSON_STR_VALUE(jobj, "command_line");

				std::cout << std::endl;
			});
			cout << "--------------------------------------------------------------------" << endl;
		}

		if (cmd == CMD_START || cmd == CMD_STOP || cmd == CMD_UNREG || cmd == CMD_REG)
		{
			string appid;
			string appname;
			method methodReq;
			string msgAction;
			switch (cmd)
			{
			case CMD_START:
				restPath = "start";
				methodReq = methods::POST;
				msgAction = "Start application";
				break;
			case CMD_STOP:
				restPath = "stop";
				methodReq = methods::POST;
				msgAction = "Stop application";
				break;
			case CMD_UNREG:
				restPath = "unreg";
				methodReq = methods::DEL;
				msgAction = "Unregister application";
				break;
			case CMD_REG:
				restPath = "reg";
				methodReq = methods::PUT;
				msgAction = "Register application";
				break;
			default:
				break;
			}

			web::json::value obj;
			if (cmd == CMD_REG)
			{
				obj["name"] = web::json::value::string(vm["name"].as<string>());
				obj["command_line"] = web::json::value::string(vm["cmd"].as<string>());
				obj["run_as"] = web::json::value::string(vm["user"].as<string>());
				obj["working_dir"] = web::json::value::string(vm["workdir"].as<string>());
				obj["active"] = web::json::value::number(vm["active"].as<int>());
				if (vm.count("time") > 0)
				{
					obj["start_time"] = web::json::value::string(vm["time"].as<string>());
				}
				if (vm.count("interval") > 0)
				{
					obj["start_interval_seconds"] = web::json::value::number(vm["interval"].as<int>());
				}

				if (vm.count("extraTime") > 0)
				{
					obj["start_interval_timeout"] = web::json::value::number(vm["extraTime"].as<int>());
				}

				if (vm.count("keep_running"))
				{
					obj["keep_running"] = web::json::value::boolean(vm["keep_running"].as<bool>());
				}

				if (vm.count("daily_start") && vm.count("daily_end"))
				{
					web::json::value objDailyLimitation = web::json::value::object();
					objDailyLimitation["daily_start"] = web::json::value::string(vm["daily_start"].as<string>());
					objDailyLimitation["daily_end"] = web::json::value::string(vm["daily_end"].as<string>());
					obj["daily_limitation"] = objDailyLimitation;
				}


				if (vm.count("env"))
				{
					std::vector<string> envs;
					Utility::splitString(vm["env"].as<string>(), envs, ":");
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
						obj["env"] = objEnvs;
					}
				}


				const std::string queryUrl = "http://127.0.0.1:" + std::to_string(port) + "/view";
				if (vm.count("force") == 0 && appExist(vm["name"].as<string>(), queryUrl))
				{
					std::cout << "Application already exist.Are you sure you want to update the application (y/n)?" << std::endl;
					std::string result;
					std::cin >> result;
					if (result != "y")
					{
						return 2;
					}
				}
			}
			else
			{
				if (vm.count("index") > 0)
				{
					obj["index"] = web::json::value::number(vm["index"].as<int>());
				}
				else if (vm.count("name") > 0)
				{
					obj["name"] = web::json::value::string(vm["name"].as<string>());
				}
			}
			// Create http_client to send the request.
			http_client client(U("http://127.0.0.1:") + GET_STRING_T(std::to_string(port)));
			http_request request(methodReq);
			std::string token;
			createToken(token);
			request.headers().add("token", token);
			request.set_request_uri(web::uri(GET_STRING_T(restPath)));
			request.set_body(obj);
			http_response response = client.request(request).get();
			auto bodyStr = response.extract_utf8string(true).get();
			if (response.status_code() == status_codes::OK)
			{
				msgAction.append(" successful:");
			}
			else
			{
				msgAction.append(" failed:");
			}
			std::cout << msgAction << std::endl;
			cout << "--------------------------------------------------------------------" << endl;
			std::cout << GET_STD_STRING(bodyStr) << std::endl;
			cout << "--------------------------------------------------------------------" << endl;

		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR:" << e.what() << endl;
	}
	return 0;
}



int preParseArgs(int argc, char* argv[], po::variables_map& vmOut, int& cmdOut)
{
	po::options_description global("Global options");
	global.add_options()
		("help,h", "produce help message")
		("command", po::value<std::string>(), "command to execute")
		("subargs", po::value<std::vector<std::string> >(), "arguments for command");

	po::positional_options_description pos;
	pos.add("command", 1).
		add("subargs", -1);

	po::variables_map vm;
	po::parsed_options parsed = po::command_line_parser(argc, argv).options(global).positional(pos).allow_unregistered().run();
	po::store(parsed, vm);
	po::notify(vm);
	if (vm.empty())
	{
		printHelp();
		return -1;
	}

	std::string cmd = vm["command"].as<std::string>();
	if (cmd == "reg")
	{
		cmdOut = CMD_REG;
		po::options_description desc("Add a new application:");
		desc.add_options()
			("help,h", "produce help message")
			("name,n", po::value<std::string>(), "application name")
			("user,u", po::value<std::string>(), "application running user name")
			("cmd,c", po::value<std::string>(), "full command line with arguments")
			("workdir,w", po::value<std::string>(), "working directory")
			("active,a", po::value<int>()->default_value(1), "application active status: 0 stop,1 start")
			("time,t", po::value<std::string>(), "start date time for short running app, E.g '2018-01-01 09:00:00'")
			("daily_start,s", po::value<std::string>(), "daily start time, E.g '09:00:00'")
			("daily_end,d", po::value<std::string>(), "daily end time, E.g '20:00:00'")
			("env,e", po::value<std::string>(), "environment variables, E.g env1=value1:env2=value2")
			("interval,i", po::value<int>(), "start interval seconds for short running app")
			("extraTime,x", po::value<int>()->default_value(0), "extra timeout for short running app,the value must less than interval")
			("keep_running,k", po::value<bool>()->default_value(false), "monitor and keep running for short running app in start interval")
			("force,f", "force without confirm.");

		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());
		po::store(po::command_line_parser(opts).options(desc).run(), vm);
		po::notify(vm);
		if (vm.count("help")
			|| vm.size() < 7
			|| vm.count("name") == 0
			|| vm.count("user") == 0
			|| vm.count("cmd") == 0
			|| vm.count("workdir") == 0
			|| vm.count("active") == 0)
		{
			cout << desc << endl;
			return 1;
		}

		if (vm.count("interval") > 0 && vm.count("extraTime") >0)
		{
			if (vm["interval"].as<int>() <= vm["extraTime"].as<int>())
			{
				cerr << "The extraTime seconds must less than interval." << endl;
				return 1;
			}
		}
		vmOut = vm;

	}
	else if (cmd == "unreg")
	{
		cmdOut = CMD_UNREG;
		po::options_description desc("Unregister and remove an application:");
		desc.add_options()
			("help,h", "produce help message")
			("name,n", po::value<std::string>(), "remove application by name")
			("index,i", po::value<int>(), "remove application by index")
			("force,f", "force without confirm.");
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());
		po::store(po::command_line_parser(opts).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("help") || vm.size() < 3) {
			cout << desc << endl;
			return 1;
		}
		if (vm.count("force") == 0)
		{
			std::cout << "Are you sure you want to remove the application (y/n)?" << std::endl;
			std::string result;
			std::cin >> result;
			if (result != "y")
			{
				return 2;
			}
		}
		vmOut = vm;
	}
	else if (cmd == "view")
	{
		cmdOut = CMD_QUERY;
		po::options_description desc("List all application running status:");
		desc.add_options()
			("help,h", "produce help message");
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());
		po::store(po::command_line_parser(opts).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("help"))
		{
			cout << desc << endl;
			return 1;
		}
		vmOut = vm;

	}
	else if (cmd == "config")
	{
		cmdOut = CMD_CONFIG;
		po::options_description desc("Display configuration infomration:");
		desc.add_options()
			("help,h", "produce help message");
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());
		po::store(po::command_line_parser(opts).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("help"))
		{
			cout << desc << endl;
			return 1;
		}
		vmOut = vm;
	}
	else if (cmd == "stop")
	{
		cmdOut = CMD_STOP;
		po::options_description desc("Stop application:");
		desc.add_options()
			("help,h", "produce help message")
			("name,n", po::value<std::string>(), "stop an application by name. use 'all' to stop all applications.")
			("index,i", po::value<int>(), "stop an application by index.")
			("force,f", "force without confirm.");
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());
		po::store(po::command_line_parser(opts).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("help") || (vm.count("name") == 0 && vm.count("index") == 0))
		{
			cout << desc << endl;
			return 1;
		}
		if (vm.count("force") == 0)
		{
			std::cout << "Are you sure you want to stop the application (y/n)?" << std::endl;
			std::string result;
			std::cin >> result;
			if (result != "y")
			{
				return 2;
			}
		}
		vmOut = vm;

	}
	else if (cmd == "start")
	{
		cmdOut = CMD_START;
		po::options_description desc("Start application:");
		desc.add_options()
			("help,h", "produce help message")
			("name,n", po::value<std::string>(), "start application by name. use 'all' to start all applications.")
			("index,i", po::value<int>(), "start application by index.");
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());
		po::store(po::command_line_parser(opts).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("help") || (vm.count("name") == 0 && vm.count("index") == 0)) {
			cerr << desc << endl;
			return 1;
		}
		vmOut = vm;
	}
	else
	{
		printHelp();
	}

	if (vm.count("help") > 0)
	{
		printHelp();
		return -1;
	}
	return 0;
}

void printHelp()
{

	cerr << "Commands:" << endl;
	cerr << "  view        List all applications" << endl;
	cerr << "  config      Display configuration infomration" << endl;
	cerr << "  start       Start application[s]" << endl;
	cerr << "  stop        Stop application[s]" << endl;
	cerr << "  reg         Add a new application" << endl;
	cerr << "  unreg       Remove an application" << endl;

	cerr << endl;
	cerr << "Run 'appmgc COMMAND --help' for more information on a command." << endl;

	cerr << endl;
	cerr << "Usage:  appmgc [COMMAND] [ARG...] [flags]" << endl;
}

int createToken(std::string& token)
{
	auto now = std::time(nullptr);
	auto tm = *std::localtime(&now);
	std::stringstream ss;
	ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
	std::string tokenPlain = ss.str();
	return tokenPlain.length();
}

bool appExist(const std::string& appName, const std::string& queryUrl)
{
	http_client client(GET_STRING_T(queryUrl));
	http_response response = client.request(methods::GET).get();
	auto jsonValue = response.extract_json(true).get();
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