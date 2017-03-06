#ifndef STACKTRACE_H
#define STACKTRACE_H

#define UNW_LOCAL_ONLY
#include <cxxabi.h>
#include <libunwind.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

class stackTrace : public std::ostringstream
{
    public:
        /** Default constructor */
        stackTrace();
        /** Default destructor */
        virtual ~stackTrace();

    protected:
    const int LINSIZ = 512;
    private:
};

#endif // STACKTRACE_H
