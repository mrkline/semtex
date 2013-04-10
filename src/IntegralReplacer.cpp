#include "precomp.hpp"

#include "IntegralReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

static std::unordered_set<std::string> acceptedFlags({"inf", "lim"});

IntegralReplacer::IntegralReplacer()
	: Replacer({"\\integral"})
{ }

void IntegralReplacer::replace(const std::string& matchedKey, Parser& p)
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
		p.errorOnLine("\\integral does not take options");

	for (const auto& flag : options->flags) {
		if (acceptedFlags.find(flag) == acceptedFlags.end())
			p.errorOnLine("Unknown argument \"" + flag + "\" for \\integral");
	}

	const size_t numArgs = argList->size();

	if (numArgs > 4)
		p.errorOnLine("Too many arguments for \\integral");

	bool inf = options->flags.find("inf") != options->flags.end();
	bool lim = options->flags.find("lim") != options->flags.end();

	// Arg 0 is the expression
	// Arg 1 is with respect to (d_)
	// Arg 2 is the lower bound
	// Arg 3 is the upper bound

	std::string replacement = "\\int";
	if ((numArgs >= 3 || inf) && lim)
		replacement += "\\limits";

	if (numArgs >= 3 && !argList->at(2).empty())
		replacement += "_{" + argList->at(2) + "}";
	else if (inf)
		replacement += "_{-\\infty}";

	if (numArgs >= 4 && !argList->at(3).empty())
		replacement += "^{" + argList->at(3) + "}";
	else if (inf)
		replacement += "^{\\infty}";

	replacement += " " + argList->at(0);

	if (numArgs >= 2)
		replacement += "\\,\\mathrm{d}" + argList->at(1);

	p.replacements.emplace_back(start, p.curr, std::move(replacement));
}
