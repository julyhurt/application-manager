
#ifndef DAILY_LIMITATION
#define DAILY_LIMITATION
#include <string>
#include <chrono>
#include <memory>
#include <cpprest/json.h>

//////////////////////////////////////////////////////////////////////////
// Define the valid time range in one day
//////////////////////////////////////////////////////////////////////////
class DailyLimitation
{
public:
	DailyLimitation();
	virtual ~DailyLimitation();
	void dump();

	virtual web::json::value AsJson();
	static std::shared_ptr<DailyLimitation> FromJson(const web::json::object& obj);

	std::chrono::system_clock::time_point m_startTime;
	std::chrono::system_clock::time_point m_endTime;
};

#endif