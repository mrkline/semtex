#include "precomp.hpp"

#include "DerivReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

DerivReplacer::DerivReplacer()
	: Replacer({"\\deriv"})
{ }

void DerivReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
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

	const size_t numArgs = argList->size();

	if (numArgs < 1)
		pi.errorOnLine("\\unit needs at least one argument");

	if (numArgs > 3)
		pi.errorOnLine("\\unit only takes one to three arguments");

	std::string replacement = "\\frac{\\mathrm{d}";

	switch (numArgs) {
		case 3:
			replacement += "^{" + argList->at(2) + "} " + argList->at(0) + "}";
			replacement += "{\\mathrm{d} " + argList->at(1) + "^{" + argList->at(2) + "}}";
			break;

		case 2:
			replacement += " " + argList->at(0) + "}";
			replacement += "{\\mathrm{d} " + argList->at(1) + "}";
			break;

		case 1:
			replacement += "}";
			replacement += "{\\mathrm{d} " + argList->at(0) + "}";
			break;
	}

	pi.replacements.emplace_back(start, pi.curr, std::move(replacement));
}
