#pragma once

#include "Replacer.hpp"

//! Replaces \\integral{args} with a properly formatted LaTeX integral
class TestReplacer final : public Replacer {
public:
	TestReplacer();

	void replace(const std::string& matchedKey, ParseInfo& pi) override;

	bool shouldRecurse() const override { return false; }
};
