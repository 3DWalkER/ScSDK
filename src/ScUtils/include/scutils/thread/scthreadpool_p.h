#ifndef SCTHREADPOOL_P_H
#define SCTHREADPOOL_P_H

#include "scutils/thread/scthreadpool.h"

#include <mutex>
#include <unordered_set>
#include <queue>
#include <condition_variable>

class ScThreadPoolThread;

class ScQueuePage
{
public:
    enum { MaxPageSize = 256 };

    ScQueuePage(ScRunnable* runnable, int pri);

    int priority() const { return m_priority; }

    bool isFull() { return m_lastIndex >= MaxPageSize - 1; }
    bool isFinished() { return m_firstIndex > m_lastIndex; }

    void push(ScRunnable* runnable);
    ScRunnable* pop();
    ScRunnable* first();
    void skipToNextOrEnd();
    bool tryTake(ScRunnable* runnable);

private:
    int m_priority = 0;
    int m_firstIndex = 0;
    int m_lastIndex = -1;
    ScRunnable* m_entries[MaxPageSize];
};

inline void ScQueuePage::push(ScRunnable* runnable)
{
    SC_ASSERT(nullptr != runnable);
    SC_ASSERT(!isFull());
    m_lastIndex += 1;
    m_entries[m_lastIndex] = runnable;
}

inline ScRunnable* ScQueuePage::pop()
{
    SC_ASSERT(!isFinished());
    ScRunnable* runnable = first();
    m_entries[m_firstIndex] = nullptr;
    m_firstIndex += 1;
    skipToNextOrEnd();
    return runnable;
}

inline ScRunnable* ScQueuePage::first()
{
    SC_ASSERT(!isFinished());
    ScRunnable* runnable = m_entries[m_firstIndex];
    SC_ASSERT(runnable);
    return runnable;
}

class ScThreadPoolPrivate
{
    SC_DECLARE_PUBLIC(ScThreadPool)
public:
    explicit ScThreadPoolPrivate(ScThreadPool* q);
    ~ScThreadPoolPrivate();

    void reset();

    bool tryStart(ScRunnable* task);
    void tryToStartMoreThreads();
    void startThread(ScRunnable* task);

    int activeThreadCount() const;
    bool isTooManyThreadsActive() const;
    bool areAllThreadsActive() const;

    void enqueueTask(ScRunnable* task, int priority = 0);

    bool waitForDone(int msecs = -1);

    ScThreadPool* q_ptr;

    int maxThreadCount{ };
    int activeThreadNum { };
    int reservedThreadNum { };

    mutable std::mutex mutex;
    std::unordered_set<ScThreadPoolThread *> allThreads;
    std::deque<ScThreadPoolThread*> waitingThreads;
    std::queue<ScThreadPoolThread*>	expiredThreads;
    std::vector<ScQueuePage*> queue;
    std::condition_variable noActiveThreads;

    int expiryTimeout{ 30000 };
    scuint32 stackSize{ 0 };
};

inline int ScThreadPoolPrivate::activeThreadCount() const
{
    return static_cast<int>(allThreads.size() - waitingThreads.size() - expiredThreads.size() + reservedThreadNum);
}

inline bool ScThreadPoolPrivate::areAllThreadsActive() const
{
    const int actThCount = activeThreadCount();
    return actThCount > maxThreadCount && (actThCount - reservedThreadNum) >= 1;
}

inline bool ScThreadPoolPrivate::isTooManyThreadsActive() const
{
    const int actThCount = activeThreadCount();
    return actThCount > maxThreadCount && (actThCount - reservedThreadNum) > 1;
}

class ScThreadPoolThread
{
    friend class ScThreadPoolPrivate;
public:
    explicit ScThreadPoolThread(ScThreadPoolPrivate* manager);
    ~ScThreadPoolThread();

    void start();
    bool wait(scint32 msecs = -1);

private:
    void run();
    void registerThreadInactive();

    ScRunnable* runnable{ };
    ScThreadPoolPrivate* manager;

    bool isFinished{ false };
    std::thread m_thread;
    std::mutex mutex;
    std::condition_variable runnableReady, finishedCv;
};

inline void ScThreadPoolThread::registerThreadInactive()
{
    if (0 == --manager->activeThreadNum)
        manager->noActiveThreads.notify_all();
}

#endif // SCTHREADPOOL_P_H
