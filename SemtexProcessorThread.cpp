#include "SemtexProcessorThread.hpp"

#include "SemtexFileQueue.hpp"

const std::chrono::milliseconds SemtexProcessorThread::dequeueTimeout = std::chrono::milliseconds(500);

SemtexProcessorThread::SemtexProcessorThread(SemtexFileQueue& sfq)
	: exit(false), queue(sfq), t(&SemtexProcessorThread::threadProc, this)
{
}

void SemtexProcessorThread::join()
{
	exit = true;
}

void SemtexProcessorThread::threadProc()
{
	while (!exit) {
		std::string fn = queue.dequeue(dequeueTimeout);
		if (!fn.empty()) {
			//! TODO: Call processSemtexFile
		}
	}
}
