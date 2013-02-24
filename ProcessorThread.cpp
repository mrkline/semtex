#include "ProcessorThread.hpp"

#include "Context.hpp"
#include "FileParser.hpp"

const std::chrono::milliseconds ProcessorThread::dequeueTimeout = std::chrono::milliseconds(500);

ProcessorThread::ProcessorThread(Context& context)
	: exit(false), busy(false), ctxt(context), t(&ProcessorThread::threadProc, this)
{
}

void ProcessorThread::join()
{
	exit = true;
}

void ProcessorThread::threadProc()
{
	while (!exit && !ctxt.error) {
		std::string fn = ctxt.queue.dequeue(dequeueTimeout);
		if (!fn.empty()) {
			busy = true;
			if(!processFile(fn, ctxt))
				ctxt.error = true;
			busy = false;
		}
	}
}
