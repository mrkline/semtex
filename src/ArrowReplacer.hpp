#ifndef __ARROW_REPLACER_HPP__
#define __ARROW_REPLACER_HPP__

#include "Replacer.hpp"

//! Replaces \\integral{args} with a properly formatted LaTeX integral
class ArrowReplacer final : public Replacer {
public:
	ArrowReplacer();

	void replace(const std::string& matchedKey, ParseInfo& pi) override;

	bool shouldRecurse() const override { return false; }
};

#endif
