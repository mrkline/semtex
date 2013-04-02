#include "precomp.hpp"

#include "IntegralReplacer.hpp"

#include "ErrorHandling.hpp"
#include "Exceptions.hpp"
#include "FileParser.hpp"

static std::unordered_set<std::string> acceptedFlags = {{"inf"}};

IntegralReplacer::IntegralReplacer()
	: Replacer({"\\integral"})
{ }

void IntegralReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();

	std::unique_ptr<MacroOptions> options;
	decltype(parseBracketArgs(pi)) argList;
	try {
		options = parseMacroOptions(pi);
		argList = parseBracketArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\integral", __FUNCTION__);
	}

	if (options->opts.size() != 0)
		errorOnLine(pi, "\\integral does not take options");

	for (const auto& flag : options->flags) {
		if (acceptedFlags.find(flag) == acceptedFlags.end())
			errorOnLine(pi, "Unknown argument \"" + flag + "\" for \\integral");
	}

	const size_t numArgs = argList->size();

	if (numArgs > 4)
		errorOnLine(pi, "Too many arguments for \\integral");

	bool inf = options->flags.size() > 0;

	// Arg 0 is the expression
	// Arg 1 is with respect to (d_)
	// Arg 2 is the lower bound
	// Arg 3 is the upper bound

	std::string replacement = "\\int";
	if (numArgs >= 3)
		replacement += "_{" + argList->at(2) + "}";
	else if (inf)
		replacement += "_{-\\infty}";

	if (numArgs >= 4)
		replacement += "^{" + argList->at(3) + "}";
	else if (inf)
		replacement += "^{\\infty}";

	replacement += " " + argList->at(0);

	if (numArgs >= 2)
		replacement += "\\,\\mathrm{d}" + argList->at(1);

	pi.replacements.emplace_back(start, pi.curr, std::move(replacement));
}
