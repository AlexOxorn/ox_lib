#include <ox/io.h>
#include <climits>
#include <unistd.h>
#include <cstdio>

namespace ox {
    const std::filesystem::path& executable_path() {
        static char result[PATH_MAX] = {};
        if (result[0] == 0) {
            ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
            if (count == -1) {
                throw std::exception();
            }
        }
        static std::filesystem::path exec_path{result};
        return exec_path;
    }

    const std::filesystem::path& executable_folder() {
        const std::filesystem::path& exec_path = executable_path();
        static std::filesystem::path exec_folder{exec_path.parent_path()};
        return exec_folder;
    }
} // namespace ox
