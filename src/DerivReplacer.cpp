#include "precomp.hpp"

#include "DerivReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

DerivReplacer::DerivReplacer()
	: Replacer({"\\deriv"})
{ }

void DerivReplacer::replace(const std::string& matchedKey, Parser& p)
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

	const size_t numArgs = argList->size();

	if (numArgs < 1)
		p.errorOnLine("\\unit needs at least one argument");

	if (numArgs > 3)
		p.errorOnLine("\\unit only takes one to three arguments");

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

	p.replacements.emplace_back(start, p.curr, std::move(replacement));
}
