#include "precomp.hpp"

#include "ArrowReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

namespace {
	const std::unordered_map<std::string, std::string> arrows({{"<-","\\leftarrow"}, {"->","\\rightarrow"},
	                                                           {"<=","\\Leftarrow"}, {"=>","\\Rightarrow"},
	                                                           {"<->","\\leftrightarrow"},
	                                                           {"<=>","\\Leftrightarrow"}});
}

ArrowReplacer::ArrowReplacer()
	: Replacer({"<-", "->", "<=", "=>", "<->", "<=>"})
{ }

void ArrowReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();

	pi.replacements.emplace_back(start, pi.curr, arrows.at(matchedKey));
}
