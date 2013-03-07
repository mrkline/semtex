#pragma once

#include "Replacer.hpp"

//! Replaces "\unit{foo}" with "\,\mathrm{foo}"
class UnitReplacer : public Replacer {
public:
	UnitReplacer();

	virtual void replace(const std::string& matchedKey, ParseInfo& pi) override;

	// Debatable if we should allow for replacements in units, but allow it for now
	virtual bool shouldRecurse() const override { return true; }
};
