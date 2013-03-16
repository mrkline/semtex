#pragma once

#include <atomic>
#include <thread>

class Context;

//! Processes files in an additional thread
class ProcessorThread {
public:
	static const std::chrono::milliseconds dequeueTimeout;

	ProcessorThread(Context& context);

	bool isBusy() const { return busy; }

	//! Tells thread to finish its current file and close
	void beginExit();

	//! Closes the thread as soon as it is done processing 
	void join();

private:
	std::atomic_bool exit; //!< Raised by join
	std::atomic_bool busy; //!< True when processing a file, false when waiting for one to process.
	Context& ctxt;
	std::thread t;

	void threadProc();
};
