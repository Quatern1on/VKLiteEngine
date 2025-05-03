#include "pch.h"

int main(int argc, char* argv[]) {
    FLAGS_logtostdout = true;
    google::InitGoogleLogging(argv[0]);

    std::vector<std::string> args(&argv[1], &argv[argc]);

    try {
        Engine::getInstance().init(args);
        Engine::getInstance().runMainLoop();
    } catch (const std::exception& e) {
        LOG(FATAL) << "Fatal exception occurred: " << e.what() << '\n';
    } catch (...) {
        LOG(FATAL) << "Fatal error occurred!";
    }

    return 0;
}
