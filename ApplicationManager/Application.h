
#ifndef APPLICATION_DEFINITION
#define APPLICATION_DEFINITION

#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <cpprest/json.h>
#include <ace/Process.h>
#include "DailyLimitation.h"
enum STATUS
{
	STOPPED = 0,
	NORMAL
};


class MyProcess : public ACE_Process
{
public:
	void attach(int pid);
};

//////////////////////////////////////////////////////////////////////////
// Application is one process used to manage
//////////////////////////////////////////////////////////////////////////
class Application
{
public:
	Application();
	virtual ~Application();
	std::string getName();
	std::string getCommandLine();
	bool isNormal();
	static void FromJson(std::shared_ptr<Application>& app, const web::json::object& obj);

	virtual void updatePid();
	void attach(std::map<std::string, int>& process);

	// Invoke immediately
	virtual void invokeNow(std::shared_ptr<Application>& self);
	// Invoke by scheduler
	virtual void invoke();
	
	virtual void stop();
	virtual void start(std::shared_ptr<Application>& self);

	virtual web::json::value AsJson(bool returnRuntimeInfo);
	virtual void dump();

	static void terminateProcess(std::shared_ptr<MyProcess>& process);
	virtual void spawnProcess();
	bool isInDailyTimeRange();

protected:
	STATUS m_active;
	std::string m_name;
	std::string m_commandLine;
	std::string m_user;
	std::string m_workdir;
	//the exit code of last instance
	int m_return;
	
	std::shared_ptr<MyProcess> m_process;
	int m_pid;
	std::recursive_mutex m_mutex;
	std::shared_ptr<DailyLimitation> m_dailyLimit;
	std::map<std::string, std::string> m_envMap;
};

#endif 