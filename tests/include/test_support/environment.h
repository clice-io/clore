#ifndef CLORE_TEST_SUPPORT_ENVIRONMENT_H
#define CLORE_TEST_SUPPORT_ENVIRONMENT_H

#include <cstdlib>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

[[nodiscard]] inline auto environment_mutex() -> std::mutex& {
    static std::mutex mutex;
    return mutex;
}

struct ScopedEnvironmentLock {
    std::unique_lock<std::mutex> lock;

    ScopedEnvironmentLock() : lock(environment_mutex()) {}
};

struct ScopedEnvVar {
    std::string name;
    std::optional<std::string> original;

    explicit ScopedEnvVar(std::string env_name) : name(std::move(env_name)) {
        if(auto* value = std::getenv(name.c_str()); value != nullptr) {
            original = value;
        }
    }

    ~ScopedEnvVar() { restore(); }

    void set(std::string_view value) const {
#ifdef _WIN32
        _putenv_s(name.c_str(), std::string(value).c_str());
#else
        setenv(name.c_str(), std::string(value).c_str(), 1);
#endif
    }

    void unset() const {
#ifdef _WIN32
        _putenv_s(name.c_str(), "");
#else
        unsetenv(name.c_str());
#endif
    }

    void restore() const {
        if(original.has_value()) {
            set(*original);
        } else {
            unset();
        }
    }
};

#endif