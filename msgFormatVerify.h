#ifndef MSGFORMATVERIFY_H
#define MSGFORMATVERIFY_H

#include <string>
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>



class msgFormatVerify
{
    public:
        /** Default constructor */
        msgFormatVerify(const char* resultsFile ,const char* regexFile);
        /** Default destructor */
        virtual ~msgFormatVerify();

        bool compare_syntax(std::vector<std::string> & results);
        bool compare_syntax_severity(std::vector<std::string> & results);

    protected:

    private:
        std::ifstream m_dataFile;
        std::ifstream m_regexFile;
        std::string m_regexSyn;

};

#endif // MSGFORMATVERIFY_H
