
#include <mutex>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <jsoncpp/json/config.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>

#include "Configuration.h"
#include "cpprest/http_msg.h"
#include "Utility.h"
#include "ApplicationShortRun.h"
#include "ApplicationPeriodRun.h"

using namespace std;
#define DEFAULT_SCHEDULE_INTERVAL 5

std::shared_ptr<Configuration> Configuration::m_instance = nullptr;
Configuration::Configuration()
	:m_scheduleInterval(0), m_restListenPort(DEFAULT_REST_LISTEN_PORT)
{
	m_jsonFilePath = Utility::getSelfFullPath() + ".json";
	LOG(INFO) << "Configuration file <" << m_jsonFilePath << ">" << std::endl;
}


Configuration::~Configuration()
{
}

std::shared_ptr<Configuration> Configuration::instance()
{
	return m_instance;
}

std::shared_ptr<Configuration> Configuration::FromJson(const std::string& str)
{
	auto jval = web::json::value::parse(GET_STRING_T(str));
	web::json::object jobj = jval.as_object();
	auto config = std::make_shared<Configuration>();
	config->m_hostDescription = GET_JSON_STR_VALUE(jobj, "HostDescription");
	config->m_scheduleInterval = GET_JSON_INT_VALUE(jobj, "ScheduleIntervalSec");
	config->m_restListenPort = GET_JSON_INT_VALUE(jobj, "RestListenPort");
	if (config->m_scheduleInterval < 1 || config->m_scheduleInterval > 100)
	{
		// Use default value instead
		config->m_scheduleInterval = DEFAULT_SCHEDULE_INTERVAL;
		LOG(INFO) << "Default value <" << config->m_scheduleInterval << "> will by used for ScheduleIntervalSec" << std::endl;
	}
	if (config->m_restListenPort < 1000 || config->m_restListenPort > 65534)
	{
		config->m_restListenPort = DEFAULT_REST_LISTEN_PORT;
		LOG(INFO) << "Default value <" << config->m_restListenPort << "> will by used for RestListenPort" << std::endl;
	}

	auto& jArr = jobj.at(GET_STRING_T("Applications")).as_array();
	for (auto iterB = jArr.begin(); iterB != jArr.end(); iterB++)
	{
		auto jsonObj = iterB->as_object();
		std::shared_ptr<Application> app = config->parseApp(*iterB);
		app->dump();
		config->registerApp(app);
	}
	m_instance = config;
	return config;
}

web::json::value Configuration::AsJson(bool returnRuntimeInfo)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	web::json::value result = web::json::value::object();

	result[GET_STRING_T("HostDescription")] = web::json::value::string(GET_STRING_T(m_hostDescription));
	result[GET_STRING_T("RestListenPort")] = web::json::value::number(m_restListenPort);
	result[GET_STRING_T("ScheduleIntervalSec")] = web::json::value::number(m_scheduleInterval);

	auto arr = web::json::value::array(m_apps.size());
	for (size_t i = 0; i < m_apps.size(); ++i)
	{
		arr[i] = m_apps[i]->AsJson(returnRuntimeInfo);
	}
	result[GET_STRING_T("Applications")] = arr;
	return result;
}

std::vector<std::shared_ptr<Application>> Configuration::getApps()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return m_apps;
}

void Configuration::registerApp(std::shared_ptr<Application> app)
{
	const static char fname[] = "Configuration::registerApp() ";

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	for (size_t i = 0; i < m_apps.size(); i++)
	{
		if (m_apps[i]->getName() == app->getName())
		{
			LOG(INFO) << fname << "Application <" << app->getName() << "> already exist." << std::endl;
			return;
		}
	}
	m_apps.push_back(app);
	app->setIndex(m_apps.size());
}

int Configuration::getScheduleInterval()
{
	return m_scheduleInterval;
}

int Configuration::getRestListenPort()
{
	return m_restListenPort;
}

const utility::string_t Configuration::getConfigContentStr()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return this->AsJson(false).serialize();
}

web::json::value Configuration::getApplicationJson()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	// Build Json
	auto result = web::json::value::array(m_apps.size());
	for (size_t i = 0; i < m_apps.size(); ++i)
	{
		result[i] = m_apps[i]->AsJson(true);
	}
	return result;
}


std::vector<std::string> Configuration::stopAllApp()
{
	std::vector<std::string> stopedApps;
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	for_each(m_apps.begin(), m_apps.end(), [&stopedApps](std::shared_ptr<Application>& app) {
		app->stop();
		stopedApps.push_back(app->getName());
	});
	saveConfigToDisk();
	return stopedApps;
}

void Configuration::stopApp(const std::string& appName)
{
	getApp(appName)->stop();
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	saveConfigToDisk();
}
void Configuration::startApp(const std::string& appName)
{
	auto app = getApp(appName);
	app->start(app);
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	saveConfigToDisk();
}

std::vector<std::string> Configuration::startAllApp()
{
	std::vector<std::string> startedApps;
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	for (auto app : m_apps)
	{
		app->start(app);
		startedApps.push_back(app->getName());
	}
	saveConfigToDisk();
	return startedApps;
}

void Configuration::dump()
{
	const static char fname[] = "Configuration::dump() ";

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	LOG(INFO) << fname << "m_hostDescription:" << m_hostDescription << std::endl;
	LOG(INFO) << fname << "m_scheduleInterval:" << m_scheduleInterval << std::endl;
	LOG(INFO) << fname << "m_configContent:" << GET_STD_STRING(this->getConfigContentStr()) << std::endl;
	for (auto app : m_apps)
	{
		app->dump();
	}
}

std::shared_ptr<Application> Configuration::addApp(const web::json::value& jsonApp)
{
	std::shared_ptr<Application> app = parseApp(jsonApp);
	bool update = false;

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	for_each(m_apps.begin(), m_apps.end(), [&app, &update](std::shared_ptr<Application>& mapApp)
	{
		if (mapApp->getName() == app->getName())
		{	
			// Stop existing app and replace
			mapApp->stop();
			app->setIndex(mapApp->getIndex());
			mapApp = app;
			update = true;
		}
	});

	if (!update)
	{
		// Register app
		registerApp(app);
	}
	// Write to disk
	saveConfigToDisk();

	return app;
}

void Configuration::removeApp(const string& appName)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	// Update in-memory app
	int appIndex = 0;
	for (auto iterA = m_apps.begin(); iterA != m_apps.end();)
	{
		if ((*iterA)->getName() == appName)
		{
			(*iterA)->stop();
			iterA = m_apps.erase(iterA);
		}
		else
		{
			// Update index
			(*iterA)->setIndex(++appIndex);
			iterA++;
		}
	}

	// Write to disk
	saveConfigToDisk();
}

void Configuration::saveConfigToDisk()
{
	std::ofstream ofs(m_jsonFilePath, ios::trunc);
	if (ofs.is_open())
	{
		ofs << prettyJson(GET_STD_STRING(this->getConfigContentStr()));
		ofs.close();
	}
}

std::shared_ptr<Application> Configuration::parseApp(web::json::value jsonApp)
{
	auto jsonObj = jsonApp.as_object();
	std::shared_ptr<Application> app;

	if (GET_JSON_INT_VALUE(jsonObj, "start_interval_seconds") > 0)
	{
		// Consider as short running application
		std::shared_ptr<ApplicationShortRun> shortApp;
		if (GET_JSON_BOOL_VALUE(jsonObj, "keep_running") == true)
		{
			auto tmpApp = std::make_shared<ApplicationPeriodRun>();
			ApplicationPeriodRun::FromJson(tmpApp, jsonObj);
			shortApp = tmpApp;
		}
		else
		{
			shortApp = std::make_shared<ApplicationShortRun>();
			ApplicationShortRun::FromJson(shortApp, jsonObj);
			
		}
		shortApp->initTimer(shortApp);
		app = shortApp;
	}
	else
	{
		// Long running application
		app = std::make_shared<Application>();
		Application::FromJson(app, jsonObj);
	}
	return app;
}

std::string Configuration::prettyJson(const string & jsonStr)
{
	Json::Reader jsonReader;
	Json::Value root;
	if (jsonReader.parse(jsonStr, root, false))
	{
		return root.toStyledString();
	}
	else
	{
		string msg = "Failed to parse json : " + jsonStr;
		LOG(ERROR) << msg << std::endl;
		throw std::invalid_argument(msg);
	}
}

std::shared_ptr<Application> Configuration::getApp(const std::string & appName)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	for (auto app : m_apps)
	{
		if (app->getName() == appName)
		{
			return app;
		}
	}
	throw std::invalid_argument("No such application found");
}

std::shared_ptr<Application> Configuration::getApp(int index)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	for (auto app : m_apps)
	{
		if (app->getIndex() == index)
		{
			return app;
		}
	}
	throw std::invalid_argument("No such application found");
}
