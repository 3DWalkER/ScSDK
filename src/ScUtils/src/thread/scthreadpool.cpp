#include "scutils/thread/scthreadpool_p.h"

#include "scutils/thread/scrunnable.h"

#include <iterator>

ScThreadPool::ScThreadPool()
	: d_ptr(new ScThreadPoolPrivate(this))
{
}

ScThreadPool::~ScThreadPool()
{
	delete d_ptr;
}

void ScThreadPool::start(ScRunnable* task, int priority)
{
	if (!task)
		return;

	SC_D(ScThreadPool);
	std::unique_lock<std::mutex> locker(d->mutex);
	if (!d->tryStart(task))
		d->enqueueTask(task, priority);
}

void ScThreadPool::start(std::function<void()> funcToRun, int priority)
{
	if (!funcToRun)
		return;
	return start(ScRunnable::create(std::move(funcToRun)), priority);
}

bool ScThreadPool::tryStart(ScRunnable* task)
{
	if (!task)
		return false;

	SC_D(ScThreadPool);
	std::unique_lock<std::mutex> locker(d->mutex);
	if (d->tryStart(task))
		return true;
	return false;
}

bool ScThreadPool::tryStart(std::function<void()> funcToRun)
{
	if (!funcToRun)
		return false;

	SC_D(ScThreadPool);
	std::unique_lock<std::mutex> locker(d->mutex);
	if (!d->allThreads.empty() && d->areAllThreadsActive())
		return false;

	ScRunnable* pRunnable = ScRunnable::create(std::move(funcToRun));
	if (d->tryStart(pRunnable))
		return true;

	delete pRunnable;
	return false;
}

int ScThreadPool::expiryTimeout() const
{
	SC_D(const ScThreadPool);
	return d->expiryTimeout;
}

void ScThreadPool::setExpiryTimeout(int msecs)
{
	SC_D(ScThreadPool);
	if (msecs == d->expiryTimeout)
		return;

	d->expiryTimeout = msecs;
}

int ScThreadPool::maxThreadCount() const
{
	SC_D(const ScThreadPool);
	return d->maxThreadCount;
}

void ScThreadPool::setMaxThreadCount(int maxCount)
{
	SC_D(ScThreadPool);
	std::unique_lock<std::mutex> lock(d->mutex);
	if (maxCount == d->maxThreadCount)
		return;

	d->maxThreadCount = maxCount;
	d->tryToStartMoreThreads();
}

int ScThreadPool::activeThreadCount() const
{
	SC_D(const ScThreadPool);
	std::unique_lock<std::mutex> lock(d->mutex);
	return d->activeThreadCount();
}

scuint32 ScThreadPool::stackSize() const
{
	SC_D(const ScThreadPool);
	return d->stackSize;
}

void ScThreadPool::setStackSize(scuint32 stackSize)
{
	SC_D(ScThreadPool);
	d->stackSize = stackSize;
}

void ScThreadPool::reserveThread()
{
	SC_D(ScThreadPool);
	std::unique_lock<std::mutex> lock(d->mutex);
	++d->reservedThreadNum;
}

void ScThreadPool::releaseThread()
{
	SC_D(ScThreadPool);
	std::unique_lock<std::mutex> lock(d->mutex);
	--d->reservedThreadNum;
	d->tryToStartMoreThreads();
}

bool ScThreadPool::waitForDone(int msecs)
{
	SC_D(ScThreadPool);
	return d->waitForDone(msecs);
}

void ScThreadPool::clear()
{
	SC_D(ScThreadPool);
	ScRunnable* task = nullptr;
	std::unique_lock<std::mutex> lock(d->mutex);
	for (ScQueuePage* page : scAsConst(d->queue))
	{
		while (!page->isFinished())
		{
			task = page->pop();
			if (task && task->isAutoDelete())
				delete task;
		}
	}

	for (ScQueuePage*& queue : d->queue)
	{
		if (queue)
			delete queue;
	}
	d->queue.clear();
}

ScQueuePage::ScQueuePage(ScRunnable* runnable, int pri)
	: m_priority(pri)
{
	push(runnable);
}

void ScQueuePage::skipToNextOrEnd()
{
	while (!isFinished() && !m_entries[m_firstIndex])
		m_firstIndex += 1;
}

bool ScQueuePage::tryTake(ScRunnable* runnable)
{
	SC_ASSERT(!isFinished());
	for (int i = m_firstIndex; i <= m_lastIndex; i++)
	{
		if (m_entries[i] == runnable)
		{
			m_entries[i] = nullptr;
			if (i == m_firstIndex)
				skipToNextOrEnd();
			return true;
		}
	}
	return false;
}

ScThreadPoolPrivate::ScThreadPoolPrivate(ScThreadPool* q)
	: q_ptr(q)
	, maxThreadCount(std::thread::hardware_concurrency())
{
}

ScThreadPoolPrivate::~ScThreadPoolPrivate()
{
	waitForDone();
}

void ScThreadPoolPrivate::reset()
{
	std::unordered_set<ScThreadPoolThread*> allThreadsCopy;
	allThreadsCopy.swap(allThreads);
	while (!expiredThreads.empty())
		expiredThreads.pop();
	waitingThreads.clear();

	mutex.unlock();

	for (ScThreadPoolThread* th : allThreadsCopy)
	{
		if (!th->isFinished)
		{
			th->runnableReady.notify_all();
			th->wait();
		}
	}
}

bool ScThreadPoolPrivate::tryStart(ScRunnable* task)
{
	SC_ASSERT(task);
	if (allThreads.empty())
	{
		startThread(task);
		return true;
	}

	if (areAllThreadsActive())
		return false;

	if (!waitingThreads.empty())
	{
		enqueueTask(task);
		auto pThread = waitingThreads.front();
		pThread->runnableReady.notify_one();
		waitingThreads.pop_front();
		return true;
	}

	if (!expiredThreads.empty())
	{
		auto* thread = expiredThreads.front();
		SC_ASSERT(thread);
		expiredThreads.pop();

		++activeThreadNum;

		thread->runnable = task;
		thread->wait();
		thread->start();
		return true;
	}

	startThread(task);
	return true;
}

void ScThreadPoolPrivate::tryToStartMoreThreads()
{
	ScQueuePage* page = nullptr;
	while (!queue.empty())
	{
		page = queue.front();
		if (!tryStart(page->first()))
			break;

		page->pop();
		if (page->isFinished())
		{
			queue.erase(queue.begin());
			delete page;
		}
	}
}

void ScThreadPoolPrivate::startThread(ScRunnable* task)
{
	SC_ASSERT(task);
	auto thread = std::make_unique<ScThreadPoolThread>(this);
	SC_ASSERT(allThreads.end() == allThreads.find(thread.get()));
	allThreads.insert(thread.get());
	++activeThreadNum;

	thread->runnable = task;
	thread.release()->start();
}

void ScThreadPoolPrivate::enqueueTask(ScRunnable* task, int priority)
{
	SC_ASSERT(task);
	for (ScQueuePage* page : scAsConst(queue))
	{
		if (page->priority() == priority && !page->isFull())
		{
			page->push(task);
			return;
		}
	}

	auto it = std::upper_bound(queue.cbegin(), queue.cend(), priority, [](int pri, const ScQueuePage* page) -> bool {
		return pri > page->priority();
		});
	queue.insert(it, new ScQueuePage(task, priority));
}

bool ScThreadPoolPrivate::waitForDone(int msecs)
{
	auto begin = std::chrono::steady_clock::now();
	std::unique_lock<std::mutex> locker(mutex);
	auto wait = [this, msecs, &locker, begin]() -> bool
		{
			auto end = begin;
			while (!queue.empty() || 0 != activeThreadNum)
			{
				end = std::chrono::steady_clock::now();
				if (std::chrono::duration<double>(end - begin).count() * 1e3 >= msecs)
					break;

				noActiveThreads.wait_for(locker, std::chrono::milliseconds(msecs));
			}
			return queue.empty() && 0 == activeThreadNum;
		};

	auto end = begin;
	do {
		if (!wait())
			return false;
		reset();

		end = std::chrono::steady_clock::now();
		if (std::chrono::duration<double>(end - begin).count() * 1e3 >= msecs)
			break;
	} while (!queue.empty() || 0 != activeThreadNum);

	return queue.empty() && 0 == activeThreadNum;
}

ScThreadPoolThread::ScThreadPoolThread(ScThreadPoolPrivate* manager)
	: manager(manager)
{

}

ScThreadPoolThread::~ScThreadPoolThread()
{
	if (m_thread.joinable())
		m_thread.join();
}

void ScThreadPoolThread::start()
{
	if (m_thread.joinable())
		m_thread.join();

	isFinished = false;
	m_thread = std::thread(&ScThreadPoolThread::run, this);
}

bool ScThreadPoolThread::wait(scint32 msecs)
{
	std::unique_lock<std::mutex> locker(mutex);
	if (isFinished)
		return true;

	if (!m_thread.joinable())
		return false;

	if (msecs < 0)
	{
		finishedCv.wait(locker, [this]() { return isFinished; });
		return true;
	}

	bool isOk = finishedCv.wait_for(locker, std::chrono::milliseconds(msecs), [this]() { return isFinished; });
	return isOk;
}

void ScThreadPoolThread::run()
{
	std::unique_lock<std::mutex> locker(manager->mutex);
	ScRunnable* r = nullptr;
	ScQueuePage* page = nullptr;
	while (true)
	{
		r = runnable;
		runnable = nullptr;

		do {
			if (nullptr != r)
			{
				const bool isAutoDelete = r->isAutoDelete();
				locker.unlock();
				try {
					r->run();
				}
				catch (...) {
					registerThreadInactive();
					throw;
				}

				if (isAutoDelete)
					delete r;
				locker.lock();
			}

			if (manager->isTooManyThreadsActive())
				break;

			if (manager->queue.empty())
				break;

			page = manager->queue.front();
			r = page->pop();
			if (page->isFinished())
			{
				manager->queue.erase(manager->queue.begin());
				delete page;
			}
		} while (true);

		if (manager->allThreads.end() == manager->allThreads.find(this))
		{
			registerThreadInactive();
			break;
		}

		if (manager->isTooManyThreadsActive())
		{
			manager->expiredThreads.push(this);
			registerThreadInactive();
			break;
		}

		manager->waitingThreads.push_back(this);
		registerThreadInactive();
		runnableReady.wait_for(locker, std::chrono::milliseconds(manager->expiryTimeout));

		if (manager->allThreads.end() == manager->allThreads.find(this))
		{
			SC_ASSERT(manager->queue.empty());
			break;
		}

		auto it = find(manager->waitingThreads.cbegin(), manager->waitingThreads.cend(), this);
		if (manager->waitingThreads.cend() != it)
		{
			manager->waitingThreads.erase(it);
			manager->expiredThreads.push(this);
			break;
		}

		++manager->activeThreadNum;
	}

	std::lock_guard<std::mutex> flocker(mutex);
	isFinished = true;
	finishedCv.notify_all();
}
