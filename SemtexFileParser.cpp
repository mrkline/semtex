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

	// Make sure other threads have taken care of all our needed files before exiting.
	bool dependenciesDone = true;
	do {
		if (!dependenciesDone) { // Don't wait the first time
			// Otherwise, sleep some to give the threads time to work
			std::this_thread::sleep_for(SemtexProcessorThread::dequeueTimeout * 2);
		}

		std::lock_guard<std::mutex> lock(generatedFilesMutex);
		for (const auto& file : neededFiles) {
			if (generatedFiles.find(file) == generatedFiles.end()) {
				dependenciesDone = false;
				break;
			}
		}
	} while (!dependenciesDone);

	if (isBaseFile) {
		// TODO: Run LaTeX
	}
}
