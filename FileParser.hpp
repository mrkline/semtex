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
	std::string replaceWith; //!< Replacement string
};

//! Returned from parseArgs
struct MacroArgs {
	std::vector<std::string> unlabeled;
	std::unordered_map<std::string, std::string> labeled;
};

struct ParseInfo {
	const std::string filename;
	const char* curr;
	const char* end;
	int currLine;
	int unixNewlines;
	int windowsNewlines;
	int macNewlines;
	Context& ctxt;

	//! \todo Would a linked list run faster?
	std::vector<Replacement> replacements;

	ParseInfo(const std::string& file, const char* current, const char* end, Context& context)
		: filename(file), curr(current), end(end), currLine(1),
		  unixNewlines(0), windowsNewlines(0), macNewlines(0), ctxt(context)
	{ }
};

//! All replacement generators should take this form:
typedef Replacement (*ReplacementGenerator)(ParseInfo& pi);

/*!
 * \brief Processes a SemTeX file, generating a corresponding LaTeX file and adding included SemTeX files
 *        to the queue
 * \param filename The path of the SemTeX file to process
 * \param context The global context (verbosity level, queues, etc.)
 * \returns true on success, false on some error
 */
bool processFile(const std::string& filename, Context& ctxt);

//! Tries to read a newline at the current location
//! \returns true if a newline was read
bool readNewline(ParseInfo& pi);

//! Called when we hit a \include or \input
//! When the function returns, pi.curr is moved past the \include statement
void processInclude(ParseInfo& pi);

/*!
 * \brief Parses SemTeX args (e.g. \macro{args}).
 * \returns A heap-allocated MacroArgs struct containing the arguments
 *
 * When the function returns, pi.curr is moved past the arguments
 */
std::unique_ptr<MacroArgs> parseArgs(ParseInfo& pi);
