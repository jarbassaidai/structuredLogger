#define BOOST_TEST_DYN_LINK
// either Suits or Fitxtures
#define BOOST_TEST_MODULE Suites
//#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include "structuredLogger.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include "msgFormatVerify.h"
#include "stackdumptest.h"
// define all the regular expressions for readability
#define DATEREG "(\\d+ \\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d+)"
#define SEVREG "(<[^>]+>)"
#define MEDREG "(\\[[^\\]]+\\])"
#define THREADREG "(\\[[\\dabcdefx]+\\])"
//#define NORMREG DATEREG SEVEREG
//#define DEBUGREG NORMREG MEDREG THREADREG

//
//  BOOST_WARN_EQUAL, BOOST_CHECK_EQUAL, BOOST_REQUIRE_EQUAL
//
//    BOOST_<level>
//    BOOST_<level>_BITWISE_EQUAL
//    BOOST_<level>_CLOSE
//    BOOST_<level>_CLOSE_FRACTION
//    BOOST_<level>_EQUAL
//    BOOST_<level>_EQUAL_COLLECTION
//    BOOST_<level>_EXCEPTION
//    BOOST_<level>_GE
//    BOOST_<level>_GT
//    BOOST_<level>_LE
//    BOOST_<level>_LT
//    BOOST_<level>_MESSAGE
//    BOOST_<level>_NE
//    BOOST_<level>_NO_THROW
//    BOOST_<level>_PREDICATE
//    BOOST_<level>_SMALL
//    BOOST_<level>_THROW
//    BOOST_ERROR
//    BOOST_FAIL
//    BOOST_IS_DEFINED
//
////
//class setup
//{
//    int m;
//
//    setup() : m(2)
//    {
//        BOOST_TEST_MESSAGE("setup mass");
//    }
//
//    ~setup()m0r3.23x
//    {
//        BOOST_TEST_MESSAGE("teardown mass");
//    }
//};
// either Fixture for setup/tear down  or
// Suite for non- tera down
//BOOST_FIXTURE_TEST_SUITE(Physics, Massive)

// The functions contained in this file are pretty dummy
// and are included only as a placeholder. Nevertheless,
// they *will* get included in the shared library if you
// don't remove them :)
//
// Obviously, you 'll have to write yourself the super-duper
// functions to include in the resulting library...
// Also, it's not necessary to write every function in this file.
// Feel free to add more files in this project. They will be
// included in the resulting library.

// highjack the main and make it the unitTest module
class setup {
public:
    setup() ;
    static void signalCatch(int);
protected:

};
setup::setup (){
    signal(SIGSEGV,&setup::signalCatch);
}
void setup::signalCatch(int sigNum) {
    ///t_spSL elog = structuredLogger::getInstance(ELOGNAME,(t_loggingStyle) (allInfoFile| tokenMsg | sepErrorFile | sepDebugFile));
    ///elog->stack_trace("crash & burn");
    ///elog.reset();
    std::cout << stackTrace().str() ;
}

extern "C"
{
BOOST_AUTO_TEST_SUITE(s_StructruedLogging)

BOOST_AUTO_TEST_CASE(general_logging)
{
    setup s;
    t_spSL elog = structuredLogger::getInstance("structLogUtest.1",(t_loggingStyle) (allInfoFile| tokenMsg | sepErrorFile | sepDebugFile));

    elog->generalL(normal,"message 1 GEN","test_case::msg");
    elog->generalL(normal,"msg normal 2 GEN");
    elog->generalL(warning,"msg warning GEN");
    elog->generalL(notification,"msg info GEN",__PRETTY_FUNCTION__);
    elog->generalL(critical,"msg fata/critial GEN","getLogger::general_logging");
    elog->generalL(debug,"msg debug GEN");
    elog->generalL(error,"msg error GEN");
    elog->generalL(warning,"warning GEN");
    elog->generalL(error,"error GEN",__PRETTY_FUNCTION__);
    std::string normreg(DATEREG);
    normreg.append(" ");
    normreg.append(SEVREG);
    normreg.append("(.*)");
    std::string xname (elog->getFname());
    xname.append(".log");
    msgFormatVerify msgFV(xname.c_str(),normreg.c_str());
    std::vector<std::string> results;
    BOOST_CHECK(msgFV.compare_syntax(results));
    std::ofstream severityCheckFile("normalSyntax.dat");
    severityCheckFile << normreg << std::endl << "<normal>" << std::endl << "<normal>" << std::endl << "<warning>" << std::endl << "<notification>" <<std::endl;
    severityCheckFile << "<critical>" << std::endl << "<debug>" << std::endl << "<error>" << std::endl << "<warning>" << std::endl << "<warning>" << std::endl;
    severityCheckFile << "<error>" << std::endl;
    severityCheckFile.close();

    msgFormatVerify msgFV1(xname.c_str(),"normalSyntax.dat");
    BOOST_CHECK(msgFV1.compare_syntax_severity(results));
    elog.reset();



}
BOOST_AUTO_TEST_CASE(tokenLogging)
{
      // token logging goes to the error file
      // since token is the method capture
      //
      setup s;
      t_spSL elog = structuredLogger::getInstance("structLogUtest.1",(t_loggingStyle)(allInfoFile | tokenMsg));

    elog->tokenL(normal,"message 1 TOKEN","test_case::msg");
    elog->tokenL(normal,"msg normal 2 TOKEN");
    elog->tokenL(warning,"msg warning TOKEN" );
    elog->tokenL(notification,"msg info TOKEN",__PRETTY_FUNCTION__);
    elog->tokenL(critical,"msg fatal/critical TOKEN ",__PRETTY_FUNCTION__);
    elog->tokenL(debug,"msg debug TOKEN");
    elog->tokenL(error,"msg error TOKEN","structuredLogger::junk(foo.bar)");
    elog->tokenL(warning,"warning TOKEN");
    elog->tokenL(error,"error TOKEN",__PRETTY_FUNCTION__);
    std::ostringstream tokenreg;
    tokenreg << DATEREG << " " << SEVREG << ".*?" << MEDREG << "(.*)";
    std::string r = tokenreg.str();
    std::string xname (elog->getFname());
    xname.append(".log");
    msgFormatVerify msgFV(xname.c_str(),r.c_str());
    std::vector<std::string> results;
    BOOST_CHECK(msgFV.compare_syntax(results));
    std::ofstream severityCheckFile("tokenSyntax.dat");
    severityCheckFile << r << std::endl << "<critical>" << std::endl << "<debug>" << std::endl << "error";
    severityCheckFile << std::endl << "<error>" << std::endl;

    severityCheckFile << std::endl << "<critical>" << std::endl << "<debug>" << std::endl << "error";
    severityCheckFile << std::endl << "<error>" << std::endl;

    severityCheckFile << std::endl << "<critical>" << std::endl << "<debug>" << std::endl << "error";
    severityCheckFile << std::endl << "<error>" << std::endl;

    severityCheckFile << std::endl << "<critical>" << std::endl << "<debug>" << std::endl << "error";
    severityCheckFile << std::endl << "<error>" << std::endl;
    severityCheckFile.close();
    xname =elog->getFname();
    xname.append(".error");
    msgFormatVerify msgFV1(xname.c_str(),"tokenSyntax.dat");
    BOOST_CHECK(msgFV1.compare_syntax_severity(results));
    elog.reset();

}
BOOST_AUTO_TEST_CASE(debugLogging)
{
    setup s;
      t_spSL elog = structuredLogger::getInstance("structLogUtest.1",sepDebugFile);

    elog->debugL(normal,"message 1 DEBUG","test_case::msg");
    elog->debugL(normal,"msg normal 2 DEBUG");
    elog->debugL(warning,"msg warning DEBUG");
    elog->debugL(notification,"msg info DEBUG",__PRETTY_FUNCTION__);
    elog->debugL(critical,"msg fatal/critical DEBUG ",__PRETTY_FUNCTION__);
    elog->debugL(debug,"msg debug DEBUG",__PRETTY_FUNCTION__);
    elog->debugL(error,"msg error DEBUG","structuredLogger::junk(foo.bar)");
    elog->debugL(warning,"warning DEBUG");
    elog->debugL(error,"error DEBUG",__PRETTY_FUNCTION__);
    std::ostringstream tokenreg;
    tokenreg << DATEREG << " " << SEVREG << ".*?" << MEDREG <<".*?" << THREADREG <<"(.*)";
    std::string r=tokenreg.str();
    std::string xname (elog->getFname());
    xname.append(".debug");
    msgFormatVerify msgFV(xname.c_str(),r.c_str());
    std::vector<std::string> results;
    bool bcheck = msgFV.compare_syntax(results);
    BOOST_CHECK(bcheck);
    if (! bcheck){
     std::cerr << "failed to match the following lines with regex" << r << std::endl;
     for (std::vector<std::string>::iterator it = results.begin(); it != results.end(); ++it){
        std::cerr << *it << std::endl;
        }
    }
    std::ofstream severityCheckFile("debugSyntax.dat");
    severityCheckFile << r << std::endl << "<debug>" << std::endl << "<debug>" << std::endl << "<debug>";
    severityCheckFile << std::endl << "<debug>" << std::endl;
    severityCheckFile.close();

    msgFormatVerify msgFV1(xname.c_str(),"debugSyntax.dat");
    BOOST_CHECK(msgFV1.compare_syntax_severity(results));
    elog->stack_trace("testing stack_trace dump ","xxx");
    elog.reset();
}
BOOST_AUTO_TEST_CASE(errorLogging)
{
    setup s;
      t_spSL elog = structuredLogger::getInstance("structLogUtest.1",sepErrorFile);

    elog->errorL(normal,"message 1 ER","test_case::msg");
    elog->errorL(normal,"msg normal 2 ER");
    elog->errorL(warning,"msg warning ER");
    elog->errorL(notification,"msg info ER",__PRETTY_FUNCTION__);
    elog->errorL(critical,"msg fatal/critical ER",__PRETTY_FUNCTION__);
    elog->errorL(debug,"msg debug ER");
    elog->errorL(error,"msg error ER","structuredLogger::junk(foo.bar)");
    elog->errorL(warning,"const char* warning ER");
    elog->errorL(error,"const char* error ER",__PRETTY_FUNCTION__);
    elog->errorL(error,"const char* error ER",__FILE__);
    std::ostringstream tokenreg;
    tokenreg << DATEREG << " " << SEVREG << ".*" << MEDREG << "(.*)";
    std::string r = tokenreg.str();
    std::string xname (elog->getFname());
    xname.append(".error");
    msgFormatVerify msgFV(xname.c_str(),r.c_str());
    std::vector<std::string> results;
    BOOST_CHECK(msgFV.compare_syntax(results));
    std::ofstream severityCheckFile("errorSyntax.dat");
    severityCheckFile << r << std::endl << "<critical>" << std::endl << "<debug>" << std::endl << "error";
    severityCheckFile << std::endl << "<error>" << std::endl;
    severityCheckFile.close();

    msgFormatVerify msgFV1(xname.c_str(),"errorSyntax.dat");
    BOOST_CHECK(msgFV1.compare_syntax_severity(results));
    stackdumptest sdt(elog,1);
    sdt.levelN(2);
    sdt.levelN(3);
    sdt.levelN(4);
    elog.reset();

}

BOOST_AUTO_TEST_CASE(timer)
{
    setup s;
   t_spSL  elog = structuredLogger::getInstance("structLogUtest.1");
   elog->timed_Start("timing call start",debug);
   elog->timed_Stop("timing call stop",debug);
   elog.reset();

}

BOOST_AUTO_TEST_CASE(scope)
{
    setup s;
    std::string method(__FUNCTION__);
    t_spSL  elog = structuredLogger::getInstance("structLogUtest.1");
    elog->named_scopeL(normal,"test normal",method);
    elog->named_scopeL(warning,"test warning");
    elog->named_scopeL(notification,"test info");
    elog->named_scopeL(critical,"test fatal");
    elog->named_scopeL(debug,"test debug");
    elog->named_scopeL(error,"test error");
    elog.reset();

}

BOOST_AUTO_TEST_CASE(default_test)
{
    setup s;
    t_spSL  elog = structuredLogger::getInstance("structLogUtest.1");
    elog.reset();
}

/*
BOOST_AUTO_TEST_CASE(tag)
{

    t_spSL  elog = structuredLogger::getInstance("structLogUtest.2");
    elog->taggedL("tagA",normal,"test normal");
    elog->taggedL("tagB",warning,"test warning");
    elog->taggedL("tagC",notification,"test info");
    elog->taggedL("tagD",critical,"test fatal");
    elog->taggedL("tagE",debug,"test debug");
    elog->taggedL("tagF",error,"test error");
}


*/

BOOST_AUTO_TEST_SUITE_END()

}
