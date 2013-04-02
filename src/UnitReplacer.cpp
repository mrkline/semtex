#include "precomp.hpp"

#include "UnitReplacer.hpp"

#include "ErrorHandling.hpp"
#include "Exceptions.hpp"
#include "FileParser.hpp"

UnitReplacer::UnitReplacer()
	: Replacer({"\\unit"})
{ }

void UnitReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
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
		errorOnLine(pi, "\\unit does not take options");

	if (options->flags.size() != 0)
		errorOnLine(pi, "\\unit does not take flags");

	if (argList->size() != 1)
		errorOnLine(pi, "Incorrect argument(s) for \\unit, which takes a single argument");

	pi.replacements.emplace_back(start, pi.curr, "\\,\\mathrm{" + argList->at(0) + "}");
}
