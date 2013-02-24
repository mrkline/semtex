#include "FileParser.hpp"

#include "Context.hpp"

void processFile(const std::string& file, Context& ctxt)
{
	if (ctxt.verbose)
		printf("Processing %s...\n", file.c_str());

	// TODO: Process file

	if (ctxt.verbose)
		printf("Done processing %s...\n", file.c_str());
}
