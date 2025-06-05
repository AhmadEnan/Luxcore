#include <lux/core/application.h>
#include <lux/core/logger.h>

#include <iostream>

namespace Lux
{
    Application* Application::s_instance = nullptr;

    Application::Application()
    {
        if(s_instance != nullptr)
        {
            std::cerr << "Application instance already exists!" << std::endl;
        }

        s_instance = this;
    }

    void Application::Run()
    {
        Logger::Get().Initialize();
        Logger::Get().SetLevel(logLevel::INFO);
        LUX_INFO("Initializaing the application");
        OnInit();

        while(m_Running)
        {
            float deltaTime = 0.016f; // temporary
            OnUpdate(deltaTime);
            OnRender();
            m_Running = false; // temporary
        }

        OnExit();
        Logger::Get().Shutdown();
    }
}