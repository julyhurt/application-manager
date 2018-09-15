#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <chrono>

#include "TimerActionKill.h"
#include "Utility.h"
#include "Timer.h"
#include "Application.h"

TimerActionKill::TimerActionKill(std::shared_ptr<MyProcess> process, int bufferTimeSeconds)
	:m_process(process)
{
	const static char fname[] = "TimerActionKill::TimerActionKill() ";
	LOG(INFO) << fname << "Entered." << std::endl;
		
	m_timer = std::make_shared<boost::asio::deadline_timer>(TIMER.getIO(), boost::posix_time::seconds(bufferTimeSeconds));
	m_timer->async_wait(boost::bind(&TimerActionKill::onTimeOut, this, boost::asio::placeholders::error));

	LOG(INFO) << "Timer will sleep <" << bufferTimeSeconds << "> seconds for process <" << process->getpid() << "> to stop." << std::endl;
}

TimerActionKill::~TimerActionKill()
{
	const static char fname[] = "TimerActionKill::~TimerActionKill() ";
	LOG(INFO) << fname << "Entered." << std::endl;
}

void TimerActionKill::onTimeOut(const boost::system::error_code& ec)
{
	const static char fname[] = "TimerActionKill::onTimeOut() ";

	if (ec)
	{
		LOG(WARNING) << "TimerActionKill is canceled." << std::endl;
	}
	else
	{
		LOG(INFO) << fname << "Timeout for process <" << m_process->getpid() << ">." << std::endl;
		Application::terminateProcess(m_process);
	}
	delete this;
}

