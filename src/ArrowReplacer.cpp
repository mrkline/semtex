#include "precomp.hpp"

#include "ArrowReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

namespace {
	const uint32_t leftID = '<' | '-' << 8;
	const uint32_t rightID = '-' | '>' << 8;
	
	uint32_t strToID(const std::string& str)
	{
		assert(str.length() <= 4);

		uint32_t ret = 0;

		for (size_t i = 0; i < str.length(); ++i)
			ret += (uint32_t)(str[i]) << (8 * i);

		return ret;
	}
}

ArrowReplacer::ArrowReplacer()
	: Replacer({"<-", "->"})
{ }

void ArrowReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();

	uint32_t id = strToID(matchedKey);

	std::string replacement;

	switch (id) {
		case leftID:
			replacement = "\\leftarrow";
			break;

		case rightID:
			replacement = "\\rightarrow";
			break;
	}

	pi.replacements.emplace_back(start, pi.curr, std::move(replacement));
}
