#pragma once

#include "Replacer.hpp"

//! Replaces \integral{<args>} with a properly formatted LaTeX integral
class SummationReplacer final : public Replacer {
public:
	SummationReplacer();

	void replace(const std::string& matchedKey, ParseInfo& pi) override;

	bool shouldRecurse() const override { return true; }
};
