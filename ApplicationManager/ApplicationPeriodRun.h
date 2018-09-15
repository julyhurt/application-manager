
#ifndef APPLICATION_DEFINITION_PERIOD_RUN
#define APPLICATION_DEFINITION_PERIOD_RUN

#include "ApplicationShortRun.h"

//////////////////////////////////////////////////////////////////////////
// Application that will start periodly but keep running all the time
//////////////////////////////////////////////////////////////////////////
class ApplicationPeriodRun :public ApplicationShortRun
{
public:
	ApplicationPeriodRun();
	virtual ~ApplicationPeriodRun();

	static void FromJson(std::shared_ptr<ApplicationPeriodRun>& app, const web::json::object& jobj);
	virtual web::json::value AsJson(bool returnRuntimeInfo);

	virtual void updatePid();

	virtual void dump();
};

#endif