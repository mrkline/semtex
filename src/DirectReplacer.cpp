#include "precomp.hpp"

#include "DirectReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

std::vector<std::string> DirectReplacer::getReplacementKeys()
{
	std::vector<std::string> ret;
	for (const auto& kp : replacements) {
		printf("Wat\n");
		ret.push_back(kp.first);
	}
	return ret;
}

DirectReplacer::DirectReplacer()
	: Replacer(getReplacementKeys())
{ }

void DirectReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();

	pi.replacements.emplace_back(start, pi.curr, replacements.at(matchedKey));
}
