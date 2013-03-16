#pragma once

#include <string>
#include <vector>

class ParseInfo;

//! Abstract base class for replacement generators
class Replacer {
public:
	//! Constructor. Takes a list of keys to initialize keyList with
	Replacer(std::initializer_list<std::string> keys) : keyList(keys) { }

	virtual ~Replacer() { }

	/*!
	 * \brief Performs a replacement, or does nothing
	 * \param matchedKey the key from Replacer::getKeys that was matched
	 * \param pi ParseInfo for the current file (or replacement, in the case of recursion)
	 */
	virtual void replace(const std::string& matchedKey, ParseInfo& pi) = 0;

	/*!
	 * \brief Gets a list of tokens that should start a replacement of this type
	 *
	 * Constructors should initialize keyList to whatever they want
	 */
	const std::vector<std::string>& getKeys() const { return keyList; };

	//! Returns true if the generated replacement can contain replacements itself
	virtual bool shouldRecurse() const = 0;

protected:
	std::vector<std::string> keyList;
};
