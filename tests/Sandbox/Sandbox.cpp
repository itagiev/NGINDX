#include <NGINDX/NGINDX.h>

class Sandbox final : public NGINDX::GameCore
{
public:
    Sandbox()
    {
    }

    ~Sandbox()
    {
    }

    void Startup() override
    {
    }

    void Cleanup() override
    {
    }

    void Update(const NGINDX::StepTimer& timer) override
    {
        timer.GetElapsedSeconds();
    }

    void Render() override
    {
    }
};

LPCWSTR Title() { return L"Sandbox"; }

NGINDX::GameCore* CreateApplication()
{
    return new Sandbox();
}
