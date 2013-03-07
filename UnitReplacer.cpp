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
	auto args = parseArgs(pi);
	std::string* unit;
	if (args->unnamed.size() == 1 && args->named.size() == 0)
		unit = &args->unnamed[0];
	else if (args->unnamed.size() == 0 && args->named.size() == 1 && args->named.find("unit") != args->named.end())
		unit = &args->named["unit"];
	else
		throw Exceptions::InvalidInputException("Incorrect argument(s) for \\unit{}", __FUNCTION__);
	const char* end = pi.curr;
	pi.replacements.emplace_back(start, end, "\\,\\mathrm{" + *unit + "}");
}
