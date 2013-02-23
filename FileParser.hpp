#pragma once

class FileQueue;

#include <mutex>
#include <string>
#include <unordered_set>

/*!
 * \brief Processes a SemTeX file, generating a corresponding LaTeX file and adding included SemTeX files
 *        to the queue
 * \param filename The path of the SemTeX file to process
 * \param isBaseFile Set to true only for the base file
 * \param verbose true to print information to stdout
 * \param sfq The queue to which newly discovered SemTeX files will be added
 * \param generatedFiles The persistant list of generated LaTeX files
 *                       (to be deleted upon program completion, if the user so desires)
 * \param generatedFilesMutex The mutex to lock while accessing generatedFiles
 */
void processFile(const std::string& filename, bool isBaseFile, bool verbose, FileQueue& sfq,
                       std::unordered_set<std::string>& generatedFiles, std::mutex& generatedFilesMutex);
