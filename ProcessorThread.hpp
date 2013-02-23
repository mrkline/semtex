#pragma once

#include <atomic>
#include <thread>

class FileQueue;

//! Processes files in an additional thread
class ProcessorThread {
public:
	static const std::chrono::milliseconds dequeueTimeout;

	//! Constructor
	//! \param sfq Queue from which to pull files to process
	ProcessorThread(FileQueue& sfq);

	//! Closes the thread as soon as it is done processing 
	void join();

private:
	std::atomic_bool exit; //!< Raised by join
	std::thread t;
	FileQueue& queue;

	void threadProc();
};
