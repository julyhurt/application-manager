#include "Timer.h"
#include <boost/thread/thread.hpp>

#include "Utility.h"

Timer::Timer()
	:m_running(false)
{
}


Timer::~Timer()
{
	if (m_running)
	{
		stop();
	}
}

void Timer::init()
{
	m_timerThread = std::make_shared<boost::thread>(boost::bind(&Timer::timerThread, &Timer::get_mutable_instance()));
}

void Timer::stop()
{
	m_ioWork = nullptr;
	m_io.stop();
	if (m_timerThread != nullptr && m_running)
	{
		m_timerThread->join();
	}
}

void Timer::timerThread()
{
	LOG(INFO) << "Timer thread started <" << Utility::getThreadId() << ">." << std::endl;
	m_running = true;
	// use work to avoid io_service exit when no job
	m_ioWork.reset(new boost::asio::io_service::work(m_io));
	m_io.run();
	m_running = false;
	LOG(WARNING) << "Timer thread exited <" << Utility::getThreadId() << ">." << std::endl;
}

boost::asio::io_service & Timer::getIO()
{
	return m_io;
}
