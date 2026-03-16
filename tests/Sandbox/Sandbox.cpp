#include <NGINDX/NGINDX.h>

class Sandbox final : public NGINDX::GameCore
{
private:
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

    void Draw() override
    {
    }

public:
    Sandbox()
    {
    }

    ~Sandbox()
    {
    }
};

LPCWSTR Title() { return L"Sandbox"; }

NGINDX::GameCore* CreateApplication()
{
    return new Sandbox();
}
