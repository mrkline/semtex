#ifndef __DERIV_REPLACER_HPP__
#define __DERIV_REPLACER_HPP__

#include "Replacer.hpp"

class DerivReplacer final : public Replacer {
public:
	DerivReplacer();

	void replace(const std::string& matchedKey, Parser& p) override;

	bool shouldRecurse() const override { return false;}
};

#endif
