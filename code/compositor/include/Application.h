#pragma once
#include <memory>

#ifdef ENABLE_F_OPCHECK
class IFileWriter;
#endif
#ifdef ENABLE_LOGGER
class ILogger;
#endif

class Application {
public:
    Application();
    ~Application();
    void run();
    void stop();

private:
#ifdef ENABLE_F_OPCHECK
    std::unique_ptr<IFileWriter> fileWriter_;
#endif
#ifdef ENABLE_LOGGER
    std::unique_ptr<ILogger>     logger_;
#endif
};
