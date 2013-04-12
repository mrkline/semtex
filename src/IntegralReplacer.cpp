#include "precomp.hpp"

#include "IntegralReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

static std::unordered_set<std::string> acceptedFlags({"inf", "lim", "mir"});

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
	bool mir = options->flags.find("mir") != options->flags.end();

	// Arg 0 is the expression
	// Arg 1 is with respect to (d_)
	// Arg 2 is the lower bound
	// Arg 3 is the upper bound
	const std::string* expr = numArgs >= 1 && !argList->at(0).empty() ? &argList->at(0) : nullptr;
	const std::string* wrt = numArgs >= 2 && !argList->at(1).empty() ? &argList->at(1) : nullptr;
	const std::string* lower = numArgs >= 3 && !argList->at(2).empty() ? &argList->at(2) : nullptr;
	const std::string* upper = numArgs >= 4 && !argList->at(3).empty() ? &argList->at(3) : nullptr;

	if (mir && upper != nullptr)
		p.warningOnLine("\\integral is ignoring the \"mirror bounds\" option since two bounds were provided.");

	if (!mir && inf && upper != nullptr && lower != nullptr)
		p.warningOnLine("\\integral is ignoring the \"infinity bounds\" option since two bounds were provided.");

	std::string replacement = "\\int";
	if ((lower != nullptr || upper != nullptr || inf) && lim)
		replacement += "\\limits";

	if (lower != nullptr)
		replacement += "_{" + ((mir ? "-" : "") + *lower) + "}";
	else if (inf)
		replacement += "_{-\\infty}";

	if (upper != nullptr)
		replacement += "^{" + *upper + "}";
	else if (mir && lower != nullptr)
		replacement +=  "^{" + *lower + "}";
	else if (inf)
		replacement += "^{\\infty}";

	if (expr != nullptr)
		replacement += " " + *expr;

	if (wrt != nullptr)
		replacement += "\\,\\mathrm{d}" + *wrt;

	p.replacements.emplace_back(start, p.curr, std::move(replacement));
}
