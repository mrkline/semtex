#ifndef __DIRECT_REPLACER_HPP__
#define __DIRECT_REPLACER_HPP__

#include "Replacer.hpp"

//! Makes simple direct swaps (!= to \\neq, >= to \\geq, etc.)
class DirectReplacer final : public Replacer {
public:
	DirectReplacer();

	void replace(const std::string& matchedKey, ParseInfo& pi) override;

	bool shouldRecurse() const override { return false; }

private:
	const std::unordered_map<std::string, std::string> replacements =
		{{"<--","\\leftarrow"}, {"-->","\\rightarrow"},
		 {"<==","\\Leftarrow"}, {"==>","\\Rightarrow"},
		 {"<-->","\\leftrightarrow"}, {"<==>","\\Leftrightarrow"},
		 {"!=","\\neq"}, {">=","\\geq"}, {"<=","\\leq"},
		 {"'t", "\\theta"},
		 {"'w", "\\wedge"},
		 {"'e", "\\varepsilon"},
		 {"'r", "\\rho"},
		 {"'t", "\\tau"},
		 {"'y", "\\psi"},
		 {"'u", "\\upsilon"},
		 {"'p", "\\pi"},
		 {"'a", "\\alpha"},
		 {"'s", "\\sigma"},
		 {"'d", "\\delta"},
		 {"'f", "\\varphi"},
		 {"'g", "\\gamma"},
		 {"'h", "\\eta"},
		 {"'k", "\\kappa"},
		 {"'l", "\\lambda"},
		 {"'z", "\\zeta"},
		 {"'x", "\\xi"},
		 {"'c", "\\chi"},
		 {"'v", "\\varsigma"},
		 {"'b", "\\beta"},
		 {"'n", "\\nu"},
		 {"'m", "\\mu"},
		 {"'Q", "\\Theta"},
		 {"'W", "\\Omega"},
		 {"'Y", "\\Psi"},
		 {"'U", "\\Upsilon"},
		 {"'S", "\\Sigma"},
		 {"'D", "\\Delta"},
		 {"'F", "\\Phi"},
		 {"'G", "\\Gamma"},
		 {"'L", "\\Lambda"},
		 {"'X", "\\Xi"}};

	std::vector<std::string> getReplacementKeys();
};

#endif
