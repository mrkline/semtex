#include "FileParser.hpp"

#include <array>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <cstring>
#include <fstream>

#include "ErrorHandling.hpp"
#include "Exceptions.hpp"
#include "Context.hpp"
#include "UnitReplacer.hpp"

static const size_t kInputLen = strlen("\\input"); //!< Length of "\input"
static const size_t kIncludeLen = strlen("\\include"); //! Length of "\include"
static std::array<const std::string, 3> extensions = {{".stex", ".sex", ".tex"}};

static std::vector<std::unique_ptr<Replacer>> replacers;

//! Only needs to be run once. initialized doesn't need to be atomic because the main thread will always run this.
static void init()
{
	static bool initialized = false;
	if (initialized)
		return;

	replacers.emplace_back(new UnitReplacer);
	initialized = true;
}

void processFile(const std::string& file, Context& ctxt)
{
	init();

	if (ctxt.verbose && !ctxt.error)
		printf("Processing %s...\n", file.c_str());

	std::ifstream inf(file, std::ifstream::binary);
	if (!inf.good()) {
		throw Exceptions::FileException("Error: Could not open " + file, __FUNCTION__);
	}
	inf.seekg(0, std::ifstream::end);
	size_t fileSize = inf.tellg();
	inf.seekg(0, std::ifstream::beg);
	std::unique_ptr<char[]> fileBuff(new char[fileSize]);
	inf.read(fileBuff.get(), fileSize);
	inf.close();
	//! \todo Convert to UTF-8 if needed

	// True if this is as .stex or .sex file and we will modify it
	bool createModdedCopy = false;
	const auto& ste = extensions[0];
	const auto& se = extensions[1];
	if ((file.length() > ste.length() && file.compare(file.length() - ste.length(), ste.length(), ste) == 0)
	    || (file.length() > se.length() && file.compare(file.length() - se.length(), se.length(), se) == 0))
		createModdedCopy = true;

	ParseInfo pi(file, fileBuff.get(), fileBuff.get() + fileSize, ctxt);
	while (pi.curr < pi.end) {
		// Characters to the end of the file
		const size_t remaining = pi.end - pi.curr;

		// Ignore commented-out lines
		if (pi.curr > fileBuff.get() && *pi.curr == '%' && *(pi.curr - 1) != '\\') {
			while (pi.curr < pi.end && *pi.curr != '\n' && *pi.curr != '\r')
				++pi.curr;
			readNewline(pi);
		}
		// If it's not-whitespace, try to match it to an include
		else if (isgraph(*pi.curr)) {
			if (strncmp(pi.curr, "\\include", std::min(kIncludeLen, remaining)) == 0 ||
				strncmp(pi.curr, "\\input", std::min(kInputLen, remaining)) == 0)
				processInclude(pi);
			// Otherwise try to match it to a mapping
			else {
				bool matched = false;
				if (createModdedCopy) { // Don't bother doing search and replace for files we won't modify
					for (const auto& r : replacers) {
						for (const auto& k : r->getKeys()) {
							if (strncmp(pi.curr, k.c_str(), std::min(k.length(), remaining)) == 0) {
								r->replace(k, pi);
								matched = true;
								break;
							}
						}
						if (matched)
							break;
					}

					//! \todo Recurse here. If a new replacement was made, create a ParsInfo for the replacement
					//        And scan through it. Repeat until no more replacements are found in the replacement.
				}
				if (!matched)
					++pi.curr; // Try again next time
			}
		}
		// Otherwise just chomp some whitespace
		else {
			while (readNewline(pi));
			eatWhitespace(pi);
		}
	}

	if (ctxt.verbose && !ctxt.error)
		printf("Done processing %s...\n", file.c_str());

	if (createModdedCopy && !ctxt.error) { // Don't bother creating a copy if we've errored out
		if (ctxt.verbose) // Fairly safe to skip another error check here since we just checked
			printf("Writing out LaTeX file for %s...\n", file.c_str());

		// Replace the file's extension
		static boost::regex fext(R"regex((stex|sex)$)regex", boost::regex::optimize);
		const std::string outname = boost::regex_replace(file, fext, "tex");
		std::ofstream outfile(outname);
		if (!outfile.good()) {
			throw Exceptions::FileException("Error: Could not open output file " + outname, __FUNCTION__);
		}
		ctxt.generatedFilesMutex.lock();
		ctxt.generatedFiles.emplace_back(outname);
		ctxt.generatedFilesMutex.unlock();
		if (pi.replacements.empty()) {
			outfile.write(fileBuff.get(), fileSize);
		}
		else {
			// TODO: Replace all newlines in replacements with the most commonly found newline in the file,
			//       perhaps using boost::replace_all
			const char* curr = fileBuff.get();
			for (const auto& r : pi.replacements) {
				// Write from the current location up to the start of the replacement
				outfile.write(curr, std::distance(curr, r.start));
				// Write the replacement
				outfile << r.replaceWith;
				curr = r.end;
			}
			// Write out the end of the file
			outfile.write(curr, std::distance(curr, pi.end));
		}
		if (ctxt.verbose && !ctxt.error) // Fairly safe to skip another error check here since we just checked
			printf("Done writing out LaTeX file for %s...\n", file.c_str());
	}
}

bool readNewline(ParseInfo& pi)
{
	if (pi.curr >= pi.end)
		return false;

	if (*pi.curr == '\n' || *pi.curr == '\r') {
		if (*pi.curr == '\r') {
			if (*(pi.curr + 1) == '\n') {
				++pi.windowsNewlines;
				pi.curr +=2;
			}
			else {
				++pi.macNewlines;
				++pi.curr;
			}
		}

		if (*pi.curr == '\n') {
			if (*(pi.curr + 1) == '\r') {
				// Rare, but possible
				++pi.windowsNewlines;
				pi.curr +=2;
			}
			else {
				++pi.unixNewlines;
				++pi.curr;
			}
		}
		++pi.currLine;
		return true;
	}
	return false;
}

void processInclude(ParseInfo& pi)
{
	// For printing purposes, etc., determine if it is \include or \input
	bool isInclude = pi.curr[3] == 'c';

	// Increment curr appropriately
	pi.curr += isInclude ? kIncludeLen : kInputLen;

	//! Get our args
	std::unique_ptr<MacroArgs> args;
	try {
		args = parseArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " for \\include or \\import", __FUNCTION__);
	}

	// We should only have one unnamed arg
	if (args->unnamed.size() != 1 || args->named.size() != 0) {
		errorOnLine(pi, "\\include and \\input only take a single, unnamed argument");
	}

	std::string filename = args->unnamed[0];

	for (const auto& ext : extensions) {
		std::string fullName = filename + ext;
		using namespace boost::filesystem;
		if (exists(symlink_status(fullName))) {
			if (pi.ctxt.verbose && !pi.ctxt.error)
				printf("Adding %s to the list of files to be processed\n", fullName.c_str());

			pi.ctxt.queue.enqueue(std::move(fullName));
		}
	}
}

std::unique_ptr<MacroArgs> parseArgs(ParseInfo& pi) {
	// Regex for matching args

	//! An unquoted, unnamed arg, such as { myArg }
	static boost::regex unquoted(R"regex(^\s*([^"=,}]*\s*[^"=,}\s]+)\s*(,|\})?)regex", boost::regex::optimize);
	//! A quoted, unnamed arg, such as { "myArg" }
	static boost::regex quoted(R"regex(^\s*"([^"]+)"\s*(,|\})?)regex", boost::regex::optimize);
	//! An unquoted, named arg, sugh as { foo = bar }
	static boost::regex unquotedNamed(R"regex(^\s*([a-zA-Z]+)\s*=\s*([^"=,}]*\s*[^"=,}\s]+)\s*(,|\})?)regex",
	                                  boost::regex::optimize);
	//! A quoted, named arg, sugh as { foo = "bar" }
	static boost::regex quotedNamed(R"regex(^\s*([a-zA-Z]+)\s*=\s*"([^"]+)"\s*(,|\})?)regex", boost::regex::optimize);
	//! A comma, separating args
	static boost::regex spacedComma(R"regex(^\s*,\s*)regex", boost::regex::optimize);

	std::unique_ptr<MacroArgs> ret(new MacroArgs());

	eatWhitespace(pi);
	// Accept one newline and more whitespace, then demand a {
	if (readNewline(pi)) {
		eatWhitespace(pi);
	}

	if (pi.curr >= pi.end)
		errorOnLine(pi, "End of file reached before finding arguments");

	if (*pi.curr != '{')
		errorOnLine(pi, "Bad argument list");

	++pi.curr;

	// Argument parsing loop
	bool namedReached = false; //Becomes true when named arguments are reached
	bool needsCommaNext = false;
	bool lastTokenWasComma = false;
	while (true) {
		eatWhitespace(pi);
		if (readNewline(pi)) {
			eatWhitespace(pi);
			// We cannot have two newlines in a row during an argument list. Make sure we don't get another
			if (readNewline(pi))
				errorOnLine(pi, "A new paragraph was found in the middle of the argument list");
		}
		// Prepare the string to pass to regex (the current line)
		const char* argEnd = pi.curr + 1;
		while(argEnd <= pi.end && *argEnd != '\r' && *argEnd != '\n')
			++argEnd;

		boost::cmatch argMatch;
		if (!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, quotedNamed)) {
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			// Make sure this argument doesn't already exist
			if (ret->named.find(newArgName) != ret->named.end())
				errorOnLine(pi, "Duplicate argument");

			ret->named[newArgName] = std::string(argMatch[2].first, argMatch[2].second);
			pi.curr = argMatch[0].second;
			namedReached = true;
			if (argMatch[3].matched) {
				if (*argMatch[3].first == '}')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
			needsCommaNext = !lastTokenWasComma;
		}
		else if	(!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, unquotedNamed)) {
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			// Make sure this argument doesn't already exist
			if (ret->named.find(newArgName) != ret->named.end())
				errorOnLine(pi, "Duplicate argument");

			ret->named[newArgName] = std::string(argMatch[2].first, argMatch[2].second);
			pi.curr = argMatch[0].second;
			namedReached = true;
			if (argMatch[3].matched) {
				if (*argMatch[3].first == '}')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
			needsCommaNext = !lastTokenWasComma;
		}
		else if(!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, quoted)) {
			if (namedReached && argMatch[0].second == argEnd) {
				// If this is the end of the line, yell at the user for putting an unnamed arg after a named one.
				// If this is not at the end of the line, we will register the arg as invalid in the next iteration
				errorOnLine(pi, "All unnamed arguments must come before named ones");
			}
			ret->unnamed.emplace_back(argMatch[1].first, argMatch[1].second);
			pi.curr = argMatch[0].second;
			if (argMatch[2].matched) {
				if (*argMatch[2].first == '}')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
			needsCommaNext = !lastTokenWasComma;
		}
		else if (!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, unquoted)) {
			if (namedReached && argMatch[0].second == argEnd) {
				// If this is the end of the line, yell at the user for putting an unnamed arg after a named one.
				// If this is not at the end of the line, we will register the arg as invalid in the next iteration
				errorOnLine(pi, "All unnamed arguments must come before named ones");
			}
			ret->unnamed.emplace_back(argMatch[1].first, argMatch[1].second);
			pi.curr = argMatch[0].second;
			if (argMatch[2].matched) {
				if (*argMatch[2].first == '}')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
			needsCommaNext = !lastTokenWasComma;
		}
		else if (boost::regex_search(pi.curr, argEnd, argMatch, spacedComma)) {
			/*
			 * Allow for stupid crap like:
			 * "myArg
			 * ,
			 * "myArg2
			 */
			if (lastTokenWasComma)
				errorOnLine(pi, "Missing argument (double commas)");

			pi.curr = argMatch[0].second;
			lastTokenWasComma = true;
			needsCommaNext = false;
		}
		else {
			errorOnLine(pi, "Invalid argument");
		}

	}


	return ret;
}
