
#include "logger.h"


/**
 * Simple dummy ostream.
 * See https://stackoverflow.com/questions/7818371
 */
class NullStreamBuffer : public std::streambuf
{
public:
    std::streamsize xsputn(const char* s, std::streamsize n) override { return n; }
    int overflow(int c) override { return 1; }
};


class NullOutStream : public std::ostream
{
public:
    NullOutStream() : std::ostream (&buf) {}
private:
    NullStreamBuffer buf;
};


Logger::Mode Logger::mode = Mode::OFF;
std::ostream* Logger::null_ostream = new NullOutStream();
