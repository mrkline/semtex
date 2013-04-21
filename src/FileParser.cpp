#include "precomp.hpp"

#include "FileParser.hpp"

#include "Exceptions.hpp"
#include "Context.hpp"
#include "DirectReplacer.hpp"
#include "DerivReplacer.hpp"
#include "IntegralReplacer.hpp"
#include "SummationReplacer.hpp"
#include "UnitReplacer.hpp"
#include "PiecewiseReplacer.hpp"
// #include "TestReplacer.hpp"

namespace { // Ensure these variables are accessible only within this file.
	const size_t kInputLen = strlen("\\input"); //!< Length of "\input"
	const size_t kIncludeLen = strlen("\\include"); //!< Length of "\include"
	std::array<const std::string, 3> extensions = {{".stex", ".sex", ".tex"}};
	std::unordered_set<std::string> trueStrings = {{"true", "True", "TRUE", "t", "T", "y", "Y", "yes", "Yes", "1"}};
	std::unordered_set<std::string> falseStrings = {{"false", "False", "FALSE", "f", "F", "n", "N", "no", "No", "0"}};

	namespace Replacers {
		IntegralReplacer ir;
		UnitReplacer ur;
		SummationReplacer sr;
		DerivReplacer dr;
		DirectReplacer ar;
		PiecewiseReplacer pr;
		// TestReplacer tr;
	}
	std::array<Replacer*, 6> replacers = {{&Replacers::ur, &Replacers::ir, &Replacers::sr, &Replacers::dr,
	                                       &Replacers::ar, &Replacers::pr}};
}

bool Parser::getStringTruthValue(const std::string& str)
{
	if (trueStrings.find(str) != trueStrings.end())
		return true;
	else if (falseStrings.find(str) != falseStrings.end())
		return false;
	else
		errorOnLine("Unknown value for boolean argument");

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

	Parser p(file, fileBuff.get(), fileBuff.get() + fileSize, ctxt);
	p.parseLoop(createModdedCopy);

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
		if (p.replacements.empty()) {
			outfile.write(fileBuff.get(), fileSize);
		}
		else {

			const char* curr = fileBuff.get();
			std::string mostCommonNewline = p.getMostCommonNewline();
			for (auto& r : p.replacements) {
				// Replace all newlines in replacements with the most commonly found newline in the file,
				boost::replace_all(r.replaceWith, "\n", mostCommonNewline);
				// Write from the current location up to the start of the replacement
				outfile.write(curr, std::distance(curr, r.start));
				// Write the replacement
				outfile << r.replaceWith;
				curr = r.end;
			}
			// Write out the end of the file
			outfile.write(curr, std::distance(curr, p.end));
		}
		if (ctxt.verbose && !ctxt.error) // Fairly safe to skip another error check here since we just checked
			printf("Done writing out LaTeX file for %s...\n", file.c_str());
	}
}

void Parser::parseLoop(bool createReplacements)
{
	const char* const first = curr;
	while (curr < end) {
		// Characters to the end of the file
		const size_t remaining = end - curr;

		// Ignore commented-out lines
		if (curr > first && *curr == '%' && *(curr - 1) != '\\') {
			while (curr < end && *curr != '\n' && *curr != '\r')
				++curr;
			readNewline();
		}
		// If it's not-whitespace, try to match it to an include
		else if (isgraph(*curr) || *curr < 0 /* unicode */) {
			//! \todo Should we do this when recursing?
			if ((remaining > kIncludeLen && // There are enough remaining characters to be our key
			     strncmp(curr, "\\include", kIncludeLen) == 0 && // These characters match the key
			     (curr[kIncludeLen] == '{' || isspace(curr[kIncludeLen]))) // This is not just part of a key
			    ||
				(remaining > kInputLen &&
			     strncmp(curr, "\\input", kInputLen) == 0 &&
			     (curr[kInputLen] == '{' || isspace(curr[kInputLen]))))
				processInclude();
			// Otherwise try to match it to a mapping
			else {
				bool matched = false;
				bool shouldRecurse = false;
				int line = currLine;
				if (createReplacements) { // Don't bother doing search and replace for files we won't modify
					const char* endSearch = curr + 1;
					while (endSearch < end && *endSearch != ' ' && *endSearch != '\r' && *endSearch != '\n')
						++endSearch;
					std::string toSearch(curr, endSearch);
					for (const auto& r : replacers) {
						auto it = r->getKeys().lower_bound(toSearch);
						if (it == r->getKeys().end())
							--it;
						const auto itLen = it->length();

						 // Don't attempt to match something that ends with characters and is followed by characters
						 // (it might be some LaTeX command or something)
						if (strncmp(curr, it->c_str(), itLen) == 0 &&
						    !(isalpha(curr[itLen] && isalpha(curr[itLen - 1])))) {
							matched = true;
							shouldRecurse = r->shouldRecurse();
							line = currLine;
							r->replace(*it, *this);
							break;
						}
						if (matched)
							break;
					}

					// Recurse here. If a new replacement was made, create a ParsInfo for the replacement
					// and scan through it. Repeat until no more replacements are found in the replacement.
					if (matched && shouldRecurse) {
						const std::string& toSubSearch = replacements.back().replaceWith;
						const char* subStart = toSubSearch.c_str();
						const char* subEnd = subStart + toSubSearch.size();
						Parser rp(filename, subStart, subEnd, ctxt, line);
						rp.parseLoop(true); // Recurse using our new context
						if (!rp.replacements.empty()) {
							std::string newRep;
							const char* curr = subStart;
							for (const auto& r : rp.replacements) {
								// Write from the current location up to the start of the replacement
								newRep.append(curr, r.start);
								// Write the replacement
								newRep.append(r.replaceWith);
								curr = r.end;
							}
							newRep.append(curr, rp.end);
							replacements.back().replaceWith = std::move(newRep);
						}
					}
				}
				if (!matched)
					++curr; // Try again next time
			}
		}
		// Otherwise just chomp some whitespace
		else {
			while (readNewline());
			eatWhitespace();
		}
	}
}

bool Parser::readNewline()
{
	if (curr >= end)
		return false;

	if (*curr == '\n' || *curr == '\r') {
		if (*curr == '\r') {
			if (curr < end - 1 && *(curr + 1) == '\n') {
				++windowsNewlines;
				curr +=2;
			}
			else {
				++macNewlines;
				++curr;
			}
		}

		if (*curr == '\n') {
			if (curr < end - 1 && *(curr + 1) == '\r') {
				// Rare, but possible
				++windowsNewlines;
				curr +=2;
			}
			else {
				++unixNewlines;
				++curr;
			}
		}
		++currLine;
		return true;
	}
	return false;
}

void Parser::processInclude()
{
	// For printing purposes, etc., determine if it is \include or \input
	bool isInclude = curr[3] == 'c';

	// Increment curr appropriately
	curr += isInclude ? kIncludeLen : kInputLen;

	//! Get our args
	std::unique_ptr<std::vector<std::string>> args;
	try {
		args = parseBracketArgs();
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " for \\include or \\import", __FUNCTION__);
	}

	// We should only have one arg
	if (args->size() != 1) {
		errorOnLine("\\include and \\input only take a single, unnamed argument");
	}

	const std::string& filename = (*args)[0];

	for (const auto& ext : extensions) {
		std::string fullName = filename + ext;
		using namespace boost::filesystem;
		if (exists(symlink_status(fullName))) {
			if (ctxt.verbose && !ctxt.error)
				printf("Adding %s to the list of files to be processed\n", fullName.c_str());

			ctxt.queue.enqueue(std::move(fullName));
		}
	}
}

std::unique_ptr<MacroOptions> Parser::parseMacroOptions() {
	// Regex for matching args

	// An unquoted, unnamed arg, such as [ myArg ]
	static boost::regex unquoted(R"regex(^\s*([^"=,\]]*\s*[^"=,\]\s]+)\s*(,|\])?)regex", boost::regex::optimize);
	// A quoted, unnamed arg, such as [ "myArg" ]
	static boost::regex quoted(R"regex(^\s*"([^"]+)"\s*(,|\])?)regex", boost::regex::optimize);
	// An unquoted, named arg, sugh as [ foo = bar ]
	static boost::regex unquotedNamed(R"regex(^\s*([a-zA-Z]+)\s*=\s*([^"=,\]]*\s*[^"=,\]\s]+)\s*(,|\])?)regex",
	                                  boost::regex::optimize);
	// A quoted, named arg, sugh as [ foo = "bar" ]
	static boost::regex quotedNamed(R"regex(^\s*([a-zA-Z]+)\s*=\s*"([^"]+)"\s*(,|\])?)regex", boost::regex::optimize);
	// A comma, separating args
	static boost::regex spacedComma(R"regex(^\s*,\s*)regex", boost::regex::optimize);

	std::unique_ptr<MacroOptions> ret(new MacroOptions);

	readToNextLineText();

	if (curr >= end)
		errorOnLine("End of file reached before finding arguments");

	if (*curr != '[')
		return ret;

	++curr;

	// Argument parsing loop
	bool needsCommaNext = false;
	bool lastTokenWasComma = false;
	while (true) {
		eatWhitespace();
		if (readNewline()) {
			eatWhitespace();
			// We cannot have two newlines in a row during an option list. Make sure we don't get another
			if (readNewline())
				errorOnLine("A new paragraph was found in the middle of the options list");
		}
		// Prepare the string to pass to regex (the current line)
		const char* argEnd = curr + 1;
		while(argEnd <= end && *argEnd != '\r' && *argEnd != '\n')
			++argEnd;

		boost::cmatch argMatch;
		if (!needsCommaNext && boost::regex_search(curr, argEnd, argMatch, quotedNamed)) {
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->opts.find(newArgName) != ret->opts.end())
				errorOnLine("Duplicate option");

			ret->opts[newArgName] = std::string(argMatch[2].first, argMatch[2].second);
			curr = argMatch[0].second;
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
		else if (!needsCommaNext && boost::regex_search(curr, argEnd, argMatch, unquotedNamed)) {
			std::string newArgName(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->opts.find(newArgName) != ret->opts.end())
				errorOnLine("Duplicate option");

			ret->opts[newArgName] = std::string(argMatch[2].first, argMatch[2].second);
			curr = argMatch[0].second;
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
		else if(!needsCommaNext && boost::regex_search(curr, argEnd, argMatch, quoted)) {
			std::string flag(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->flags.find(flag) != ret->flags.end())
				errorOnLine("Duplicate flag");

			ret->flags.insert(std::move(flag));
			curr = argMatch[0].second;
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
		else if (!needsCommaNext && boost::regex_search(curr, argEnd, argMatch, unquoted)) {
			std::string flag(argMatch[1].first, argMatch[1].second);
			// Make sure this option doesn't already exist
			if (ret->flags.find(flag) != ret->flags.end())
				errorOnLine("Duplicate flag");

			ret->flags.insert(std::move(flag));
			curr = argMatch[0].second;

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
		else if (boost::regex_search(curr, argEnd, argMatch, spacedComma)) {
			/*
			 * Allow for stupid crap like:
			 * "myArg
			 * ,
			 * "myArg2
			 */
			if (lastTokenWasComma)
				errorOnLine("Missing option (double commas)");

			curr = argMatch[0].second;
			lastTokenWasComma = true;
			needsCommaNext = false;
		}
		else {
			errorOnLine("Invalid option");
		}

	}

	return ret;
}

std::unique_ptr<std::vector<std::string>> Parser::parseBracketArgs()
{
	std::unique_ptr<std::vector<std::string>> ret(new std::vector<std::string>);

	const char* argsEnd = curr;
	while (true) {
		readToNextLineText();

		if (curr >= end || *curr != '{')
			break;

		const char* argStart = ++curr; // Advance to the first character of the argument (after the '{')
		int braceLevel = 1;

		while (braceLevel > 0) {
			if (curr >= end)
				errorOnLine("End of file reached before finding end of argument");

			if (*curr == '\r' || *curr == '\n') {
				readNewline();
			}
			else {
				if (*curr == '{' && *(curr - 1) != '\\')
					++braceLevel;
				else if (*curr == '}' && *(curr - 1) != '\\')
					--braceLevel;

				++curr;
			}
		}
		ret->emplace_back(argStart, curr - 1);
		argsEnd = curr;
	}
	curr = argsEnd;
	return ret;
}

std::string Parser::getMostCommonNewline() const
{
	if (unixNewlines >= windowsNewlines && unixNewlines >= macNewlines)
		return "\n";
	else if (windowsNewlines >= macNewlines)
		return "\r\n";
	else
		return "\n";
}


void Parser::errorOnLine(const std::string& msg) const
{
		std::stringstream err;
		err << filename << ":" << currLine << ": error: " << msg;
		throw Exceptions::InvalidInputException(err.str(), __FUNCTION__);
}

void Parser::warningOnLine(const std::string& msg) const
{
		std::stringstream err;
		err << filename << ":" << currLine << ": warning: " << msg;
		printf("%s\n", err.str().c_str());
}
