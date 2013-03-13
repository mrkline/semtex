#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Context;

//! Contains the location of where to insert a replacement, and where to put it
struct Replacement {
	const char* start = nullptr; //!< Where to start the replacement
	const char* end = nullptr; //!< One byte past the end of the string to be replaced
	std::string replaceWith; //!< Immutable replacement string

	Replacement(const char* s, const char* e, std::string&& r)
		: start(s), end(e), replaceWith(r)
	{ }

	Replacement(Replacement&& m) = default;
};

//! Returned from parseArgs
struct MacroArgs {
	std::vector<std::string> unnamed;
	std::unordered_map<std::string, std::string> named;
};

struct ParseInfo {
	const std::string filename;
	const char* curr;
	const char* const end;
	int currLine;
	int unixNewlines;
	int windowsNewlines;
	int macNewlines;
	Context& ctxt;

	//! \todo Would a linked list run faster?
	std::vector<Replacement> replacements;

	ParseInfo(const std::string& file, const char* current, const char* end, Context& context, int startingLine = 1)
		: filename(file), curr(current), end(end), currLine(startingLine),
		  unixNewlines(0), windowsNewlines(0), macNewlines(0), ctxt(context)
	{ }
};

/*!
 * \brief Used to parse a true or false value (usually from an argument)
 * \param pi Used to get the current line number for possible error reporting
 * \param str The string to examine
 * \returns true for strings like "true", "True", "TRUE", "t", "T", "y", "Y", "yes", "Yes", "1", or
 *          false for strings like "false", "False", "FALSE", "f", "F", "n", "N", "no", "No", "0"
 * \throws InvalidInputException if the string matches neither of these groups
 */
bool getStringTruthValue(const ParseInfo& pi, const std::string& str);

/*!
 * \brief Processes a SemTeX file, generating a corresponding LaTeX file and adding included SemTeX files
 *        to the queue
 * \param filename The path of the SemTeX file to process
 * \param context The global context (verbosity level, queues, etc.)
 */
void processFile(const std::string& filename, Context& ctxt);

//! The loop that pareses through an entire character sequence specified by the provided ParseInfo
void parseLoop(ParseInfo& pi, bool createReplacements);

//! Reads tabs and spaces until a non-whitespace character or a newline is hit
inline void eatWhitespace(ParseInfo& pi)
{
	while (pi.curr < pi.end && std::isblank(*pi.curr))
		++pi.curr;
}

//! Tries to read a newline at the current location
//! \returns true if a newline was read
bool readNewline(ParseInfo& pi);

//! Called when we hit a \include or \input
//! When the function returns, pi.curr is moved past the \include statement
void processInclude(ParseInfo& pi);

/*!
 * \brief Parses SemTeX args (e.g. \macro{args}).
 * \returns A heap-allocated MacroArgs struct containing the arguments
 * \todo Would stack allocation be better?
 *
 * When the function returns, pi.curr is moved past the arguments
 */
std::unique_ptr<MacroArgs> parseArgs(ParseInfo& pi);
