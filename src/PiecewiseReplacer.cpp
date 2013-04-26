#include "precomp.hpp"

#include "PiecewiseReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

namespace {
	const std::string rbraceKey = "\\rightbrace";
	const std::string endKey = "\\end{piecewise}";
	const std::string pieceKey = "\\piece";
}

PiecewiseReplacer::PiecewiseReplacer()
	: Replacer({"\\begin{piecewise}"})
{ }

void PiecewiseReplacer::replace(const std::string& matchedKey, Parser& p)
{
	const char* start = p.curr;
	p.curr += matchedKey.length();

	std::unique_ptr<MacroOptions> options;
	decltype(p.parseBracketArgs()) argList;
	try {
		options = p.parseMacroOptions();
		argList = p.parseBracketArgs();
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in " + matchedKey, __FUNCTION__);
	}

	if (options->opts.size() != 0)
		p.errorOnLine(matchedKey + " does not take options");

	if (options->flags.size() != 0)
		p.errorOnLine(matchedKey + " does not take flags");

	const size_t numArgs = argList->size();

	if (numArgs > 1)
		p.errorOnLine("Too many arguments for \\begin{piecewise}");

	bool rightBraceSeen = false;

	std::string replacement;

	if (numArgs == 1)
		replacement += argList->at(0) + " = ";

	replacement += "\\left\\{\\begin{array}{l l}\n";

	while (true) {
		p.readToNextLineText();
		if (p.curr >= p.end || std::distance(p.curr, p.end) <= (int)pieceKey.length())
			p.errorOnLine("End of file reached before reaching end of \"piecewise\" definition");

		if (strncmp(p.curr, endKey.c_str(), endKey.length()) == 0) {
			p.curr += endKey.length();
			break;
		}
		if (strncmp(p.curr, rbraceKey.c_str(), rbraceKey.length()) == 0) {
			if (rightBraceSeen)
				p.errorOnLine("\"" + rbraceKey + "\" seen twice (only needed once)");
			rightBraceSeen = true;
			p.curr += rbraceKey.length();
			continue;
		}
		replacement += parsePiece(p);
	}
	replacement += "\\end{array}\\right";
	replacement += (rightBraceSeen ? "\\}" : ".");

	p.replacements.emplace_back(start, p.curr, std::move(replacement));
}

std::string PiecewiseReplacer::parsePiece(Parser& p)
{
	if (strncmp(p.curr, pieceKey.c_str(), pieceKey.length()) != 0)
		p.errorOnLine("Expected a \"\\piece\" inside piecewise definition");

	p.curr += pieceKey.length();

	std::unique_ptr<MacroOptions> options;
	decltype(p.parseBracketArgs()) argList;
	try {
		options = p.parseMacroOptions();
		argList = p.parseBracketArgs();
	}
	catch (const Exceptions::InvalidInputException& ex) {
		throw Exceptions::InvalidInputException(ex.message + " in " + pieceKey, __FUNCTION__);
	}

	if (options->opts.size() != 0)
		p.errorOnLine(pieceKey + " does not take options");

	if (options->flags.size() != 0)
		p.errorOnLine(pieceKey + " does not take flags");

	const size_t numArgs = argList->size();

	if (numArgs < 1)
		p.errorOnLine(pieceKey + " needs at least one argument");

	if (numArgs > 2)
		p.errorOnLine(pieceKey + " only takes one or two arguments");

	std::string piece = "\t";

	piece += argList->at(0) + ", & ";

	if (numArgs == 2)
		piece += argList->at(1) + " ";

	piece += "\\\\\n";

	return piece;
}
