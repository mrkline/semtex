#include "DerivReplacer.hpp"

#include <unordered_set>

#include "ErrorHandling.hpp"
#include "Exceptions.hpp"

static std::unordered_set<std::string> acceptedArgs = {{"wrt", "of", "n"}};

DerivReplacer::DerivReplacer()
	: Replacer({"\\deriv"})
{ }

void DerivReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();
	std::unique_ptr<MacroArgs> args;
	try {
		args = parseArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\deriv", __FUNCTION__);
	}

	if (args->unnamed.size() > 3)
		errorOnLine(pi, "Incorrect number of unnamed arguments for \\deriv");

	for (const auto& arg : args->named) {
		if (acceptedArgs.find(arg.first) == acceptedArgs.end())
			errorOnLine(pi, "Unknown argument \"" + arg.first + "\" for \\integral");
	}

	std::string* wrt = nullptr;
	std::string* of = nullptr;
	std::string* n = nullptr;

	switch (args->unnamed.size()) {
		case 3:
			n = &args->unnamed[3];
		case 2:
			of = &args->unnamed[2];
		case 1:
			wrt = &args->unnamed[1];
	}

	const auto wrtIt = args->named.find("wrt");
	const auto ofIt = args->named.find("of");
	const auto nIt = args->named.find("n");

	const auto ei = args->named.end();

	if (wrtIt != ei) {
		if (wrt != nullptr)
			errorOnLine(pi, "Duplicate \"with respect to\" argument for \\deriv");
		else
			wrt = & wrtIt->second;
	}

	if (ofIt != ei) {
		if (of != nullptr)
			errorOnLine(pi, "Duplicate \"of\" argument for \\deriv");
		else
			of = &ofIt->second;
	}

	if (nIt != ei) {
		if (n != nullptr)
			errorOnLine(pi, "Duplicate \"n\" argument for \\deriv");
		else
			n = &nIt->second;
	}

	if (wrt == nullptr)
		errorOnLine(pi, "Missing mandatory \"of\" argument for \\deriv");

	std::string replacement = "\\frac{\\mathrm{d}";

	if (n != nullptr)
		replacement += "^{" + *n + "}";

	if (of != nullptr)
		replacement += *of;

	replacement += "}{\\mathrm{d}" + *wrt;

	if (n != nullptr)
		replacement += "^{" + *n + "}";

	replacement += "}";
	pi.replacements.emplace_back(start, pi.curr, std::move(replacement));
}
