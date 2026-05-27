#pragma once
#include <memory>

class IFileWriter;
class ILogger;

class Application {
public:
    Application();
    ~Application();
    void run();

private:
    std::unique_ptr<IFileWriter> fileWriter_;
    std::unique_ptr<ILogger>     logger_;
};
