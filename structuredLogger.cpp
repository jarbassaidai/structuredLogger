#include "structuredLogger.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/container/map.hpp"
#include "structuredLogger.h"
std::string FormatTime();

///
///\class structuredLogger
///<single logger for all logging within a program
///< counter is the error record number
///< scoped logging only works with macros as it needs to be in the actual scope
///< tagged records are much like scoped and are one time only which seems useless
//
//BOOST_LOG_ATTRIBUTE_KEYWORD(file,"File",std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(channel,"Channel",std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(token_attr, "Token", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(scope1, "Scope1", std::string )
BOOST_LOG_ATTRIBUTE_KEYWORD(timeline, "Timeline", attrs::timer::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id,"ThreadID",attrs::current_thread_id::value_type)


min_severity_filter structuredLogger::m_min_severity = expr::channel_severity_filter(channel,severity);

// This mutable constant will always lock exclusively
exclusive_mc structuredLogger::m_tokenattr("method");
///\fn getInstance
///\arg const char * logname
///\arg t_loggingStyle
///\brief get single instance logger

std::shared_ptr<structuredLogger>  structuredLogger::getInstance(const char * logname, t_loggingStyle logStyle)
{
     singleton<structuredLogger> s_instance ;
     std::shared_ptr<structuredLogger> rVal = s_instance.getInstance();
     rVal->init(logname,logStyle);
      return rVal;
}
///\fn getInstanceByName
///\arg const char * logname
///\arg t_loggingStyle
///\brief multiple instances by name

std::shared_ptr<structuredLogger>  structuredLogger::getInstanceByName(const char * logname, t_loggingStyle logStyle)
{
    static boost::container::map<std::string, structuredLogger * > m_fileMap;
    boost::container::map<std::string, structuredLogger*>::iterator it;
    if (m_fileMap.empty()) {
        m_fileMap.insert(std::pair<std::string,structuredLogger *>(std::string(logname),new structuredLogger(logname,logStyle)) );

    }else {

        it = m_fileMap.find(std::string(logname));
        if (it == m_fileMap.end()) {
            m_fileMap.insert(std::pair<std::string,structuredLogger *>(std::string(logname),new structuredLogger(logname,logStyle)) );
        }
    }

    it = m_fileMap.find(std::string(logname));

     return (std::shared_ptr<structuredLogger>(it->second));
}
structuredLogger::structuredLogger(const char *logname,t_loggingStyle logStyle)
{
    init(logname,logStyle);
}
///\fn init
///\arg const char * logname/filespec [in]
///\arg t_loggingStyle  logstyle [in]
///\< initialize the logger with a logname/filespec
///\< set the 3 possible logging favors debug, logs everything
///\< token, logs only those tokens that have been specified
///\< normal anything that is not abnormal
///\< open up 0-3 files "name.log", "name.error", and "name.debug"

void structuredLogger::init(const char * logname,t_loggingStyle logStyle)
{
    if (!m_init) {
        if (!(logStyle & (singleFile|allInfoFile|sepErrorFile|sepDebugFile))) {
            struct badLoggingStyle b;
            throw(b);
        }
        if (logname == nullptr)
            logname = "program";
        ///set up log name for rotation
        m_fname.append(logname);

        m_fname.append(FormatTime());

        m_init = true;

    // set defaults
    //m_debug = false;
    m_enabledLoggers=logStyle;
    /**
    *
    // full message looks like
    // logRecordNumber yyyy-mm-dd hh:mm:ss <severity>  [scope] [thread] [timing] message ** debug pluse timing
    //  logRecordNumber yyyy-mm-dd hh:mm:ss <severity> message  ** normal logging
    //
    **/
    debugFormatter();
    tokenFormatter();
    normalFormatter();

    /// normal log on by default
    /// allInfoFile covers severity normal through warning
    if (m_enabledLoggers & allInfoFile)
    {
        std::string lname(m_fname);
        lname += ".log";
                                          ///  boost::shared_ptr< text_sink >  sink = boost::make_shared< text_sink >() ;
        boost::shared_ptr< text_sink >  sink(new text_sink());
                                                        ///keywords::file_name=lname.c_str(),
                                                        ///keywords::rotation_size = ROTATIONSIZE_1MB));
        m_sinkNormal = sink;

        sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(lname.c_str() ));
        sink->locked_backend()->auto_flush(true);
        if (m_enabledLoggers & tokenMsg) {
            sink->set_formatter(m_fmtToken);
        }else {
            sink->set_formatter(m_fmtNormal); //&structuredLogger::formatter); //(fmt);
        }
        sink->set_filter(severity <= (e_severity_level) warning );
                                          //sink->set_filter(structuredLogger::m_min_severity || severity >= normal );
        logging::core::get()->add_sink(sink);
        sink.reset();
    }
                                          /// error log
    /// todo make this optional
    /// add a separate <name><date>.error file covering severity error through stacktrace
    if (m_enabledLoggers & sepErrorFile)
    {
        std::string lname(m_fname);
        lname += ".error";
        ///  boost::shared_ptr< text_sink >  sink = boost::make_shared< text_sink >() ;
        boost::shared_ptr< text_sink >  sink(new text_sink());
                                                        ///keywords::file_name=lname.c_str(),
                                                        ///keywords::rotation_size = ROTATIONSIZE_1MB));       std::string lename(logname);

        ///sink = boost::make_shared<text_sink>();
        m_sinkError = sink;
        sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(lname.c_str() ));
        sink->locked_backend()->auto_flush(true);
        sink->set_formatter(m_fmtToken);
        sink->set_filter(severity >= error );
        logging::core::get()->add_sink(sink);
        sink.reset();
    }

    /// debug log
    /// todo make this optional
    /// separate file covering severity debug through stacktrace
    if (m_enabledLoggers & sepDebugFile)
    {
        std::string lname(m_fname);
        lname += ".debug";
                                          ///  boost::shared_ptr< text_sink >  sink = boost::make_shared< text_sink >() ;
        boost::shared_ptr< text_sink >  sink(new text_sink());
                                                        ///keywords::file_name=lname.c_str(),
                                                        ///keywords::rotation_size = ROTATIONSIZE_1MB));
        ///sink = boost::make_shared<text_sink>();
        m_sinkDebug = sink;
        sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(lname.c_str() ));
        sink->locked_backend()->auto_flush(true);
        sink->set_formatter(m_fmtDebug);
        sink->set_filter(severity >= debug );

        logging::core::get()->add_sink(sink);
        sink.reset();
    }
    /// single file covering severity normal through stacktrace
    if (m_enabledLoggers & singleFile)
    {
          std::string lname(m_fname);
        lname += ".everything";
                                          ///  boost::shared_ptr< text_sink >  sink = boost::make_shared< text_sink >() ;
        boost::shared_ptr< text_sink >  sink(new text_sink());
                                                        ///keywords::file_name=lname.c_str(),
                                                        ///keywords::rotation_size = ROTATIONSIZE_1MB));
        ///sink = boost::make_shared<text_sink>();
        m_sinkDebug = sink;
        sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(lname.c_str() ));
        sink->locked_backend()->auto_flush(true);
        sink->set_formatter(m_fmtDebug);
        sink->set_filter(severity <= stacktrace );

        logging::core::get()->add_sink(sink);
        sink.reset();
    }

/*
    attrs::mutable_constant<std::string> keyWordAttr("INFORMATION");
    attrs::mutable_constant<std::string> scope1Attr("none");
    m_keyWordAttr = &keyWordAttr;
    m_scope1Attr = &scope1Attr;

    logging::core::get()->add_global_attribute("Token",keyWordAttr);
    logging::core::get()->add_global_attribute("Scope1",scope1Attr);
 */
    /// Add attributes
    logging::add_common_attributes(); /// LineID, TimeStamp, ProcessID ThreadID
    attrs::mutable_constant<std::string> tokenAddr("method");
    src::severity_logger_mt< severity_level > slg;
    slg.add_attribute("Token",structuredLogger::m_tokenattr);
    logging::core::get()->add_global_attribute("Token",structuredLogger::m_tokenattr);


//    keywords::auto_flush=true;
//    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());   //ctor
//    logging::core::get()->add_global_attribute("KeyWord",attrs::mutable_constant<std::string>m_keyWordAttr);
//    logging::core::get()->add_global_attribute("Tag",attrs::tag_attr);
    }/** end if not initialized */

}
///\fn debugFormatter
///\arg none
///\return none
///< creates the debug formatter with the following out put form
///< full message looks like
///< logRecordNumber yyyy-mm-dd hh:mm:ss <severity>  [scope] [thread] [timing] message ** debug plus timing

void structuredLogger::debugFormatter() {
///logging::formatter
 m_fmtDebug =     expr::stream
                                 << line_id << " "
                                 << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                                 <<" <" << severity << "> "
                                  << expr::if_(expr::has_attr(token_attr) ) // get method for call
                                 [
                                     expr::stream << "[" << token_attr << "] "
                                 ]



                                 << expr::if_(expr::has_attr(thread_id) )  // get thread id
                                 [
                                    expr::stream << "[" << thread_id << "] "
                                 ]

                                 << expr::if_(expr::has_attr(timeline))
                                 [
                                     expr::stream << "[" << timeline << "] "
                                 ]


                                // << expr::smessage;  //<< std::endl
                                << expr::if_(expr::has_attr(scope1))
                                 [
                                    expr::stream
                                    << expr::format_named_scope(
                                    "Scopes",
                                    keywords::format = "%n (%f:%l)",
                                    keywords::iteration = expr::reverse)
                                 ]

                                << expr::smessage;  //<< std::endl
}
///\fn tokenFormatter
///\arg none
///\return none
///<  logRecordNumber yyyy-mm-dd hh:mm:ss <severity> [token] message  ** normal logging
void structuredLogger::tokenFormatter(){
   ///logging::formatter
    m_fmtToken =     expr::stream
                                 << line_id << " "
                                 << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                                 <<" <" << severity << "> "
                                  << expr::if_(expr::has_attr(token_attr) ) // get method for call
                                 [
                                     expr::stream << "[" << token_attr << "] "
                                 ]
                               << expr::smessage ;
}
///\fn normalFormatter
///\arg none
///\return none
///<  logRecordNumber yyyy-mm-dd hh:mm:ss <severity> message  ** normal logging
void structuredLogger::normalFormatter() {

    ///logging::formatter
    m_fmtNormal =     expr::stream
                                 << line_id << " "
                                 << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                                 <<" <" << severity << "> "
                                 << expr::smessage ;

}
///\fn ~stucturedLogger
///\arg none
///\return none
///< just remove all sinks and set m_init to false
structuredLogger::~structuredLogger()
{
    if (! m_deleteInProgress) {
        m_deleteInProgress = true;

        //logging::core::get()->remove_all_sinks();

        boost::shared_ptr<logging::core> core = logging::core::get();

        //std::cout  << " normalSink count:" << m_sinkNormal.use_count() << std::endl;
        if (m_sinkNormal.get() != nullptr){
            m_sinkNormal->flush();
            m_sinkNormal->stop();
            //core->remove_sink(m_sinkNormal);
            m_sinkNormal.reset();
        }


       // core->remove_sink(m_sinkDebug);
        if (m_sinkDebug.get() != nullptr){
            m_sinkDebug->flush();
            m_sinkDebug->stop();
            m_sinkDebug.reset();
        }

        //core->remove_sink(m_sinkError);
        if (m_sinkError.get() != nullptr){
            m_sinkError->flush();
            m_sinkError->stop();
            m_sinkError.reset();
        }
        m_init = false;
        m_deleteInProgress = false;
    }
}
///\fn logMsg
///\arg severity_level [in]  severity_level
///\arg const char * msg [in] message to be logged
///< log a message , severity level will determine which file/s the message will be logged

void structuredLogger::logMsg(severity_level level, const char * msg)
{
    src::severity_logger_mt< severity_level > slg;
        BOOST_LOG_SEV(slg, level) << msg;

}

//[ example_tutorial_attributes_named_scope
// scope only takes a literal const char *& which can not be passed as a variable  from the the macro __FUNCTION__
// With that in mind we will not use the macro BOOST_LOG_NAMED_SCOPE instead we will just add the
// default argument in as part of the string
///\fn named_scopeL
///\arg severity_level [in]  severity_level
///\arg const char * msg [in] message to be logged
///\arg std::string & scope name
///< log a message , severity level will determine which file/s the message will be logged

void  structuredLogger::named_scopeL(severity_level level, const char * msg,std::string &scope)
{
    //src::severity_logger< severity_level > slg;
    src::severity_logger_mt< severity_level > slg;
  std::string tmag("[");
    tmag.append(scope);
    tmag.append("]");

   // m_scope1Attr->set(scope);
    BOOST_LOG_SEV(slg, level) << tmag.c_str() << msg;
}
//]

//[ example_tutorial_attributes_tagged_logging
///\fn tokenL
///\arg severity_level [in]  severity_level
///\arg const char * msg [in] message to be logged
///\arg const char * method name
///< log a message , severity level will determine which file/s the message will be logged
///< log a token message ie by method name
void structuredLogger::tokenL(severity_level level,const char * msg, const char* method)
{

    src::severity_logger_mt< severity_level > slg;
    structuredLogger::m_tokenattr.set(method);

    //m_keyWordAttr->set(tag);
    BOOST_LOG_SEV(slg, level) << msg ;
    //m_slg->delete_attribute("Tag")
}
//]
//[ example_tutorial_attributes_tagged_logging
///\fn  debugL
///\arg severity_level [in]  severity_level
///\arg const char * msg [in] message to be logged
///\arg const char * method name [in]
///< log a message , with a debug message

void structuredLogger::debugL(severity_level level,const char * msg,const char* method)
{

    src::severity_logger_mt< severity_level > slg;
    structuredLogger::m_tokenattr.set(method);
    BOOST_LOG_SEV(slg, level) << msg ;

}
///\fn logMsg
///\arg severity_level [in]  severity_level
///\arg const char * msg [in] message to be logged
///\arg const char * method name
///< log a message to the error log

void structuredLogger::errorL(severity_level l,const char * msg, const char * method)
{
  src::severity_logger_mt< severity_level > slg;
  structuredLogger::m_tokenattr.set(method);
  BOOST_LOG_SEV(slg,l) << msg;
 #if 0   //attrs::Severity.set(l);
    structuredLogger::m_tokenattr.set(method);
    //BOOST_LOG_SEV(slg,severityL) < msg;

    logging::record rec = slg.open_record(keywords::severity =l); //keywords::severity = normal,keywords::channel);
    if (rec)
    {
        logging::record_ostream strm(rec);
        strm << msg;
        strm.flush();
        slg.push_record(boost::move(rec));
    }
#endif
}
/**
*   the following methods have not been tested
*/

//[ example_tutorial_attributes_timed_logging
void structuredLogger::timed_Start(const char* item, severity_level level)
{
    BOOST_LOG_SCOPED_THREAD_ATTR("Timeline", attrs::timer());
    src::severity_logger_mt< severity_level > slg;
    //src::severity_logger< severity_level > slg;
    BOOST_LOG_SEV(slg, level) << "starting timing " << item;
}
void structuredLogger::timed_Stop( const char * item, severity_level level)
{
    src::severity_logger_mt< severity_level > slg;
    BOOST_LOG_SEV(slg, level) << "Stopping timing " << item;
}
/**
*  general_logging called from inline generalL
**/
///\fn general_logging
///\arg string msg
///\arg severity_level
///\arg string method

void structuredLogger::general_logging(std::string &msg, severity_level l ,std::string &method)
{
    general_logging(msg.c_str(),l,method.c_str());
}
void structuredLogger::general_logging(const char* msg,severity_level l,const char* method)
{
    src::severity_logger_mt< severity_level > slg;
    //attrs::Severity.set(l);
    structuredLogger::m_tokenattr.set(method);
    //BOOST_LOG_SEV(slg,severityL) < msg;

    logging::record rec = slg.open_record(keywords::severity = l); //keywords::severity = normal,keywords::channel);
    if (rec)
    {
        logging::record_ostream strm(rec);
        strm << msg;
        strm.flush();
        slg.push_record(boost::move(rec));
    }

}
///\fn stack_trace
///\arg const char * item
///\arg const char * method name of the caller
///< force a stack dump and log it to debug and error
void structuredLogger::stack_trace(const char* item,const char * method)
{
    src::severity_logger_mt< severity_level > slg;
    structuredLogger::m_tokenattr.set(method);
    BOOST_LOG_SEV(slg, debug) << stackTrace().str() ;
}
#if 0
void structuredLogger::formatter(logging::record_view const& rec, logging::formatting_ostream& strm)
{
    structuredLogger sl = structuredLogger::getLogger(nullptr);
    strm << line_id << " "
         << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
         <<" <" << severity << "> "
         << expr::if_(expr::has_attr(token_attr) ) // get method for call
         [
           expr::stream << "[" << token_attr << "] "
         ]
/*
          << expr::if_(expr::has_attr(scope1))
         [
         expr::stream << "(" << scope1 << ") "
         ]
*/
         << expr::if_(expr::has_attr(thread_id) )  // get thread id
        [
           expr::stream << "[" << thread_id << "] "
        ]
/*
         << expr::if_(expr::has_attr(timeline))
         [
         expr::stream << "[" << timeline << "] "
        ]
*/
         << rec[expr::smessage] ;
}
#endif
//
// needs to be in try block of boost
//void structuredLogger::stackDumpL(const char * msg){
//system call backtrace
//
//    BOOST_LOG_SEV(*m_slg,critical) << logging::current_scope();
//
//}
//]

/// The operator puts a human-friendly representation of the severity level to the stream
///\fn std::ostream& structuredLogger::operator<<
///\arg (std::ostream& strm,
///\arg  severity_level level)
/// <protected call that converts severity to a string
std::ostream& operator<< (std::ostream& strm, severity_level level)
{
    static const char* sevStrings[] =
    {
        "normal",
        "notification",
        "warning",
        "error",
        "critical",
        "debug",
        "stacktrace"
    };

    if (static_cast< std::size_t >(level ) < sizeof(sevStrings) / sizeof(*sevStrings))
        strm << sevStrings[level];
    else
        strm << static_cast< int >(level );

    return strm;
}

std::string FormatTime()
{
  using namespace boost::posix_time;
  ptime now = second_clock::universal_time();
  static std::locale loc(std::cout.getloc(),
                         new time_facet("%Y.%m.%d_%H.%M.%S"));

  std::stringstream wss;
  wss.imbue(loc);
  wss << now;
  return wss.str();
}
