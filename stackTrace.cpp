#include "stackTrace.h"

stackTrace::stackTrace() : std::ostringstream()
{
    unw_cursor_t cursor;
    unw_context_t context;

    /// Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    *this << std::endl << ">>>>>>>>>Start of stackdump <<<<<<<<<<<<" << std::endl;
    /// Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0)
    {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0)
        {
            break;
        }

        *this << "\t0x0" << std::hex << pc << " " ;
        char* line = (char *) calloc(1,LINSIZ);

        if (unw_get_proc_name(&cursor, line, LINSIZ, &offset) == 0)
        {

            int status;
            char* demangled = abi::__cxa_demangle(line, nullptr, nullptr, &status);
            if (status == 0)
            {
                *this << demangled << " 0x0" << std::hex << offset << std::endl;

            }else {
                *this << " 0x0" << std::hex << offset << std::endl;

            }
            std::free(line);
            std::free(demangled);
            line=nullptr;
            demangled = nullptr;

        }
        else
        {
             *this << "\t -- error: unable to obtain symbol name for this frame" << std::endl;
        }
        if (line != nullptr)
            free(line);
    }   /// end while
    *this << ">>>>>>>>>>>>>>> End of stackdump <<<<<<<<<<<<<" << std::endl;
}


stackTrace::~stackTrace()
{
    //dtor
}



