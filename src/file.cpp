#include <ox/file.h>
#include <limits.h>
#include <unistd.h>
#include <cstdio>

namespace ox {
    const std::filesystem::path& executable_path() {
        static std::filesystem::path exec_path;
        if (exec_path.empty()) {
            char result[ PATH_MAX ];
            ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
            exec_path.assign(result);
        }     
        return exec_path;
    }

    const std::filesystem::path& executable_folder() {
        static std::filesystem::path exec_folder;
        if (exec_folder.empty()) {
            const std::filesystem::path& exec_path = executable_path();
            exec_folder.assign(exec_path.parent_path());
        }     
        return exec_folder;
    }
}
