#include "precomp.hpp"

#include "FileParser.hpp"

#include "ErrorHandling.hpp"
#include "Exceptions.hpp"
#include "Context.hpp"
/*
#include "DerivReplacer.hpp"
#include "IntegralReplacer.hpp"
#include "SummationReplacer.hpp"
#include "UnitReplacer.hpp"
*/
#include "TestReplacer.hpp"

namespace { // Ensure these variables are accessible only within this file.
	const size_t kInputLen = strlen("\\input"); //!< Length of "\input"
	const size_t kIncludeLen = strlen("\\include"); //!< Length of "\include"
	std::array<const std::string, 3> extensions = {{".stex", ".sex", ".tex"}};
	std::unordered_set<std::string> trueStrings = {{"true", "True", "TRUE", "t", "T", "y", "Y", "yes", "Yes", "1"}};
	std::unordered_set<std::string> falseStrings = {{"false", "False", "FALSE", "f", "F", "n", "N", "no", "No", "0"}};

	namespace Replacers {
		/*
		UnitReplacer ur;
		IntegralReplacer ir;
		SummationReplacer sr;
		DerivReplacer dr;
		*/
		TestReplacer tr;
	}
	std::array<Replacer*, 1> replacers = {{&Replacers::tr}} ; // {{&Replacers::ur, &Replacers::ir, &Replacers::sr, &Replacers::dr}};
}

bool getStringTruthValue(const ParseInfo& pi, const std::string& str)
{
	if (trueStrings.find(str) != trueStrings.end())
		return true;
	else if (falseStrings.find(str) != falseStrings.end())
		return false;
	else
		errorOnLine(pi, "Unknown value for boolean argument");

	// Make the compiler shut up about control reaching the end ofa a non-void function
	return false;
}

void processFile(const std::string& file, Context& ctxt)
{
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
	parseLoop(pi, createModdedCopy);

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

void parseLoop(ParseInfo& pi, bool createReplacements)
{
	const char* const first = pi.curr;
	while (pi.curr < pi.end) {
		// Characters to the end of the file
		const size_t remaining = pi.end - pi.curr;

		// Ignore commented-out lines
		if (pi.curr > first && *pi.curr == '%' && *(pi.curr - 1) != '\\') {
			while (pi.curr < pi.end && *pi.curr != '\n' && *pi.curr != '\r')
				++pi.curr;
			readNewline(pi);
		}
		// If it's not-whitespace, try to match it to an include
		else if (isgraph(*pi.curr)) {
			//! \todo Should we do this when recursing?
			if ((remaining > kIncludeLen && // There are enough remaining characters to be our key
			     strncmp(pi.curr, "\\include", kIncludeLen) == 0 && // These characters match the key
			     (pi.curr[kIncludeLen] == '{' || isspace(pi.curr[kIncludeLen]))) // This is not just part of a key
			    ||
				(remaining > kInputLen &&
			     strncmp(pi.curr, "\\input", kInputLen) == 0 &&
			     (pi.curr[kInputLen] == '{' || isspace(pi.curr[kInputLen]))))
				processInclude(pi);
			// Otherwise try to match it to a mapping
			else {
				bool matched = false;
				bool shouldRecurse;
				int line;
				if (createReplacements) { // Don't bother doing search and replace for files we won't modify
					for (const auto& r : replacers) {
						for (const auto& k : r->getKeys()) {
							if (remaining > k.length() && // There are enough remaining characters to be our key
							    strncmp(pi.curr, k.c_str(), k.length()) == 0 && // These characters match the key
							    (pi.curr[k.length()] == '{' ||
								 pi.curr[k.length()] == '[' ||
								 isspace(pi.curr[k.length()]))) { // Not just part of key
								matched = true;
								shouldRecurse = r->shouldRecurse();
								line = pi.currLine;
								r->replace(k, pi);
								break;
							}
						}
						if (matched)
							break;
					}

					// Recurse here. If a new replacement was made, create a ParsInfo for the replacement
					// and scan through it. Repeat until no more replacements are found in the replacement.
					if (matched && shouldRecurse) {
						const std::string& toSubSearch = pi.replacements.back().replaceWith;
						const char* subStart = toSubSearch.c_str();
						const char* subEnd = subStart + toSubSearch.size();
						ParseInfo rpi(pi.filename, subStart, subEnd, pi.ctxt, line);
						parseLoop(rpi, true); // Recurse using our new context
						if (!rpi.replacements.empty()) {
							std::string newRep;
							const char* curr = subStart;
							for (const auto& r : rpi.replacements) {
								// Write from the current location up to the start of the replacement
								newRep.append(curr, r.start);
								// Write the replacement
								newRep.append(r.replaceWith);
								curr = r.end;
							}
							newRep.append(curr, rpi.end);
							pi.replacements.back().replaceWith = std::move(newRep);
						}
					}
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
}

bool readNewline(ParseInfo& pi)
{
	if (pi.curr >= pi.end)
		return false;

	if (*pi.curr == '\n' || *pi.curr == '\r') {
		if (*pi.curr == '\r') {
			if (pi.curr < pi.end - 1 && *(pi.curr + 1) == '\n') {
				++pi.windowsNewlines;
				pi.curr +=2;
			}
			else {
				++pi.macNewlines;
				++pi.curr;
			}
		}

		if (*pi.curr == '\n') {
			if (pi.curr < pi.end - 1 && *(pi.curr + 1) == '\r') {
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
	std::unique_ptr<std::vector<std::string>> args;
	try {
		args = parseBracketArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " for \\include or \\import", __FUNCTION__);
	}

	// We should only have one arg
	if (args->size() != 1) {
		errorOnLine(pi, "\\include and \\input only take a single, unnamed argument");
	}

	const std::string& filename = (*args)[0];

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

std::unique_ptr<MacroOptions> parseMacroOptions(ParseInfo& pi) {
	// Regex for matching args

	//! An unquoted, unnamed arg, such as [ myArg ]
	static boost::regex unquoted(R"regex(^\s*([^"=,\]]*\s*[^"=,\]\s]+)\s*(,|\])?)regex", boost::regex::optimize);
	//! A quoted, unnamed arg, such as [ "myArg" ]
	static boost::regex quoted(R"regex(^\s*"([^"]+)"\s*(,|\])?)regex", boost::regex::optimize);
	//! An unquoted, named arg, sugh as [ foo = bar ]
	static boost::regex unquotedNamed(R"regex(^\s*([a-zA-Z]+)\s*=\s*([^"=,\]]*\s*[^"=,\]\s]+)\s*(,|\])?)regex",
	                                  boost::regex::optimize);
	//! A quoted, named arg, sugh as [ foo = "bar" ]
	static boost::regex quotedNamed(R"regex(^\s*([a-zA-Z]+)\s*=\s*"([^"]+)"\s*(,|\])?)regex", boost::regex::optimize);
	//! A comma, separating args
	static boost::regex spacedComma(R"regex(^\s*,\s*)regex", boost::regex::optimize);

	std::unique_ptr<MacroOptions> ret(new MacroOptions());

	eatWhitespace(pi);
	// Accept one newline and more whitespace, then demand a [
	if (readNewline(pi)) {
		eatWhitespace(pi);
	}

	if (pi.curr >= pi.end)
		errorOnLine(pi, "End of file reached before finding arguments");

	if (*pi.curr != '[')
		return ret;

	++pi.curr;

	// Argument parsing loop
	bool needsCommaNext = false;
	bool lastTokenWasComma = false;
	while (true) {
		eatWhitespace(pi);
		if (readNewline(pi)) {
			eatWhitespace(pi);
			// We cannot have two newlines in a row during an option list. Make sure we don't get another
			if (readNewline(pi))
				errorOnLine(pi, "A new paragraph was found in the middle of the options list");
		}
		// Prepare the string to pass to regex (the current line)
		const char* argEnd = pi.curr + 1;
		while(argEnd <= pi.end && *argEnd != '\r' && *argEnd != '\n')
			++argEnd;

		boost::cmatch argMatch;
		if (!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, quotedNamed)) {
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->opts.find(newArgName) != ret->opts.end())
				errorOnLine(pi, "Duplicate option");

			ret->opts[newArgName] = std::string(argMatch[2].first, argMatch[2].second);
			pi.curr = argMatch[0].second;
			if (argMatch[3].matched) {
				if (*argMatch[3].first == ']')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
			needsCommaNext = !lastTokenWasComma;
		}
		else if (!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, unquotedNamed)) {
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->opts.find(newArgName) != ret->opts.end())
				errorOnLine(pi, "Duplicate option");

			ret->opts[newArgName] = std::string(argMatch[2].first, argMatch[2].second);
			pi.curr = argMatch[0].second;
			if (argMatch[3].matched) {
				if (*argMatch[3].first == ']')
					break;
				lastTokenWasComma = *argMatch[3].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
			needsCommaNext = !lastTokenWasComma;
		}
		else if(!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, quoted)) {
			std::string flag(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->flags.find(flag) != ret->flags.end())
				errorOnLine(pi, "Duplicate flag");

			ret->flags.insert(std::move(flag));
			pi.curr = argMatch[0].second;
			if (argMatch[2].matched) {
				if (*argMatch[2].first == ']')
					break;
				lastTokenWasComma = *argMatch[2].first == ',';
			}
			else {
				lastTokenWasComma = false;
			}
			needsCommaNext = !lastTokenWasComma;
		}
		else if (!needsCommaNext && boost::regex_search(pi.curr, argEnd, argMatch, unquoted)) {
			std::string flag(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->flags.find(flag) != ret->flags.end())
				errorOnLine(pi, "Duplicate flag");

			ret->flags.insert(std::move(flag));
			pi.curr = argMatch[0].second;
			if (argMatch[2].matched) {
				if (*argMatch[2].first == ']')
					break;
				lastTokenWasComma = *argMatch[2].first == ',';
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
				errorOnLine(pi, "Missing option (double commas)");

			pi.curr = argMatch[0].second;
			lastTokenWasComma = true;
			needsCommaNext = false;
		}
		else {
			errorOnLine(pi, "Invalid option");
		}

	}

	return ret;
}

std::unique_ptr<std::vector<std::string>> parseBracketArgs(ParseInfo& pi)
{
	std::unique_ptr<std::vector<std::string>> ret(new std::vector<std::string>);

	const char* argsEnd = pi.curr;
	while (true) {
		eatWhitespace(pi);
		// Accept one newline and more whitespace, then demand a {
		if (readNewline(pi)) {
			eatWhitespace(pi);
		}

		if (pi.curr >= pi.end || *pi.curr != '{')
			break;

		const char* argStart = ++pi.curr; // Advance to the first character of the argument (after the '{')
		int braceLevel = 1;

		while (braceLevel > 0) {
			if (pi.curr >= pi.end)
				errorOnLine(pi, "End of file reached before finding end of argument");

			if (*pi.curr == '\r' || *pi.curr == '\n') {
				readNewline(pi);
			}
			else {
				if (*pi.curr == '{' && *(pi.curr - 1) != '\\')
					++braceLevel;
				else if (*pi.curr == '}' && *(pi.curr - 1) != '\\')
					--braceLevel;

				++pi.curr;
			}
		}
		ret->emplace_back(argStart, pi.curr - 1);
		argsEnd = pi.curr;
	}
	pi.curr = argsEnd;
	return ret;
}
