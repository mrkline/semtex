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
	std::unique_ptr<MacroArgs> args;
	try {
		args = parseArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\unit", __FUNCTION__);
	}
	std::string* unit;
	if (args->unnamed.size() == 1 && args->named.size() == 0)
		unit = &args->unnamed[0];
	else if (args->unnamed.size() == 0 && args->named.size() == 1 && args->named.find("u") != args->named.end())
		unit = &args->named["u"];
	else
		errorOnLine(pi, "Incorrect argument(s) for \\unit.\n\t\\unit takes a single argument, optionally named \"u\".");

	const char* end = pi.curr;
	pi.replacements.emplace_back(start, end, "\\,\\mathrm{" + *unit + "}");
}
