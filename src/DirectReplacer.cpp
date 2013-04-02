#include "precomp.hpp"

#include "DirectReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

namespace {
	const std::unordered_map<std::string, std::string> arrows({{"<--","\\leftarrow"}, {"-->","\\rightarrow"},
	                                                           {"<==","\\Leftarrow"}, {"==>","\\Rightarrow"},
	                                                           {"<-->","\\leftrightarrow"},
	                                                           {"<==>","\\Leftrightarrow"},
	                                                           {"!=","\\neq"},
	                                                           {">=","\\geq"},
	                                                           {"<=","\\leq"}});
}

DirectReplacer::DirectReplacer()
	: Replacer({"<--", "-->", "<==", "==>", "<-->", "<==>", "!=", ">=", "<="})
{ }

void DirectReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();

	pi.replacements.emplace_back(start, pi.curr, arrows.at(matchedKey));
}
