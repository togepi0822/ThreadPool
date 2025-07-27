#ifndef INCLUDE_TP_LOG_H
#define INCLUDE_TP_LOG_H

#include <tp/config.h>

#include <iostream>

NAMESPACE_TP_BEGIN

class LogStream {
public:
    LogStream(const char* func,
              const size_t line, std::ostream& os = std::cout)
        : os_(os) {
        os_ << "[" << func << ":" << line << "] ";
    }

    ~LogStream() {
        os_ << std::endl;
    }

    template<class T>
    LogStream& operator<<(const T& msg) {
        os_ << msg;
        return *this;
    }

    LogStream& operator<<(std::ostream& (*op)(std::ostream&)) {
        op(os_);
        return *this;
    }

    void operator()(const char* msg) const { os_ << msg; }

private:
    std::ostream& os_;
};

NAMESPACE_TP_END

#endif //INCLUDE_TP_LOG_H
