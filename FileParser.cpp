#include "FileParser.hpp"

#include <thread>

#include "FileQueue.hpp"
#include "ProcessorThread.hpp"

void processFile(const std::string& file, bool isBaseFile, bool verbose, FileQueue& sfq,
                       std::unordered_set<std::string>& generatedFiles, std::mutex& generatedFilesMutex)
{
	if (verbose)
		printf("Processing %s\n", file.c_str());

	std::vector<std::string> neededFiles; // Files needed to build this file

	// TODO: Process file

	if (isBaseFile) {
		// Wait for all dependencies to finish being generated.
		// This is true when all threads are not processing a file and there are no files in the queue.

		// TODO: Run LaTeX
	}
}
