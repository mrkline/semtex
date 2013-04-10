#ifndef __TEST_REPLACER_HPP__
#define __TEST_REPLACER_HPP__

#include "Replacer.hpp"

//! Replaces \\integral{args} with a properly formatted LaTeX integral
class TestReplacer final : public Replacer {
public:
	TestReplacer();

	void replace(const std::string& matchedKey, Parser& p) override;

	bool shouldRecurse() const override { return false; }
};

#endif
