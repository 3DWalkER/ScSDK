#include "scutils/thread/scrunnable.h"

class ScFunctionRunnable : public ScRunnable
{
    std::function<void()> m_funcToRun;
public:
    ScFunctionRunnable(std::function<void()> funcToRun) 
        : m_funcToRun(std::move(funcToRun)) { }

    void run() override
    {
        if (m_funcToRun)
            m_funcToRun();
    }
};

ScRunnable* ScRunnable::create(std::function<void()> funcToRun)
{
    return new ScFunctionRunnable(std::move(funcToRun));
}
