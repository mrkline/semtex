#include "precomp.hpp"

#include "DirectReplacer.hpp"

#include "Exceptions.hpp"
#include "FileParser.hpp"

DirectReplacer::DirectReplacer()
	: replacements(
		{{"<--","\\leftarrow"}, {"-->","\\rightarrow"},
		 {"<==","\\Leftarrow"}, {"==>","\\Rightarrow"},
		 {"<-->","\\leftrightarrow"}, {"<==>","\\Leftrightarrow"},
		 {"!=","\\neq"}, {">=","\\geq"}, {"<=","\\leq"},
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
		 {"\"X", "\\Xi"}})
{
	for (const auto& kp : replacements)
		keySet.emplace(kp.first);
}

void DirectReplacer::replace(const std::string& matchedKey, ParseInfo& pi)
{
	const char* start = pi.curr;
	pi.curr += matchedKey.length();

	pi.replacements.emplace_back(start, pi.curr, replacements.at(matchedKey));
}
