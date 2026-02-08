#pragma once

#include <cstdio>
#include <string>
#include <fstream>

#include "macros.h"
#include "time_utils.h"
#include "lf_queue.h"
#include "thread_utils.h"

namespace Common {
    constexpr size_t LOG_QUEUE_SIZE = 8 * 1024 * 1024;

    enum class LogType : int8_t {
        CHAR = 0,
        INTEGER = 1,
        LONG_INTEGER = 2,
        LONG_LONG_INTEGER = 3,
        UNSIGNED_INTEGER = 4,
        UNSIGNED_LONG_INTEGER = 5,
        UNSIGNED_LONG_LONG_INTEGER = 6,
        FLOAT = 7,
        DOUBLE = 8
    };

    struct LogElement {
        LogType type_ = LogType::CHAR;
        union {
            char c;
            int i;
            long li;
            long long lli;
            unsigned u;
            unsigned long ul;
            unsigned long long ull;
            float f;
            double d;
        } u_;
    };

    // Removed final for tests
    class Logger {
    public:
        auto flushQueue() noexcept {
            while(running_){
                for(auto next = queue_.getNextToRead(); queue_.size() && next; next = queue_.getNextToRead()){
                    switch (next->type_) {
                        case LogType::CHAR: file_ << next->u_.c; break;
                        case LogType::INTEGER: file_ << next->u_.i; break;
                        case LogType::LONG_INTEGER: file_ << next->u_.li; break;
                        case LogType::LONG_LONG_INTEGER: file_ << next->u_.lli; break;
                        case LogType::UNSIGNED_INTEGER: file_ << next->u_.u; break;
                        case LogType::UNSIGNED_LONG_INTEGER: file_ << next->u_.ul; break;
                        case LogType::UNSIGNED_LONG_LONG_INTEGER: file_ << next->u_.ull; break;
                        case LogType::FLOAT: file_ << next->u_.f; break;
                        case LogType::DOUBLE: file_ << next->u_.d; break;
                        default:
                            break;
                    }
                    queue_.updateReadIndex();
                }
                // This line makes it work. WHY?
                file_.flush();

                using namespace std::literals::chrono_literals;
                std::this_thread::sleep_for(5ms);
            }
        }

        // explicit - to prevent from implicite coversion for ctor with one param
        explicit Logger(std::string &file_name) : file_name_(file_name), queue_(LOG_QUEUE_SIZE) {
            file_.open(file_name_);
            ASSERT(file_.is_open(), "Could not open log file: " + file_name_);

            logger_thread_ = createAndStartThread(-1, "Logger", [this]() { flushQueue(); } );
            ASSERT(logger_thread_ != nullptr, "Could not start logger thread.");
        }

        ~Logger() {
            std::cout << "Closing Logger for: "<< file_name_ << std::endl;
            while(queue_.size()){
                using namespace std::literals::chrono_literals;
                std::this_thread::sleep_for(1s);
            }
            running_ = false;
            logger_thread_->join();
            
            file_.close();
        }

        Logger() = delete;
        Logger(const Logger&) = delete;
        Logger(const Logger&&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger& operator=(const Logger&&) = delete;

        auto pushValue(const LogElement &log_element) noexcept {
            *(queue_.getNextToWriteTo()) = log_element;
            queue_.updateNextToWriteTo();
        }

        auto pushValue(const char value) noexcept {
            pushValue(LogElement{LogType::CHAR, {.c = value}});
        }

        auto pushValue(const int value) noexcept {
            pushValue(LogElement{LogType::INTEGER, {.i = value}});
        }

        auto pushValue(const long value) noexcept {
            pushValue(LogElement{LogType::LONG_INTEGER, {.li = value}});
        }

        auto pushValue(const long long value) noexcept {
            pushValue(LogElement{LogType::LONG_LONG_INTEGER, {.lli = value}});
        }

        auto pushValue(const unsigned value) noexcept {
            pushValue(LogElement{LogType::UNSIGNED_INTEGER, {.u = value}});
        }

        auto pushValue(const unsigned long value) noexcept {
            pushValue(LogElement{LogType::UNSIGNED_LONG_INTEGER, {.ul = value}});
        }

        auto pushValue(const unsigned long long value) noexcept {
            pushValue(LogElement{LogType::UNSIGNED_LONG_LONG_INTEGER, {.ull = value}});
        }

        auto pushValue(const float value) noexcept {
            pushValue(LogElement{LogType::FLOAT, {.f = value}});
        }

        auto pushValue(const double value) noexcept {
            pushValue(LogElement{LogType::DOUBLE, {.d = value}});
        }

        auto pushValue(const char *value) noexcept {
            while (*value) {
                pushValue(*value);
                ++value;
            }
        }

        auto pushValue(const std::string &value) noexcept {
            pushValue(value.c_str());
        }

        template<typename T, typename... A>
        auto log(const char *s, const T &value, A... args) noexcept {
            while (*s) {
                if (*s == '%') {
                    if (UNLIKELY(*(s + 1) == '%')) { // to allow %% -> % escape character.
                        ++s;
                    } else {
                        pushValue(value); // substitute % with the value specified in the arguments.
                        log(s + 1, args...); // pop an argument and call self recursively.
                        return;
                    }
                }
                pushValue(*s++);
            }
            FATAL("extra arguments provided to log()");
        }

        // note that this is overloading not specialization. gcc does not allow inline specializations.
        auto log(const char *s) noexcept {
            while (*s) {
                if (*s == '%') {
                    if (UNLIKELY(*(s + 1) == '%')) { // to allow %% -> % escape character.
                        ++s;
                    } else {
                        FATAL("missing arguments to log()");
                    }
                }
                pushValue(*s++);
            }
        }

    private:
        const std::string file_name_;
        std::ofstream file_;
        LFQueue<LogElement> queue_;
        std::atomic<bool> running_ = {true};
        std::thread *logger_thread_ = nullptr;
    };
}