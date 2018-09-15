
#ifndef _TIMER_ACTION_ONCE_
#define _TIMER_ACTION_ONCE_
#include <mutex>
#include <memory>
#include <boost/asio/deadline_timer.hpp>

class MyProcess;
//////////////////////////////////////////////////////////////////////////
// Each timer action assotiate to a short running application
// This timer only run one time
//////////////////////////////////////////////////////////////////////////
class TimerActionKill
{
public:
	TimerActionKill(std::shared_ptr<MyProcess> process, int bufferTimeSeconds);
	virtual ~TimerActionKill();

	// Callback function from timer thread
	virtual void onTimeOut(const boost::system::error_code& ec);

private:
	std::shared_ptr<MyProcess> m_process;
	std::shared_ptr<boost::asio::deadline_timer> m_timer;
};

#endif

