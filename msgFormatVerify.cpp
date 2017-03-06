#include "msgFormatVerify.h"


    // current message formats to veri:e:efy probably should be read from a file for
    // logRecordNumber yyyy-mm-dd hh:mm:ss <severity>  [scope] [thread] [timing] message ** debug pluse timing
    //  logRecordNumber yyyy-mm-dd hh:mm:ss <severity> message  ** normal logging


msgFormatVerify::msgFormatVerify(const char * filename, const char *regexFile)
{

    m_dataFile.open(filename);
    boost::filesystem::path p(regexFile);
    boost::system::error_code bsyserr;
    if (boost::filesystem::exists(p,bsyserr)) {
        m_regexFile.open(regexFile);
    }else {
        m_regexSyn.append(regexFile);
    }
}

msgFormatVerify::~msgFormatVerify()
{
    //dtor
}
// give a file and a regular expression
// return bool and any lines that don't match
bool msgFormatVerify::compare_syntax(std::vector<std::string> & results)
{
    std::string line;

    bool rval = m_dataFile.is_open();


    m_dataFile.seekg(0,m_dataFile.beg);
    boost::regex expr(m_regexSyn.c_str(),boost::regex::perl|boost::match_partial);
    boost::cmatch found;

    while (getline(m_dataFile,line))
    {
        if (boost::regex_match(line.c_str(),found,expr )) {
            results.push_back(found[2].first);
        }else{
            rval = false;
            results.push_back(found[0].first);
        }

    }

    return rval;
}

// give a file and a file with regular expression and a list of severities
// return bool and any lines that don't match
bool msgFormatVerify::compare_syntax_severity(std::vector<std::string> & results)
{
    std::string line;
    std::string severity;
    bool rval = true;
    getline(m_regexFile,m_regexSyn);  // read off first line for pattern
    boost::regex expr(m_regexSyn.c_str(),boost::regex::perl|boost::match_partial);
    boost::cmatch found;

    while (getline(m_dataFile,line))
    {
        if (boost::regex_match(line.c_str(),found,expr) ) {

            getline(m_regexFile,severity);
            boost::regex svexpr(severity.c_str(), boost::regex::perl|boost::match_partial);
            std::string v(found[1].second);
            if(boost::regex_match(v.c_str(),found,svexpr)){
                results.push_back(line.c_str());
                rval=false;
            }

        }else{
            rval = false;
            results.push_back(found[0].first);
        }

    }
    return rval;
}
