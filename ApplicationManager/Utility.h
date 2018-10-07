
#ifndef UTILITY
#define UTILITY

#include <string>
#include <map>
#include <chrono>
#include <cpprest/json.h>
#include <glog/logging.h>

#define GET_STRING_T(sstr) utility::conversions::to_string_t(std::string(sstr))
#define GET_STD_STRING(sstr)  utility::conversions::to_utf8string(sstr)

// Get attribute from json Object
#define GET_JSON_STR_VALUE(jsonObj, key) Utility::stdStringTrim(GET_STD_STRING(GET_JSON_STR_T_VALUE(jsonObj, key)))
#define GET_JSON_STR_T_VALUE(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? GET_STRING_T("") : jsonObj.at(GET_STRING_T(key)).as_string())
#define GET_JSON_INT_VALUE(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? 0 : jsonObj.at(GET_STRING_T(key)).as_integer())
#define GET_JSON_BOOL_VALUE(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? false : jsonObj.at(GET_STRING_T(key)).as_bool())
#define HAS_JSON_FIELD(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? false : true)

#define DEFAULT_REST_LISTEN_PORT 6060

//////////////////////////////////////////////////////////////////////////
// All common functions
//////////////////////////////////////////////////////////////////////////
class Utility
{
public:
	Utility();
	virtual ~Utility();

	// OS related
	static std::map<std::string, int> getProcessList();
	static std::string getSelfFullPath();

	// String related
	static bool isNumber(std::string s);
	static void stringReplace(std::string &strBase, const std::string strSrc, const std::string strDst);
	static std::string stdStringTrim(const std::string &str);
	static void splitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
	static bool startWith(const std::string& str, std::string head);

	static void initLogging(const char* arg0);

	static unsigned long long getThreadId();
	static bool getUid(std::string userName, long& uid, long& groupid);

	static void getEnvironmentSize(const std::map<std::string, std::string> &envMap, int &totalEnvSize, int &totalEnvArgs);

	// %Y-%m-%d %H:%M:%S
	static std::chrono::system_clock::time_point convertStr2Time(const std::string & strTime);
	static std::string convertTime2Str(const std::chrono::system_clock::time_point & time);
	// %H:%M:%S
	static std::chrono::system_clock::time_point convertStr2DayTime(const std::string & strTime);
	static std::string convertDayTime2Str(const std::chrono::system_clock::time_point & time);
	// Timezone
	static std::string getSystemPosixTimeZone();

	// Base64
	static std::string encode64(const std::string &val);
	static std::string decode64(const std::string &val);
};

#endif