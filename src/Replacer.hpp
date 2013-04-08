#ifndef __REPLACER_HPP__
#define __REPLACER_HPP__

class ParseInfo;

//! Abstract base class for replacement generators
class Replacer {
public:
	/*!
	 * \brief Constructor. Assumes the base class's constructor will fill the key list later
	 * \see DirectReplacer as an example
	 */
	Replacer() : keyList() { }

	//! Constructor. Takes a list of keys to initialize keyList with
	Replacer(std::initializer_list<std::string> keys) : keyList(keys)
	{
		// Make the longest key first so that smaller keys with the same beginnings as larger ones
		// aren't too greedy (e.g. <-> would be parsed as <- and >)
		std::sort(keyList.begin(), keyList.end(), [](const std::string& a, const std::string& b)
		                                             { return b.length() < a.length(); });
	}

	Replacer(std::vector<std::string>&& keys) : keyList(std::forward<decltype(keys)>(keys))
	{
		// Make the longest key first so that smaller keys with the same beginnings as larger ones
		// aren't too greedy (e.g. <-> would be parsed as <- and >)
		std::sort(keyList.begin(), keyList.end(), [](const std::string& a, const std::string& b)
		                                             { return b.length() < a.length(); });
	}

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

#endif
