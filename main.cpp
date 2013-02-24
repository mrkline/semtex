#include <tclap/CmdLine.h>

#include <algorithm>
#include <string>
#include <thread>
#include <vector>

#include "Context.hpp"
#include "FileParser.hpp"
#include "FileQueue.hpp"
#include "ProcessorThread.hpp"

// Prototype for the function below so we can declare ctxt with the other static variables.
// Slightly kludgy, I know.
void severalCallback(FileQueue& sfq);

static bool threadsStarted = false;
static std::vector<ProcessorThread> auxThreads;
static Context ctxt(&severalCallback);

void severalCallback(FileQueue& sfq)
{
	if (threadsStarted)
		return;

	if (ctxt.verbose)
		printf("Processing multiple files. Starting up additional threads.\n");

	threadsStarted = true;
}

int main(int argc, char** argv) {
	TCLAP::SwitchArg verbFlag("v", "verbose", "Print additional output");
	TCLAP::UnlabeledValueArg<std::string> fileArg("file", "Base SemTeX file", true, "",  "file");

	TCLAP::CmdLine cmd("SemTeX - streamlined LaTeX", ' ', "alpha");
	cmd.add(verbFlag);
	cmd.add(fileArg);

	cmd.parse(argc, argv);

	ctxt.verbose = verbFlag.getValue();

	if (ctxt.verbose)
		printf("Running SemTex - Streamlined LaTeX\n");

	processFile(fileArg.getValue(), ctxt);

	// Wait for the threads to finish doing their thing
	// See FileQueue::setDequeueEnabled for an explanation on why we are disabling dequeuing here
	while (true) {
		ctxt.queue.setDequeueEnabled(false);
		bool done = std::none_of(auxThreads.begin(), auxThreads.end(),
		                         [](const ProcessorThread& pt) { return pt.isBusy(); } );
		done = done && ctxt.queue.empty();
		ctxt.queue.setDequeueEnabled(true);
		if (done)
			break;
		std::this_thread::sleep_for(ProcessorThread::dequeueTimeout);
	}

	for (auto& thread : auxThreads)
		thread.join();


	// TODO: Run LaTeX, or just clean up if there was an error

	return 0;
}
