
#ifndef CONFIGURATION
#define CONFIGURATION

#include <memory>
#include <vector>
#include <mutex>
#include "Application.h"

//////////////////////////////////////////////////////////////////////////
// All the operation functions to access appmg.json
//////////////////////////////////////////////////////////////////////////
class Configuration
{
public:
	Configuration();
	virtual ~Configuration();

	static std::shared_ptr<Configuration> instance();
	static std::shared_ptr<Configuration> FromJson(const std::string& str);
	web::json::value AsJson(bool returnRuntimeInfo);
	
	std::vector<std::shared_ptr<Application>> getApps();
	std::shared_ptr<Application> addApp(const web::json::value& jsonApp);
	void removeApp(const std::string& appName);
	void registerApp(std::shared_ptr<Application> app);
	int getScheduleInterval();
	int getRestListenPort();
	const utility::string_t getConfigContentStr();
	web::json::value getApplicationJson();
	std::vector<std::string> stopAllApp();
	std::vector<std::string> startAllApp();
	std::shared_ptr<Application> getApp(const std::string& appName);
	void stopApp(const std::string& appName);
	void startApp(const std::string& appName);

	static std::string prettyJson(const std::string & jsonStr);
	void dump();

private:
	void saveConfigToDisk();
	std::shared_ptr<Application> parseApp(web::json::value jsonApp);
	
private:
	std::vector<std::shared_ptr<Application>> m_apps;
	std::string m_hostDescription;
	int m_scheduleInterval;
	int m_restListenPort;

	std::recursive_mutex m_mutex;
	std::string m_jsonFilePath;

	static std::shared_ptr<Configuration> m_instance;
};

#endif