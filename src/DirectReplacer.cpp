#include "precomp.hpp"

#include "DirectReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

DirectReplacer::DirectReplacer()
	: replacements(
		 // Relationsl operators
		{{"<--","\\leftarrow"}, {"-->","\\rightarrow"},
		 {"<==","\\Leftarrow"}, {"==>","\\Rightarrow"},
		 {"<-->","\\leftrightarrow"}, {"<==>","\\Leftrightarrow"},
		 {"!=","\\neq"}, {">=","\\geq"}, {"<=","\\leq"},
		 // Greek letters
		 {"\"t", "\\theta"},
		 {"\"w", "\\omega"},
		 {"\"e", "\\varepsilon"},
		 {"\"r", "\\rho"},
		 {"\"t", "\\tau"},
		 {"\"y", "\\psi"},
		 {"\"u", "\\upsilon"},
		 {"\"p", "\\pi"},
		 {"\"a", "\\alpha"},
		 {"\"s", "\\sigma"},
		 {"\"d", "\\delta"},
		 {"\"f", "\\varphi"},
		 {"\"g", "\\gamma"},
		 {"\"h", "\\eta"},
		 {"\"k", "\\kappa"},
		 {"\"l", "\\lambda"},
		 {"\"z", "\\zeta"},
		 {"\"x", "\\xi"},
		 {"\"c", "\\chi"},
		 {"\"v", "\\varsigma"},
		 {"\"b", "\\beta"},
		 {"\"n", "\\nu"},
		 {"\"m", "\\mu"},
		 {"\"Q", "\\Theta"},
		 {"\"W", "\\Omega"},
		 {"\"Y", "\\Psi"},
		 {"\"U", "\\Upsilon"},
		 {"\"S", "\\Sigma"},
		 {"\"D", "\\Delta"},
		 {"\"F", "\\Phi"},
		 {"\"G", "\\Gamma"},
		 {"\"L", "\\Lambda"},
		 {"\"X", "\\Xi"},
		 // Math functions
		 {"\\sinc", "\\mathrm{sinc}"}})
{
	for (const auto& kp : replacements)
		keySet.emplace(kp.first);
}

void DirectReplacer::replace(const std::string& matchedKey, Parser& p)
{
	const char* start = p.curr;
	p.curr += matchedKey.length();

	p.replacements.emplace_back(start, p.curr, replacements.at(matchedKey));
}
