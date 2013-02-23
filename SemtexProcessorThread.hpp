#pragma once

#include <atomic>
#include <thread>

class SemtexFileQueue;

//! Processes files in an additional thread
class SemtexProcessorThread {
public:
	static const std::chrono::milliseconds dequeueTimeout;

	//! Constructor
	//! \param sfq Queue from which to pull files to process
	SemtexProcessorThread(SemtexFileQueue& sfq);

	//! Closes the thread as soon as it is done processing 
	void join();

private:
	std::atomic_bool exit; //!< Raised by join
	std::thread t;
	SemtexFileQueue& queue;

	void threadProc();
};
