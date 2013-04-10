#include "precomp.hpp"

#include "UnitReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

UnitReplacer::UnitReplacer()
	: Replacer({"\\unit"})
{ }

void UnitReplacer::replace(const std::string& matchedKey, Parser& p)
{
	const char* start = p.curr;
	p.curr += matchedKey.length();

	std::unique_ptr<MacroOptions> options;
	decltype(p.parseBracketArgs()) argList;
	try {
		options = p.parseMacroOptions();
		argList = p.parseBracketArgs();
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\integral", __FUNCTION__);
	}

	if (options->opts.size() != 0)
		p.errorOnLine("\\unit does not take options");

	if (options->flags.size() != 0)
		p.errorOnLine("\\unit does not take flags");

	if (argList->size() != 1)
		p.errorOnLine("Incorrect argument(s) for \\unit, which takes a single argument");

	p.replacements.emplace_back(start, p.curr, "\\,\\mathrm{" + argList->at(0) + "}");
}
