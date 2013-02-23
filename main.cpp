#include <tclap/CmdLine.h>

#include <string>
#include <unordered_set>
#include <vector>
#include <mutex>

#include "SemtexFileParser.hpp"
#include "SemtexFileQueue.hpp"
#include "SemtexProcessorThread.hpp"

//! TODO: Bundle into a global state struct
static bool verbose = false;
static bool threadsStarted = false;
static std::unordered_set<std::string> generatedFiles;
static std::mutex generatedFilesMutex;
static std::vector<SemtexProcessorThread> auxThreads;

void severalCallback(SemtexFileQueue& sfq)
{
	if (threadsStarted)
		return;
	
	if (verbose)
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

	verbose = verbFlag.getValue();

	SemtexFileQueue sfq(severalCallback);

	if (verbose)
		printf("Running SemTex - Streamlined LaTeX\n");

	processSemtexFile(fileArg.getValue(), true, verbose, sfq, generatedFiles, generatedFilesMutex);

	return 0;
}
