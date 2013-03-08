#include "ProcessorThread.hpp"

#include "Context.hpp"
#include "Exceptions.hpp"
#include "FileParser.hpp"

const std::chrono::milliseconds ProcessorThread::dequeueTimeout = std::chrono::milliseconds(500);

ProcessorThread::ProcessorThread(Context& context)
	: exit(false), busy(false), ctxt(context), t(&ProcessorThread::threadProc, this)
{
}

void ProcessorThread::beginExit()
{
	exit = true;
}

void ProcessorThread::join()
{
	exit = true;
	t.join();
}

void ProcessorThread::threadProc()
{
	while (!exit && !ctxt.error) {
		std::string fn = ctxt.queue.dequeue(dequeueTimeout);
		if (!fn.empty()) {
			busy = true;
			// TODO: Exception handling
			try {
				processFile(fn, ctxt);
			}
			catch (const Exceptions::Exception& ex) {
				ctxt.error = true;
				fprintf(stderr, "%s\n", ex.message.c_str());
			}
			catch (const std::exception& ex) {
				ctxt.error = true;
				fprintf(stderr, "Unexpected fatal error: %s\n", ex.what());
			}
			catch (...) {
				ctxt.error = true;
				fprintf(stderr, "Unexpected fatal error");
			}
			busy = false;
		}
	}
}
