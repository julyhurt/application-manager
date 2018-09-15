
#ifndef APPLICATION_DEFINITION_SOHORT_RUN
#define APPLICATION_DEFINITION_SOHORT_RUN

#include <memory>
#include <string>
#include <map>
#include <mutex>
#include "Application.h"
#include "TimerAction.h"
#include "TimerActionKill.h"

//////////////////////////////////////////////////////////////////////////
// Application that will start periodly
//////////////////////////////////////////////////////////////////////////
class ApplicationShortRun : public Application
{
public:
	ApplicationShortRun();
	virtual ~ApplicationShortRun();

	static void FromJson(std::shared_ptr<ApplicationShortRun>& app, const web::json::object& jobj);

	virtual void invoke();
	virtual void invokeNow(std::shared_ptr<Application>& self);
	virtual void start(std::shared_ptr<Application>& self);
	virtual web::json::value AsJson(bool returnRuntimeInfo);
	void initTimer(std::shared_ptr<ApplicationShortRun> app);
	virtual void updatePid();
	int getStartInterval();
	std::chrono::system_clock::time_point getStartTime();
	virtual void dump();
protected:
	std::chrono::system_clock::time_point m_startTime;
	int m_startInterval;
	int m_bufferTime;
	TimerAction* m_timer;
	std::shared_ptr<MyProcess> m_bufferProcess;
};

#endif 