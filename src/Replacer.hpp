#ifndef __REPLACER_HPP__
#define __REPLACER_HPP__

class Parser;

//! Abstract base class for replacement generators
class Replacer {
public:

	/*!
	 * \brief Constructor. Assumes the base class's constructor will fill the key list later
	 * \see DirectReplacer as an example
	 */
	Replacer() : keySet(std::greater<std::string>()) { }

	//! Constructor. Takes a list of keys to initialize keySet with
	Replacer(std::initializer_list<std::string> keys)
		: keySet(keys, std::greater<std::string>())
	{ }

	virtual ~Replacer() { }

	/*!
	 * \brief Performs a replacement, or does nothing
	 * \param matchedKey the key from Replacer::getKeys that was matched
	 * \param p Parser for the current file (or replacement, in the case of recursion)
	 */
	virtual void replace(const std::string& matchedKey, Parser& p) = 0;

	/*!
	 * \brief Gets a list of tokens that should start a replacement of this type
	 *
	 * Constructors should initialize keySet to whatever they want
	 */
	const std::set<std::string, std::greater<std::string>>& getKeys() const { return keySet; };

	//! Returns true if the generated replacement can contain replacements itself
	virtual bool shouldRecurse() const = 0;

protected:
	std::set<std::string, std::greater<std::string>> keySet;

private:
};

#endif
