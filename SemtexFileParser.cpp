#include "SemtexFileParser.hpp"

#include <thread>

#include "SemtexFileQueue.hpp"
#include "SemtexProcessorThread.hpp"

void processSemtexFile(const std::string& file, bool isBaseFile, bool verbose, SemtexFileQueue& sfq,
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
