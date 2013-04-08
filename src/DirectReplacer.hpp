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
	const std::unordered_map<std::string, std::string> replacements;
};

#endif
