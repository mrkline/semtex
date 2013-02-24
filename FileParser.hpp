#pragma once

#include <string>

class Context;

/*!
 * \brief Processes a SemTeX file, generating a corresponding LaTeX file and adding included SemTeX files
 *        to the queue
 * \param filename The path of the SemTeX file to process
 * \param context The global context (verbosity level, queues, etc.)
 */
void processFile(const std::string& filename, Context& ctxt);
