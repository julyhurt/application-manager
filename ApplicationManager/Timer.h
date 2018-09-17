
#ifndef _TIMER_
#define _TIMER_

#include <boost/serialization/singleton.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

//////////////////////////////////////////////////////////////////////////
// Timer IO service and thread
//////////////////////////////////////////////////////////////////////////
class Timer : public boost::serialization::singleton<Timer>
{
public:
	Timer();
	virtual ~Timer();

	void init();
	void stop();
	boost::asio::io_service& getIO();

	void runTimerThread();

private:
	boost::asio::io_service m_io;
	std::shared_ptr<boost::asio::io_service::work> m_ioWork;
	std::shared_ptr<boost::thread> m_timerThread;
	bool m_running;
};


#define TIMER Timer::get_mutable_instance()

#endif