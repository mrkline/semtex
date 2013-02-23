#include "ProcessorThread.hpp"

#include "FileQueue.hpp"

const std::chrono::milliseconds ProcessorThread::dequeueTimeout = std::chrono::milliseconds(500);

ProcessorThread::ProcessorThread(FileQueue& sfq)
	: exit(false), queue(sfq), t(&ProcessorThread::threadProc, this)
{
}

void ProcessorThread::join()
{
	exit = true;
}

void ProcessorThread::threadProc()
{
	while (!exit) {
		std::string fn = queue.dequeue(dequeueTimeout);
		if (!fn.empty()) {
			//! TODO: Call processFile
		}
	}
}
