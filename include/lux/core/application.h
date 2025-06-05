#pragma once

namespace Lux
{
    class Application
    {
        public:
            Application();
            virtual ~Application() = default;
            void Run();

            // user ovveridable methods
            virtual void OnInit() {}
            virtual void OnUpdate(const float& deltaTime) {}
            virtual void OnRender() {}
            virtual void OnExit() {}

            static Application& GetInstance();
        private:
            bool m_Running = true;
            static Application* s_instance;
    };
}