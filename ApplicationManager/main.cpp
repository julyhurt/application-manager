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

// whether use a dedicate thread for timer event
#define USE_SEPERATE_TIMER_THREAD false

static std::shared_ptr<Configuration> readConfiguration();
static std::string                    m_applicationPath;
static std::shared_ptr<RestHandler>   m_httpHandler;
static std::shared_ptr<boost::asio::deadline_timer> m_timer;
void monitorAllApps(const boost::system::error_code &ec);

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

		auto apps = config->getApps();
		auto process = Utility::getProcessList();
		for_each(apps.begin(), apps.end(), [&process](std::vector<std::shared_ptr<Application>>::reference p) {p->attach(process); });

		if (USE_SEPERATE_TIMER_THREAD)
		{
			TIMER.init();
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
		else
		{
			monitorAllApps(boost::system::error_code());
			TIMER.runTimerThread();
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
			throw std::runtime_error("can not open configuration file");
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

void monitorAllApps(const boost::system::error_code &ec)
{
	auto apps = Configuration::instance()->getApps();
	for (const auto& app : apps)
	{
		app->invoke();
	}
	// Set next timer
	if (nullptr == m_timer)
	{
		m_timer = std::make_shared<boost::asio::deadline_timer>(TIMER.getIO());
	}
	m_timer->expires_at(m_timer->expires_at() + boost::posix_time::seconds(Configuration::instance()->getScheduleInterval()));
	m_timer->async_wait(&monitorAllApps);
}
