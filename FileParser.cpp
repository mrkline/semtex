#include "FileParser.hpp"

#include "Context.hpp"

void processFile(const std::string& file, Context& ctxt)
{
	if (ctxt.verbose)
		printf("Processing %s\n", file.c_str());

	std::vector<std::string> neededFiles; // Files needed to build this file

	// TODO: Process file
}
