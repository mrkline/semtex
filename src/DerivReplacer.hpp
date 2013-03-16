#pragma once

#include "Replacer.hpp"

class DerivReplacer final : public Replacer {
public:
	DerivReplacer();

	void replace(const std::string& matchedKey, ParseInfo& pi) override;

	bool shouldRecurse() const override { return false;}
};
