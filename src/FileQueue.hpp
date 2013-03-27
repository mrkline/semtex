#ifndef __FILE_QUEUE_HPP__
#define __FILE_QUEUE_HPP__

//! A thread-safe queue for tracking SemTeX files that need to be processed
class FileQueue {
public:
	//! Callback to issue if there is more than one file in the queue.
	//! This is likely a good indication to use multi-threading.
	typedef void (*QueueUsedCallback)(const FileQueue& q);

	//! Constructor
	//! \param call A callback to issue if there is more than one file in the queue.
	FileQueue(QueueUsedCallback call);

	//! Enqueue a file to be processed
	void enqueue(std::string&& filename);

	/*!
	 * \brief Used to temporarily disable dequeuing so that we can check if we have additional files to process.
	 *
	 * We check if we are finished processing files by checking if all threads are not busy, then if the queue is empty.
	 * Without disabling dequeuing, there is some chance that a thread could dequeue the last file(s) in between those
	 * two steps, convincing the system that it was done processing files when the last ones still remained.
	 */
	void setDequeueEnabled(bool de) { canDequeue = de; }

	/*!
	 * \brief Attempts to dequeue a file from the queue
	 * \param timeout The amount of time to wait for a file to arrive in the queue
	 * \returns The file from the front of the queue, or an empty string if the queue was still empty
	 *          when the timeout was reached.
	 */
	std::string dequeue(const std::chrono::milliseconds& timeout);

	//! Returns true if the queue is empty
	bool empty();

private:
	QueueUsedCallback cb;
	std::queue<std::string> q;
	std::mutex qMutex; //!< Makes the queue thread-safe
	std::condition_variable populatedNotifier; //!< Signalled when the queue is repopulated
	std::atomic_bool canDequeue;
};

#endif
