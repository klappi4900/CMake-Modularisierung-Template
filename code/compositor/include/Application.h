#pragma once
#include <memory>

class IFileWriter;
class ILogger;

class Application {
public:
    Application();
    ~Application() = default;
    void run();
    void stop();

private:
    std::unique_ptr<IFileWriter> fileWriter_;
    std::unique_ptr<ILogger>     logger_;
};
