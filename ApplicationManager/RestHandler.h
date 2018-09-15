
#ifndef REST_HANDLER
#define REST_HANDLER

#include <iostream>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h> // HTTP server 

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

//////////////////////////////////////////////////////////////////////////
// REST service
//////////////////////////////////////////////////////////////////////////
class RestHandler
{
public:
	RestHandler(int port);
	virtual ~RestHandler();

protected:
	void open();
	void close();

private:
	void handle_get(http_request message);
	void handle_put(http_request message);
	void handle_post(http_request message);
	void handle_delete(http_request message);
	void handle_error(pplx::task<void>& t);
	bool checkToken(const std::string& token);
	std::string getToken(const http_request& message);
	std::shared_ptr<http_listener> m_listener;

};
#endif
