#ifndef __PIECEWISE_REPLACER_H__
#define __PIECEWISE_REPLACER_H__

#include "Replacer.hpp"

class PiecewiseReplacer final : public Replacer {
public:
	PiecewiseReplacer();

	void replace(const std::string& matchedKey, Parser& p) override;

	bool shouldRecurse() const override { return true; }

private:
	static std::string parsePiece(Parser& p);
};

#endif
