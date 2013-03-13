#include "IntegralReplacer.hpp"

#include <algorithm>
#include <unordered_set>

#include "ErrorHandling.hpp"
#include "Exceptions.hpp"
#include "FileParser.hpp"

static std::unordered_set<std::string> acceptedArgs = {{"expr", "wrt", "lower", "upper", "inf"}};

IntegralReplacer::IntegralReplacer()
	: Replacer({"\\integral"})
{ }

void IntegralReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();
	std::unique_ptr<MacroArgs> args;
	try {
		args = parseArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\integral", __FUNCTION__);
	}

	if (args->unnamed.size() > 4)
		errorOnLine(pi, "Incorrect number of unnamed arguments for \\integral");

	if (std::any_of(args->named.begin(), args->named.end(),
	                [](const decltype(args->named)::value_type& t)
	                { return acceptedArgs.find(t.first) == acceptedArgs.end(); }))
		errorOnLine(pi, "Unknown argument for \\integral");

	std::string* expr = nullptr;
	std::string* wrt = nullptr;
	std::string* lower = nullptr;
	std::string* upper = nullptr;
	bool inf = false;

	switch (args->unnamed.size()) {
		case 4:
			upper = &args->unnamed[3];
		case 3:
			lower = &args->unnamed[2];
		case 2:
			wrt = &args->unnamed[1];
		case 1:
			expr = &args->unnamed[0];
	}

	const auto& exprIt = args->named.find("expr");
	const auto& wrtIt = args->named.find("wrt");
	const auto& lowerIt = args->named.find("lower");
	const auto& upperIt = args->named.find("upper");
	const auto& infIt = args->named.find("inf");

	const auto& ei = args->named.end();

	if (exprIt != ei) {
		if (expr != nullptr)
			errorOnLine(pi, "Duplicate expression argument for \\integral");
		else
			expr = &exprIt->second;
	}

	if (wrtIt != ei) {
		if (wrt != nullptr)
			errorOnLine(pi, "Duplicate \"with respect to\" argument for \\integral");
		else
			wrt = &wrtIt->second;
	}

	if (lowerIt != ei) {
		if (lower != nullptr)
			errorOnLine(pi, "Duplicate lower bound argument for \\integral");
		else
			lower = &lowerIt->second;
	}

	if (upperIt != ei) {
		if (upper != nullptr)
			errorOnLine(pi, "Duplicate upper bound argument for \\integral");
		else
			upper = &upperIt->second;
	}

	if (infIt != ei) {
		try {
			inf = getStringTruthValue(pi, infIt->second);
		}
		catch (const Exceptions::InvalidInputException& ex) {
			throw Exceptions::InvalidInputException(ex.message + " \"inf\" in \\integral", __FUNCTION__);
		}
	}

	if (expr == nullptr)
		errorOnLine(pi, "Missing mandatory expression argument for \\integral");

	std::string replacement = "\\int";
	if (lower != nullptr)
		replacement += "_{" + *lower + "}";
	else if (inf)
		replacement += "_{-\\infty}";

	if (upper != nullptr)
		replacement += "^{" + *upper + "}";
	else if (inf)
		replacement += "^{\\infty}";

	replacement += " " + *expr;

	if (wrt != nullptr)
		replacement += "\\,\\mathrm{d}" + *wrt;

	pi.replacements.emplace_back(start, pi.curr, std::move(replacement));
}
