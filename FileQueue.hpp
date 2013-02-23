#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

//! A thread-safe queue for tracking SemTeX files that need to be processed
class FileQueue {
public:
	//! Callback to issue if there is more than one file in the queue.
	//! This is likely a good indication to use multi-threading.
	typedef void (*SeveralCallback)(FileQueue& q);

	//! Constructor
	//! \param cb A callback to issue if there is more than one file in the queue.
	FileQueue(SeveralCallback call);

	//! Enqueue a file to be processed
	void enqueue(std::string&& filename);

	/*!
	 * \brief Attempts to dequeue a file from the queue
	 * \param timeout The amount of time to wait for a file to arrive in the queue
	 * \returns The file from the front of the queue, or an empty string if the queue was still empty
	 *          when the timeout was reached.
	 */
	std::string dequeue(const std::chrono::milliseconds& timeout);

private:
	SeveralCallback cb;
	std::queue<std::string> q;
	std::mutex qMutex; //!< Makes the queue thread-safe
	std::condition_variable populatedNotifier; //!< Signalled when the queue is repopulated
};
