#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <glog/logging.h>
#include <ace/Init_ACE.h>

#include "RestHandler.h"
#include "Utility.h"
#include "Application.h"
#include "Configuration.h"
#include "Timer.h"

using namespace std;

static std::shared_ptr<Configuration> readConfiguration();
static std::string                    m_applicationPath;
static std::shared_ptr<RestHandler>   m_httpHandler;

int main(int argc, char * argv[])
{
	const static char fname[] = "main() ";
	try
	{
		ACE::init();
		Utility::initLogging(argv[0]);
		LOG(INFO) << fname;

		m_applicationPath = Utility::getSelfFullPath();
		auto config = readConfiguration();
		m_httpHandler = std::make_shared<RestHandler>(config->getRestListenPort());

		TIMER.init();

		auto apps = config->getApps();
		auto process = Utility::getProcessList();
		for_each(apps.begin(), apps.end(), [&process](std::vector<std::shared_ptr<Application>>::reference p) {p->attach(process); });
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(config->getScheduleInterval()));

			auto apps = config->getApps();

			for (const auto& app : apps)
			{
				app->invoke();
			}
		}
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << fname << "ERROR:" << e.what() << std::endl;
	}
	catch (...)
	{
		LOG(ERROR) << fname << "ERROR:" << "unknown exception" << std::endl;
	}
	LOG(ERROR) << "ERROR exited" << endl;
	TIMER.stop();
	ACE::fini();
	_exit(0);
	return 0;
}

std::shared_ptr<Configuration> readConfiguration()
{
	const static char fname[] = "readConfiguration() ";

	try
	{
		std::shared_ptr<Configuration> config;
		web::json::value jsonValue;
		string jsonPath = m_applicationPath + ".json";
		ifstream jsonFile(jsonPath);
		if (!jsonFile.is_open())
		{
			LOG(INFO) << "ERROR can not open configuration file <" << jsonPath << ">" << std::endl;
			config = std::make_shared<Configuration>();
		}
		else
		{
			std::string str((std::istreambuf_iterator<char>(jsonFile)), std::istreambuf_iterator<char>());
			jsonFile.close();

			LOG(INFO) << str << std::endl;
			config = Configuration::FromJson(str);
			config->dump();
		}

		return config;
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << fname << "ERROR:" << e.what() << std::endl;
		throw e;
	}
	catch (...)
	{
		LOG(ERROR) << fname << "ERROR:" << "unknown exception" << std::endl;
		throw;
	}
}

