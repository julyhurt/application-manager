#include <ctime>
#include <boost/date_time/local_time/local_time.hpp>
#include "DailyLimitation.h"
#include "Utility.h"

DailyLimitation::DailyLimitation()
{
}


DailyLimitation::~DailyLimitation()
{
}

void DailyLimitation::dump()
{
	const static char fname[] = "DailyLimitation::dump() ";

	LOG(INFO) << fname << "m_startTime:" << Utility::convertDayTime2Str(m_startTime) << std::endl;
	LOG(INFO) << fname << "m_endTime:" << Utility::convertDayTime2Str(m_endTime) << std::endl;
}

web::json::value DailyLimitation::AsJson()
{
	web::json::value result = web::json::value::object();

	result[GET_STRING_T("daily_start")] = web::json::value::string(GET_STRING_T(Utility::convertDayTime2Str(m_startTime)));
	result[GET_STRING_T("daily_end")] = web::json::value::string(GET_STRING_T(Utility::convertDayTime2Str(m_endTime)));
	return result;
}

std::shared_ptr<DailyLimitation> DailyLimitation::FromJson(const web::json::object & jobj)
{
	std::shared_ptr<DailyLimitation> result;
	if (!jobj.empty())
	{
		if (!(HAS_JSON_FIELD(jobj, "daily_start") && HAS_JSON_FIELD(jobj, "daily_end")))
		{
			throw std::invalid_argument("should both have daily_start and daily_end parameter");
		}
		result = std::make_shared<DailyLimitation>();
		result->m_startTime = Utility::convertStr2DayTime(GET_JSON_STR_VALUE(jobj, "daily_start"));
		result->m_endTime = Utility::convertStr2DayTime(GET_JSON_STR_VALUE(jobj, "daily_end"));
	}
	return result;
}

std::chrono::system_clock::time_point DailyLimitation::convert2tzTime(std::chrono::system_clock::time_point& origin_time, std::string& posixTimezone)
{
	const static char fname[] = "ApplicationShortRun::convert2tzTime() ";


	// https://www.boost.org/doc/libs/1_58_0/doc/html/date_time/examples.html#date_time.examples.simple_time_zone
	// 1. Get original time with target time zone
	//std::chrono::system_clock::time_point origin_time = std::chrono::system_clock::now();
	std::time_t origin_timet = std::chrono::system_clock::to_time_t(origin_time);
	tm* origin_localtime = std::localtime(&origin_timet);
	boost::gregorian::date origin_date(origin_localtime->tm_year + 1900, origin_localtime->tm_mon + 1, origin_localtime->tm_mday);
	boost::posix_time::time_duration origin_time_duration(origin_localtime->tm_hour, origin_localtime->tm_min, origin_localtime->tm_sec);
	// https://stackoverflow.com/questions/36411557/how-to-make-sure-that-posix-time-zone-constructor-wont-crash-when-invalid-strin
	boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone(posixTimezone));
	boost::local_time::local_date_time origin_local_time(origin_date, origin_time_duration, zone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
	//std::cout << origin_local_time.to_string() << std::endl;

	// 2. Convert time to current time zone
	boost::local_time::time_zone_ptr dst_tz(new boost::local_time::posix_time_zone(Utility::getSystemPosixTimeZone()));
	auto target_local_time = origin_local_time.local_time_in(dst_tz);
	//std::cout << target_time.to_string() << std::endl;

	// 3. Convert local_date_time to time_point
	// https://stackoverflow.com/questions/4910373/interoperability-between-boostdate-time-and-stdchrono
	auto timepoint = std::chrono::system_clock::from_time_t(boost::posix_time::to_time_t(target_local_time.utc_time()));

	LOG(INFO) << fname << "time <" << Utility::convertTime2Str(origin_time) << "> with timezone <" << posixTimezone
		<< "> was convert system time <" << Utility::convertTime2Str(timepoint) << "> from timezone <" << Utility::getSystemPosixTimeZone() << ">." << std::endl;
	return std::move(timepoint);
}