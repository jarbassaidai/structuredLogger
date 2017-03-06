#ifndef STRUCTUREDLOGGER_H
#define STRUCTUREDLOGGER_H



#include <cstddef>
#include <string>
#include <ostream>
#include <fstream>
#include <iomanip>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/support/exception.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "stackTrace.h"


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

static std::string blank;


    typedef enum severity_level
    {
        normal=0,
        notification,
        warning,
        error,
        critical,
        debug,
        stacktrace
    }e_severity_level;

    typedef enum file_logger
    {
        singleFile = 0x1,
        allInfoFile = 0x2,
        sepErrorFile = 0x4, // severity >= error
        sepDebugFile = 0x8, // severity == debug
        sepSyslog = 0x10,
        sepSocket = 0x20,
        tokenMsg = 0x40, // allows for one token supplied by call to be viewed  will accept all severities

    }t_loggingStyle;


typedef expr::channel_severity_filter_actor< std::string, severity_level > min_severity_filter;
typedef sinks::asynchronous_sink< sinks::text_ostream_backend > text_sink;
typedef src::severity_logger_mt<
    severity_level     // the type of the severity level
//    std::string         // the type of the channel name
> my_logger_mt;

// either for reading or storing the value
typedef attrs::mutable_constant<
    std::string,                                // attribute value type
    boost::shared_mutex,                               // synchronization primitive
    boost::unique_lock<boost::shared_mutex >,           // exclusive lock type
    boost::shared_lock<boost::shared_mutex>
> exclusive_mc;

class structuredLogger;

typedef std::shared_ptr<structuredLogger> t_spSL;

template < class T>
class singleton{
public:
    std::shared_ptr<T> static getInstance()
    {
        static std::shared_ptr<T> s_instance;
        if (s_instance.get() == nullptr){
            s_instance.reset(new T );
        }
        return s_instance;
    }
};



std::ostream& operator<< (std::ostream& strm, severity_level);

class structuredLogger
{
    public:

        virtual ~structuredLogger();
   //     std::ostream& operator<< (std::ostream& strm, severity_level);
        void logMsg(severity_level level, const char *msg);
        void tokenL(severity_level level,const char * msg,const char * method = __PRETTY_FUNCTION__);
        void debugL(severity_level level,const char * msg,const char * method=__PRETTY_FUNCTION__);
        void errorL(severity_level level,const char * msg,const char * method=__PRETTY_FUNCTION__);
        void timed_Start(const char* item, severity_level level);
        void timed_Stop(const char* item, severity_level level);
        void named_scopeL( severity_level, const char * ,std::string &scope= blank);
        void stack_trace(const char* item,const char * method=__PRETTY_FUNCTION__);
        void inline generalL(severity_level l, const char *msg , const char * token=__PRETTY_FUNCTION__) { return general_logging(msg,l,token);}
        std::shared_ptr<structuredLogger> static getInstance(const char * logname, t_loggingStyle logStyle=allInfoFile);
        inline std::string &  getFname() { return m_fname; }
#if 0
        void inline setSeverity_level(severity_level l) { m_sev_level = l; }
        void inline setOneFile() { m_enabledLoggers = (unsigned long) allInOneFile;}
        void inline setSepDebugFile() {m_enabledLoggers |= (unsigned long) sepDebugFile; }
        void inline setSepInfoFile() {m_enabledLoggers |= (unsigned long) sepInfoFile; }
        void inline setSepErrorFile() {m_enabledLoggers |= (unsigned long) sepErrorFile; }
        void inline setDebug(bool v) {m_debug = v;}
        bool inline isDebug() { return m_debug;}
#endif

        void init(const char * logname, t_loggingStyle logStyle=allInfoFile);

    protected:
        friend class singleton<structuredLogger>;
        structuredLogger() {};
        structuredLogger(const char * logname, t_loggingStyle logStyle= allInfoFile);
        void general_logging(const char *, severity_level l=normal,const char * method=__FUNCTION__);
        void general_logging(std::string &, severity_level l=normal, std::string &method= blank);
        void debugFormatter () ;
        void tokenFormatter() ;
        void normalFormatter();
        std::string m_fname;

        //src::severity_channel_logger< severity_level, std::string > *m_slg;
        //severity_level m_sev_level;
        bool m_debug; // set if debug logging is setup
        unsigned long m_enabledLoggers;
        static min_severity_filter m_min_severity;
        static exclusive_mc m_tokenattr;

        std::string m_blank();

        logging::formatter m_fmtDebug;
        logging::formatter m_fmtToken;
        logging::formatter m_fmtNormal;
        boost::shared_ptr< text_sink > m_sinkDebug;
        boost::shared_ptr< text_sink > m_sinkError;
        boost::shared_ptr< text_sink > m_sinkNormal;

    private:
        bool m_init=false;
        bool m_deleteInProgress = true;
};

#endif // STRUCTUREDLOGGER_H
