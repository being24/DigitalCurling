#include "log.hpp"
#include <cassert>
#include <iostream>
#include <exception>

namespace digital_curling::server {

Log::Buffer::Buffer(std::string_view prefix)
{
    buffer_ << prefix;
}

Log::Buffer::~Buffer()
{
    PrintLine(buffer_.str());
}

Log::Log(std::ofstream && file)
    : file_(std::move(file))
{
    assert(instance_ == nullptr);
    instance_ = this;
}

Log::~Log()
{
    instance_ = nullptr;
}

Log::Buffer Log::Trace()
{
    return Buffer("[trace] ");
}

Log::Buffer Log::Debug()
{
    return Buffer("[debug] ");
}

Log::Buffer Log::Info()
{
    return Buffer("[info] ");
}

Log::Buffer Log::Warn()
{
    return Buffer("[warn] ");
}

Log::Buffer Log::Error()
{
    return Buffer("[error] ");
}

void Log::PrintLine(std::string && message)
{
    assert(instance_);
    std::lock_guard guard(instance_->mutex_);
    instance_->file_ << message << '\n';
    instance_->file_.flush();
    std::cout << std::move(message) << std::endl;
}

}
