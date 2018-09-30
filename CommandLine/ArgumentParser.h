#ifndef ARGUMENT_PARSER
#define ARGUMENT_PARSER
#include <string>
#include <iomanip>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <boost/program_options.hpp>

using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using namespace std;
namespace po = boost::program_options;

class ArgumentParser
{
public:
	ArgumentParser(int argc, char* argv[], int listenPort);
	virtual ~ArgumentParser();

	void parse();

private:
	void printMainHelp();
	void processReg();
	void processUnReg();
	void processView();
	void processConfig();
	void processStartStop(bool start);

	bool confirmInput(const char* msg);
	http_response requestHttp(const method & mtd, string path);
	http_response requestHttp(const method & mtd, string path, web::json::value& body);
	http_response requestHttp(const method & mtd, string path, std::map<string,string>& query, web::json::value * body = nullptr);
	bool isAppExist(const std::string& appName);
	void addHttpHeader(http_request& request);

private:
	void printApps(web::json::value json);
	void moveForwardCommandLineVariables(po::options_description& desc);

private:
	po::variables_map m_commandLineVariables;
	std::vector<po::option> m_pasrsedOptions;
	int m_listenPort;
};
#endif

