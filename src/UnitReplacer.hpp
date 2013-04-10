#ifndef __UNIT_REPLACER_HPP__
#define __UNIT_REPLACER_HPP__

#include "Replacer.hpp"

//! Replaces "\unit{foo}" with "\,\mathrm{foo}"
class UnitReplacer final : public Replacer {
public:
	UnitReplacer();

	void replace(const std::string& matchedKey, Parser& p) override;

	// Debatable if we should allow for replacements in units, but allow it for now
	bool shouldRecurse() const override { return true; }
};

#endif
