#pragma once
#include "application.h"
#include <iostream>

extern Lux::Application* CreateApplication();

int main(int argc, char** argv)
{
    Lux::Application* app = CreateApplication();
    if (app == nullptr)
    {
        std::cerr << "Failed to create application instance!" << std::endl;
        return -1;
    }
    
    app->Run();
    delete app;
    
    std::cout << "Press any key to exit...\n";
    std::cin.get();
    return 0;
}

#define LUX_APPLICATION(app) Lux::Application* CreateApplication() { return new app(); }