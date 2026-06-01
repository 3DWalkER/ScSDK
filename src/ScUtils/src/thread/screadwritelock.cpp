#include "scutils/thread/screadwritelock.h"

#include "scutils/buffer/scfreelist.h"
#include "scutils/utils/scglobalstatic.h"

#include <thread>
#include <mutex>
#include <unordered_map>

class ScReadWriteLockPrivate
{
public:
	explicit ScReadWriteLockPrivate(bool isRecursive = false) : isRecursive(isRecursive) {}

	bool lockForWrite(int timeout, std::unique_lock<std::mutex>& locker);
	bool lockForRead(int timeout, std::unique_lock<std::mutex>& locker);
	void unlock();

	bool recursiveLockForWrite(int timeout);
	bool recursiveLockForRead(int timeout);
	void recursiveUnlock();

	void release();
	static ScReadWriteLockPrivate* allocate();

	int id = 0;
	const bool isRecursive;
	std::mutex mutex;
	int readerCount{ 0 };
	int writerCount{ 0 };
	int waitingReaders{ 0 };
	int waitingWriters{ 0 };
	std::condition_variable readerCond;
	std::condition_variable writerCond;

	std::thread::id currentWriter{ };
	std::unordered_map<std::thread::id, int> currentReaders;
};

namespace
{
	struct ScFreeListConstants : ScFreeListDefaultConstants
	{
		enum { BlockCount = 4, MaxIndex = 0xffff };
		static const int Sizes[BlockCount];
	};

	const int ScFreeListConstants::Sizes[ScFreeListConstants::BlockCount] = {
		16,
		128,
		1024,
		ScFreeListConstants::MaxIndex - (16 + 128 + 1024)
	};

	typedef ScFreeList<ScReadWriteLockPrivate, ScFreeListConstants> FreeList;
	SC_GLOBAL_STATIC(FreeList, freelist);
}

namespace
{
	enum
	{
		StateMask = 0x3,
		StateLockedForRead = 0x1,
		StateLockedForWrite = 0x2,
	};

	const auto dummyLockedForRead = reinterpret_cast<ScReadWriteLockPrivate*>(uintptr_t(StateLockedForRead));
	const auto dummyLockedForWrite = reinterpret_cast<ScReadWriteLockPrivate*>(uintptr_t(StateLockedForWrite));
	inline bool isUncontendedLocked(const ScReadWriteLockPrivate* d) {
		return scuintptr(d) & StateMask;
	}
}

ScReadWriteLock::ScReadWriteLock(RecursionMode recursionMode)
	: d_ptr(Recursive == recursionMode ? new ScReadWriteLockPrivate(true) : nullptr)
{
}

ScReadWriteLock::~ScReadWriteLock()
{
	auto d = d_ptr.loadRelaxed();
	if (isUncontendedLocked(d))
	{
		fprintf(stderr, "ScReadWriteLock: destroying locked ScReadWriteLock!\n");
		return;
	}
	delete d;
}

void ScReadWriteLock::lockForRead()
{
	if (d_ptr.testAndSetAcquire(nullptr, dummyLockedForRead))
		return;
	tryLockForRead(-1);
}

bool ScReadWriteLock::tryLockForRead(int timeout)
{
	ScReadWriteLockPrivate* d;
	if (d_ptr.testAndSetAcquire(nullptr, dummyLockedForRead, d))
		return true;

	while (true)
	{
		if (d == 0)
		{
			if (!d_ptr.testAndSetAcquire(nullptr, dummyLockedForRead, d))
				continue;
			return true;
		}

		if ((scuintptr(d) & StateMask) == StateLockedForRead)
		{
			const auto val = reinterpret_cast<ScReadWriteLockPrivate*>(scuintptr(d) + (1U << 4));
			SC_ASSERT_X(scuintptr(val) > (1U << 4), "ScReadWriteLock::tryLockForRead()", "Overflow in lock counter");
			if (!d_ptr.testAndSetAcquire(d, val, d))
				continue;
			return true;
		}

		if (d == dummyLockedForWrite)
		{
			if (!timeout)
				return false;

			auto val = ScReadWriteLockPrivate::allocate();
			val->writerCount = 1;
			if (!d_ptr.testAndSetOrdered(d, val, d))
			{
				val->writerCount = 0;
				val->release();
				continue;
			}
			d = val;
		}
		SC_ASSERT(!isUncontendedLocked(d));

		if (d->isRecursive)
			return d->recursiveLockForRead(timeout);

		std::unique_lock<std::mutex> locker(d->mutex);
		if (d != d_ptr.loadRelaxed())
		{
			d = d_ptr.loadAcquire();
			continue;
		}
		return d->lockForRead(timeout, locker);
	}
}

void ScReadWriteLock::lockForWrite()
{
	tryLockForWrite(-1);
}

bool ScReadWriteLock::tryLockForWrite(int timeout)
{
	ScReadWriteLockPrivate* d;
	if (d_ptr.testAndSetAcquire(nullptr, dummyLockedForWrite, d))
		return true;

	while (true)
	{
		if (d == 0)
		{
			if (!d_ptr.testAndSetAcquire(d, dummyLockedForWrite, d))
				continue;
			return true;
		}

		if (isUncontendedLocked(d))
		{
			if (!timeout)
				return false;

			auto val = ScReadWriteLockPrivate::allocate();
			if (d == dummyLockedForWrite)
				val->writerCount = 1;
			else
				val->readerCount = (scuintptr(d) >> 4) + 1;

			if (!d_ptr.testAndSetOrdered(d, val, d))
			{
				val->writerCount = val->readerCount = 0;
				val->release();
				continue;
			}
			d = val;
		}

		SC_ASSERT(!isUncontendedLocked(d));
		if (d->isRecursive)
			return d->recursiveLockForWrite(timeout);

		std::unique_lock<std::mutex> locker(d->mutex);
		if (d != d_ptr.loadRelaxed())
		{
			d = d_ptr.loadAcquire();
			continue;
		}

		return d->lockForWrite(timeout, locker);
	}
}

void ScReadWriteLock::unlock()
{
	ScReadWriteLockPrivate* d = d_ptr.loadAcquire();
	while (true)
	{
		SC_ASSERT_X(d, "ScReadWriteLock::unlock()", "Cannot unlock an unlocked lock");

		if (scuintptr(d) <= 2)
		{
			if (!d_ptr.testAndSetOrdered(d, nullptr, d))
				continue;
			return;
		}

		if ((scuintptr(d) & StateMask) == StateLockedForRead)
		{
			SC_ASSERT(scuintptr(d) > (1U << 4));
			auto val = reinterpret_cast<ScReadWriteLockPrivate*>(scuintptr(d) - (1U << 4));
			if (!d_ptr.testAndSetOrdered(d, val, d))
				continue;
			return;
		}

		SC_ASSERT(!isUncontendedLocked(d));

		if (d->isRecursive)
		{
			d->recursiveUnlock();
			return;
		}

		std::unique_lock<std::mutex> locker(d->mutex);
		if (d->writerCount)
		{
			SC_ASSERT(d->writerCount == 1);
			SC_ASSERT(d->readerCount == 0);
			d->writerCount = 0;
		}
		else
		{
			SC_ASSERT(d->readerCount > 0);
			d->readerCount--;
			if (d->readerCount > 0)
				return;
		}

		if (d->waitingReaders || d->waitingWriters)
		{
			d->unlock();
		}
		else
		{
			SC_ASSERT(d_ptr.loadRelaxed() == d);
			d_ptr.storeRelease(nullptr);
			d->release();
		}
		return;
	}
}

bool ScReadWriteLockPrivate::lockForWrite(int timeout, std::unique_lock<std::mutex>& locker)
{
	SC_ASSERT(!mutex.try_lock());

	int elapsed = 0;
	std::chrono::steady_clock::time_point begin, end;
	if (timeout > 0)
		begin = std::chrono::steady_clock::now();

	while (readerCount || writerCount)
	{
		if (timeout == 0)
			return false;

		if (timeout > 0)
		{
			end = std::chrono::steady_clock::now();
			elapsed = static_cast<int>(std::chrono::duration<double>(end - begin).count() * 1e3);
			if (elapsed > timeout)
			{
				if (waitingReaders && !waitingWriters && !writerCount)
					readerCond.notify_all();
				return false;
			}
			waitingWriters++;
			writerCond.wait_for(locker, std::chrono::milliseconds(timeout - elapsed));
		}
		else
		{
			waitingWriters++;
			writerCond.wait(locker);
		}
		waitingWriters--;
	}

	SC_ASSERT(writerCount == 0);
	SC_ASSERT(readerCount == 0);
	writerCount = 1;
	return true;
}

bool ScReadWriteLockPrivate::lockForRead(int timeout, std::unique_lock<std::mutex>& locker)
{
	SC_ASSERT(!mutex.try_lock());

	int elapsed = 0;
	std::chrono::steady_clock::time_point begin, end;
	if (timeout > 0)
		begin = std::chrono::steady_clock::now();

	while (waitingWriters || writerCount)
	{
		if (timeout == 0)
			return false;

		if (timeout > 0)
		{
			end = std::chrono::steady_clock::now();
			elapsed = static_cast<int>(std::chrono::duration<double>(end - begin).count() * 1e3);
			if (elapsed > timeout)
				return false;

			waitingReaders++;
			readerCond.wait_for(locker, std::chrono::milliseconds(timeout - elapsed));
		}
		else
		{
			waitingReaders++;
			readerCond.wait(locker);
		}
		waitingReaders--;
	}
	readerCount++;
	SC_ASSERT(writerCount == 0);
	return true;
}

void ScReadWriteLockPrivate::unlock()
{
	SC_ASSERT(!mutex.try_lock());
	if (waitingWriters)
		writerCond.notify_one();
	else if (waitingReaders)
		readerCond.notify_all();
}

bool ScReadWriteLockPrivate::recursiveLockForWrite(int timeout)
{
	SC_ASSERT(isRecursive);
	std::unique_lock<std::mutex> locker(mutex);

	std::thread::id self = std::this_thread::get_id();
	if (currentWriter == self)
	{
		writerCount++;
		return true;
	}

	if (!lockForWrite(timeout, locker))
		return false;

	currentWriter = self;
	return true;
}

bool ScReadWriteLockPrivate::recursiveLockForRead(int timeout)
{
	SC_ASSERT(isRecursive);
	std::unique_lock<std::mutex> locker(mutex);
	std::thread::id self = std::this_thread::get_id();

	auto it = currentReaders.find(self);
	if (it != currentReaders.end())
	{
		++it->second;
		return true;
	}

	if (!lockForRead(timeout, locker))
		return false;

	currentReaders[self] = 1;
	return true;
}

void ScReadWriteLockPrivate::recursiveUnlock()
{
	SC_ASSERT(isRecursive);
	std::unique_lock<std::mutex> locker(mutex);

	std::thread::id self = std::this_thread::get_id();
	if (self == currentWriter)
	{
		if (--writerCount > 0)
			return;
		currentWriter = std::thread::id();;
	}
	else
	{
		auto it = currentReaders.find(self);
		if (it == currentReaders.end())
		{
			fprintf(stderr, "ScReadWriteLock::unlock: unlocking from a thread that did not lock!\n");
			return;
		}
		else
		{
			if (--it->second <= 0)
			{
				currentReaders.erase(it);
				readerCount--;
			}

			if (readerCount)
				return;
		}
	}

	unlock();
}

void ScReadWriteLockPrivate::release()
{
	SC_ASSERT(!isRecursive);
	SC_ASSERT(!waitingReaders && !waitingWriters && !readerCount && !writerCount);
	freelist->release(id);
}

ScReadWriteLockPrivate* ScReadWriteLockPrivate::allocate()
{
	int i = freelist->next();
	ScReadWriteLockPrivate* d = &(*freelist)[i];
	d->id = i;
	SC_ASSERT(!d->isRecursive);
	SC_ASSERT(!d->waitingReaders && !d->waitingWriters && !d->readerCount && !d->writerCount);
	return d;
}
