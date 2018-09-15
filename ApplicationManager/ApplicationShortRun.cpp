
#include <mutex>
#include <iomanip>
#include <ctime>
#include <time.h>
#include <chrono>
#include <ace/Time_Value.h>
#include "ApplicationShortRun.h"
#include "Utility.h"

ApplicationShortRun::ApplicationShortRun()
	:m_startInterval(0), m_bufferTime(0), m_timer(NULL)
{
	const static char fname[] = "ApplicationShortRun::ApplicationShortRun() ";
	LOG(INFO) << fname << "Entered." << std::endl;
}


ApplicationShortRun::~ApplicationShortRun()
{
	const static char fname[] = "ApplicationShortRun::~ApplicationShortRun() ";
	LOG(INFO) << fname << "Entered." << std::endl;
	if (m_timer != NULL) m_timer->cancelTimer();
}

void ApplicationShortRun::FromJson(std::shared_ptr<ApplicationShortRun>& app, const web::json::object& jobj)
{
	std::shared_ptr<Application> fatherApp = app;
	Application::FromJson(fatherApp, jobj);
	app->m_startInterval = GET_JSON_INT_VALUE(jobj, "start_interval_seconds");
	app->m_startTime = Utility::convertStr2Time(GET_JSON_STR_VALUE(jobj, "start_time"));
	if (HAS_JSON_FIELD(jobj, "start_interval_timeout"))
	{
		app->m_bufferTime = GET_JSON_INT_VALUE(jobj, "start_interval_timeout");
	}
}


void ApplicationShortRun::updatePid()
{
	Application::updatePid();
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (nullptr != m_bufferProcess && m_bufferProcess->running())
	{
		ACE_Time_Value tv;
		tv.msec(5);
		int ret = m_bufferProcess->wait(tv);
		if (ret > 0)
		{
			m_return = m_bufferProcess->return_value();
		}
	}
}

void ApplicationShortRun::invoke()
{
	// Only refresh Pid for short running
	updatePid();
}

void ApplicationShortRun::invokeNow(std::shared_ptr<Application>& self)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_bufferTime > 0)
	{
		m_bufferProcess = m_process;
		auto bufferTimer = new TimerActionKill(m_bufferProcess, m_bufferTime);
	}
	else
	{
		terminateProcess(m_process);
	}
	// Spawn new process
	m_process = std::make_shared<MyProcess>();
	if (this->isInDailyTimeRange())
	{
		spawnProcess();
	}
}

web::json::value ApplicationShortRun::AsJson(bool returnRuntimeInfo)
{
	const static char fname[] = "ApplicationShortRun::AsJson() ";
	LOG(INFO) << fname << std::endl;
	web::json::value result = Application::AsJson(returnRuntimeInfo);

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	result[GET_STRING_T("start_time")] = web::json::value::string(GET_STRING_T(Utility::convertTime2Str(m_startTime)));
	result[GET_STRING_T("start_interval_seconds")] = web::json::value::number(m_startInterval);
	result[GET_STRING_T("start_interval_timeout")] = web::json::value::number(m_bufferTime);
	return result;
}

void ApplicationShortRun::start(std::shared_ptr<Application>& self)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_active == STOPPED)
	{
		m_active = NORMAL;
		initTimer(std::dynamic_pointer_cast<ApplicationShortRun>(self));
	}
}

void ApplicationShortRun::initTimer(std::shared_ptr<ApplicationShortRun> app)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_timer != NULL) m_timer->cancelTimer();
	m_timer = new TimerAction(app);
}

int ApplicationShortRun::getStartInterval()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return m_startInterval;
}

std::chrono::system_clock::time_point ApplicationShortRun::getStartTime()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return m_startTime;
}

void ApplicationShortRun::dump()
{
	const static char fname[] = "ApplicationShortRun::dump() ";

	Application::dump();
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	LOG(INFO) << fname << "m_startTime:" << Utility::convertTime2Str(m_startTime) << std::endl;
	LOG(INFO) << fname << "m_startInterval:" << m_startInterval << std::endl;
	LOG(INFO) << fname << "m_bufferTime:" << m_bufferTime << std::endl;
}
