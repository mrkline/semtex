#include "precomp.hpp"

#include "SummationReplacer.hpp"

#include "ErrorHandling.hpp"
#include "Exceptions.hpp"
#include "FileParser.hpp"

static std::unordered_set<std::string> acceptedArgs = {{"expr", "wrt", "from", "to", "inf"}};

SummationReplacer::SummationReplacer()
	: Replacer({"\\summ"})
{ }

void SummationReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();
	std::unique_ptr<MacroArgs> args;
	try {
		args = parseArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\summ", __FUNCTION__);
	}

	if (args->unnamed.size() > 4)
		errorOnLine(pi, "Incorrect number of unnamed arguments for \\summ");

	for (const auto& arg : args->named) {
		if (acceptedArgs.find(arg.first) == acceptedArgs.end())
			errorOnLine(pi, "Unknown argument \"" + arg.first + "\" for \\summ");
	}

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
	const auto& lowerIt = args->named.find("from");
	const auto& upperIt = args->named.find("to");
	const auto& infIt = args->named.find("inf");

	const auto& ei = args->named.end();

	if (exprIt != ei) {
		if (expr != nullptr)
			errorOnLine(pi, "Duplicate expression argument for \\summ");
		else
			expr = &exprIt->second;
	}

	if (wrtIt != ei) {
		if (wrt != nullptr)
			errorOnLine(pi, "Duplicate \"with respect to\" argument for \\summ");
		else
			wrt = &wrtIt->second;
	}

	if (lowerIt != ei) {
		if (lower != nullptr)
			errorOnLine(pi, "Duplicate lower bound argument for \\summ");
		else
			lower = &lowerIt->second;
	}

	if (upperIt != ei) {
		if (upper != nullptr)
			errorOnLine(pi, "Duplicate upper bound argument for \\summ");
		else
			upper = &upperIt->second;
	}

	if (infIt != ei) {
		try {
			inf = getStringTruthValue(pi, infIt->second);
		}
		catch (const Exceptions::InvalidInputException& ex) {
			throw Exceptions::InvalidInputException(ex.message + " \"inf\" in \\summ", __FUNCTION__);
		}
	}

	if (expr == nullptr)
		errorOnLine(pi, "Missing mandatory expression argument for \\summ");

	std::string replacement = "\\sum";
	if (lower != nullptr)
		replacement += "_{" + (wrt != nullptr ? *wrt + "=" : "") + *lower + "}";
	else if (inf)
		replacement += "_{"+ (wrt != nullptr ? *wrt + "=" : "") + "-\\infty}";
	else if (wrt != nullptr)
		replacement += "_{" + *wrt + "}";

	if (upper != nullptr)
		replacement += "^{" + *upper + "}";
	else if (inf)
		replacement += "^{\\infty}";

	replacement += " " + *expr;

	pi.replacements.emplace_back(start, pi.curr, std::move(replacement));
}
