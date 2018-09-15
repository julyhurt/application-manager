
#include "ApplicationPeriodRun.h"
#include "Configuration.h"
#include "Utility.h"

ApplicationPeriodRun::ApplicationPeriodRun()
{
	const static char fname[] = "ApplicationPeriodRun::ApplicationPeriodRun() ";
	LOG(INFO) << fname << "Entered." << std::endl;
}


ApplicationPeriodRun::~ApplicationPeriodRun()
{
	const static char fname[] = "ApplicationPeriodRun::~ApplicationPeriodRun() ";
	LOG(INFO) << fname << "Entered." << std::endl;
}

void ApplicationPeriodRun::updatePid()
{
	ApplicationShortRun::updatePid();

	auto app = Configuration::instance()->getApp(this->getName());
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (!m_process->running() && this->isNormal() && std::chrono::system_clock::now() > getStartTime())
	{
		this->invokeNow(app);
	}
}

void ApplicationPeriodRun::FromJson(std::shared_ptr<ApplicationPeriodRun>& app, const web::json::object & jobj)
{
	std::shared_ptr<ApplicationShortRun> fatherApp = app;
	ApplicationShortRun::FromJson(fatherApp, jobj);
}

web::json::value ApplicationPeriodRun::AsJson(bool returnRuntimeInfo)
{
	const static char fname[] = "ApplicationPeriodRun::AsJson() ";
	LOG(INFO) << fname << std::endl;

	web::json::value result = ApplicationShortRun::AsJson(returnRuntimeInfo);
	result[GET_STRING_T("keep_running")] = web::json::value::boolean(true);
	return result;
}

void ApplicationPeriodRun::dump()
{
	const static char fname[] = "ApplicationPeriodRun::dump() ";

	ApplicationShortRun::dump();
	LOG(INFO) << fname << "keep_running:" << "true" << std::endl;
}


