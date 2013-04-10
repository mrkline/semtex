#ifndef __INTEGRAL_REPLACER_HPP__
#define __INTEGRAL_REPLACER_HPP__

#include "Replacer.hpp"

//! Replaces \\integral{args} with a properly formatted LaTeX integral
class IntegralReplacer final : public Replacer {
public:
	IntegralReplacer();

	void replace(const std::string& matchedKey, Parser& p) override;

	bool shouldRecurse() const override { return true; }
};

#endif
