#include "precomp.hpp"

#include "FileQueue.hpp"

FileQueue::FileQueue(QueueUsedCallback call)
	: cb(call), canDequeue(true)
{
}

void FileQueue::enqueue(std::string&& filename)
{
	std::lock_guard<std::mutex> lock(qMutex);
	q.push(std::forward<std::string>(filename));

	if (cb != nullptr)
		cb(*this);

	// Notify anyone waiting for additional files that more have arrived
	populatedNotifier.notify_one();
}

std::string FileQueue::dequeue(const std::chrono::milliseconds& timeout)
{
	// If we are not allowed to dequeue right now, just wait the expected time and return
	if (!canDequeue) {
		std::this_thread::sleep_for(timeout);
		return std::string();
	}
	std::unique_lock<std::mutex> lock(qMutex);
	if (populatedNotifier.wait_for(lock, timeout, [this] { return !q.empty(); })) {
		std::string ret = std::move(q.front());
		q.pop();
		return ret;
	}
	else {
		return std::string();
	}
}

bool FileQueue::empty()
{
	std::unique_lock<std::mutex> lock(qMutex);
	return q.empty();
}
