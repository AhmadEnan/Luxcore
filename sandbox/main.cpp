#include <lux/lux.h>

class SandboxApp : public Lux::Application
{
    public:
    void OnInit() override
    {
        std::cout << "Sandbox App init\n";
    }
};

LUX_APPLICATION(SandboxApp);