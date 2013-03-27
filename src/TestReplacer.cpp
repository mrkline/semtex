#include "precomp.hpp"

#include "TestReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

TestReplacer::TestReplacer()
	: Replacer({"\\test"})
{ }

void TestReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();

	std::unique_ptr<MacroOptions> options;
	decltype(parseBracketArgs(pi)) argList;
	try {
		options = parseMacroOptions(pi);
		argList = parseBracketArgs(pi);
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in \\test", __FUNCTION__);
	}

	for (const auto& opt : options->opts)
		printf("Option: %s=%s\n", opt.first.c_str(), opt.second.c_str());

	for (const auto& flag : options->flags)
		printf("Flag: %s\n", flag.c_str());

	for (const auto& arg : *argList)
		printf("Arg: %s\n", arg.c_str());

	pi.replacements.emplace_back(start, pi.curr, "");
}
