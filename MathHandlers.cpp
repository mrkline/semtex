#include "MathHandlers.hpp"

void integralHandler(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();
	parseArgs(pi);
	const char* end = pi.curr;
	pi.replacements.emplace_back(start, end, "integral here");
}
