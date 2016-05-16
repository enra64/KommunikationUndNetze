#include "httpheader.h"

HttpHeader::HttpHeader(const std::string& line)
{
    size_t seperatorPosition = line.find(":");
    if(seperatorPosition != std::string::npos){
        mField = line.substr(0, seperatorPosition);
        mValue = line.substr(seperatorPosition + 2, std::string::npos);
    }
}

HttpHeader::HttpHeader(const std::string& field, const std::string &value) :
    mField(field),
    mValue(value){}

std::string HttpHeader::parsePath(std::string& line, bool& success){
    size_t httpPos = line.find(" HTTP/");

    // truncate to path
    line = "htdocs" + line.substr(4, httpPos - 4);

    // no http found
    if(httpPos == std::string::npos)
        success = false;

    // "/" is the whole address
    if(line.length() == 7)
        return "htdocs/index.html";

    return line;
}

bool HttpHeader::parseCompleteHeader(std::string& header, std::vector<HttpHeader>& headerList)
{
    // better not to try and parse that
    if(header.find("GET") != 0)
        return false;

    size_t delimiterPosition;
    bool pathParsingSuccess = true;
    while((delimiterPosition = header.find("\r\n")) != std::string::npos){
        std::string line = header.substr(0, delimiterPosition);
        if(line.find("GET ") == 0)
            headerList.push_back(HttpHeader("GET", parsePath(line, pathParsingSuccess)));
        else
            headerList.push_back(HttpHeader(line));
        header.erase(0, delimiterPosition + 2);
    }
    return pathParsingSuccess && headerList.size() > 0;
}
