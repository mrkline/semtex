#include "precomp.hpp"

#include "UnitReplacer.hpp"

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
	decltype(pi.parseBracketArgs()) argList;
	try {
		options = pi.parseMacroOptions();
		argList = pi.parseBracketArgs();
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\integral", __FUNCTION__);
	}

	if (options->opts.size() != 0)
		pi.errorOnLine("\\unit does not take options");

	if (options->flags.size() != 0)
		pi.errorOnLine("\\unit does not take flags");

	if (argList->size() != 1)
		pi.errorOnLine("Incorrect argument(s) for \\unit, which takes a single argument");

	pi.replacements.emplace_back(start, pi.curr, "\\,\\mathrm{" + argList->at(0) + "}");
}
