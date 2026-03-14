#include <NGINDX/NGINDX.h>

class Sandbox : public NGINDX::GameCore
{
public:
    Sandbox()
        : GameCore{}
    {
    }

    void Startup() override
    {
    }

    void Cleanup() override
    {
    }

    void Update(float deltaTime) override
    {
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
