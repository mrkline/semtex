#include "FileParser.hpp"

#include <boost/regex.hpp>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sstream>

#include "Exceptions.hpp"
#include "Context.hpp"

struct ReplacementMapping {
 	std::string key; //!< String to trigger a replacement
	ReplacementGenerator* gen; //!< Function pointer to a function that will do the replacement

	// Used for sorting mappings from shortest to longest
	bool operator< (const ReplacementMapping& other) { return key.length() < other.key.length(); }
};

static const size_t kInputLen = strlen("\\input"); //!< Length of "\input"
static const size_t kIncludeLen = strlen("\\include"); //! Length of "\include"

// TODO: Create a series of ReplacementMappings
static void init()
{
}

static bool fileExists(const std::string& file)
{
	struct stat sb;
	return stat(file.c_str(), &sb) == 0;
}

bool processFile(const std::string& file, Context& ctxt)
{
	if (ctxt.verbose)
		printf("Processing %s...\n", file.c_str());

	std::ifstream inf(file, std::ifstream::binary);
	if (!inf.good()) {
		printf("Error: Could not open %s\n", file.c_str());
		return false;
	}
	inf.seekg(0, std::ifstream::end);
	size_t fileSize = inf.tellg();
	inf.seekg(0, std::ifstream::beg);
	std::unique_ptr<char[]> fileBuff(new char[fileSize]);
	inf.read(fileBuff.get(), fileSize);
	inf.close();
	//! \todo Convert to UTF-8 if needed

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
		// If it's not-whitespace, try to match it to something
		else if (isgraph(*pi.curr)) {
			if (strncmp(pi.curr, "\\include", std::min(kIncludeLen, remaining)) == 0 ||
				strncmp(pi.curr, "\\input", std::min(kInputLen, remaining)) == 0)
				processInclude(pi);
			else
				++pi.curr; // Try again next time
		}
		else {
			// Gobble whitespace
			while (readNewline(pi));
			eatWhitespace(pi);
		}
	}

	if (ctxt.verbose)
		printf("Done processing %s...\n", file.c_str());

	return true;
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
	printf("Is %san include\n", isInclude ? "" : "not "); // TEMP

	// Increment curr appropriately
	pi.curr += isInclude ? kIncludeLen : kInputLen;

	//! Get our args
	std::unique_ptr<MacroArgs> args;
	try {
		args = parseArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		std::stringstream err;
		err << pi.filename << ":" << pi.currLine << ": " << ex.message << " for \\include or \\import";
		throw Exceptions::InvalidInputException(err.str(), __FUNCTION__);
	}

	exit(0);

	// We should only have one unnamed arg
	if (args->unnamed.size() != 1 || args->named.size() != 0) {
		std::stringstream err;
		err << pi.filename << ":" << pi.currLine << ": ";
		err << "\\include and \\input only take a single, unnamed argument";
		throw Exceptions::InvalidInputException(err.str(), __FUNCTION__);
	}

	const std::string& filename = args->unnamed[0];

	if (fileExists(filename + ".tex")) {
		if (pi.ctxt.verbose)
			printf("Adding %s to the list of files to be processed\n", (filename + ".tex").c_str());
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
		throw Exceptions::InvalidInputException("End of file reached before finding arguments", __FUNCTION__);

	if (*pi.curr != '{')
		throw Exceptions::InvalidInputException("A new paragraph was found before arguments were found", __FUNCTION__);

	++pi.curr;

	// Argument parsing loop
	bool namedReached = false; //Becomes true when named arguments are reached
	bool lastTokenWasComma = false;
	while (true) {
		eatWhitespace(pi);
		if (readNewline(pi)) {
			eatWhitespace(pi);
			// We cannot have two newlines in a row during an argument list. Make sure we don't get another
			if (readNewline(pi)) {
				throw Exceptions::InvalidInputException("A new paragraph was found in the middle of the argument list",
				                                        __FUNCTION__);
			}
		}
		// Prepare the string to pass to regex (the current line)
		const char* argEnd = pi.curr + 1;
		while(argEnd <= pi.end && *argEnd != '\r' && *argEnd != '\n')
			++argEnd;

		boost::cmatch argMatch;
		if (boost::regex_search(pi.curr, argEnd, argMatch, quotedNamed)) {
			// TODO (once done debugging): just use emplace?
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			std::string newArg(argMatch[2].first, argMatch[2].second);
			ret->named[newArgName] = newArg;
			printf("Quoted, named arg found: %s=%s\n", newArgName.c_str(), newArg.c_str());
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
		}
		else if	(boost::regex_search(pi.curr, argEnd, argMatch, unquotedNamed)) {
			// TODO (once done debugging): just use emplace?
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			std::string newArg(argMatch[2].first, argMatch[2].second);
			ret->named[newArgName] = newArg;
			printf("Unquoted, named arg found: %s=%s\n", newArgName.c_str(), newArg.c_str());
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
		}
		else if(boost::regex_search(pi.curr, argEnd, argMatch, quoted)) {
			if (namedReached)
				throw Exceptions::InvalidInputException("All unnamed arguments must come before named ones",
				                                        __FUNCTION__);
			// TODO (once done debugging): just use emplace?
			std::string newArg(argMatch[1].first, argMatch[1].second);
			ret->unnamed.push_back(newArg);
			printf("Quoted, unnamed arg found: %s\n", newArg.c_str());
			pi.curr = argMatch[0].second;
			if (argMatch[2].matched) {
				if (*argMatch[2].first == '}')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
		}
		else if (boost::regex_search(pi.curr, argEnd, argMatch, unquoted)) {
			if (namedReached)
				throw Exceptions::InvalidInputException("All unnamed arguments must come before named ones",
				                                        __FUNCTION__);
			// TODO (once done debugging): just use emplace?
			std::string newArg(argMatch[1].first, argMatch[1].second);
			ret->unnamed.push_back(newArg);
			printf("Unquoted, unnamed arg found: %s\n", newArg.c_str());
			pi.curr = argMatch[0].second;
			if (argMatch[2].matched) {
				if (*argMatch[2].first == '}')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
		}
		else if (boost::regex_search(pi.curr, argEnd, argMatch, spacedComma)) {
			/*
			 * Allow for stupid crap like:
			 * "myArg
			 * ,
			 * "myArg2
			 */
			if (lastTokenWasComma)
				throw Exceptions::InvalidInputException("Missing argument (double commas)", __FUNCTION__);
			pi.curr = argMatch[0].second;
			lastTokenWasComma = true;
		}
		else {
			throw Exceptions::InvalidInputException("Invalid argument", __FUNCTION__);
		}

	}


	return ret;
}
