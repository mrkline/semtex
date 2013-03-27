#ifndef __SUMMATION_REPLACER_HPP__
#define __SUMMATION_REPLACER_HPP__

#include "Replacer.hpp"

//! Replaces \\summ{args} with a properly formatted LaTeX summation
class SummationReplacer final : public Replacer {
public:
	SummationReplacer();

	void replace(const std::string& matchedKey, ParseInfo& pi) override;

	bool shouldRecurse() const override { return true; }
};

#endif
