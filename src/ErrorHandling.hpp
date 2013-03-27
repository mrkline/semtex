#ifndef __ERROR_HANDLING_HPP__
#define __ERROR_HANDLING_HPP__

#include "Exceptions.hpp"
#include "FileParser.hpp"

/*!
 * \brief A method for throwing standardized exceptions for input errors
 * \param pi The ParseInfo for the file, used to get the file name and line
 * \param msg The error-specific message to attach to the exception
 */
inline void errorOnLine(const ParseInfo& pi, const std::string& msg)
{
		std::stringstream err;
		err << pi.filename << ":" << pi.currLine << ": " << msg;
		throw Exceptions::InvalidInputException(err.str(), __FUNCTION__);
}

#endif
