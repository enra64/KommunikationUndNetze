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

std::string HttpHeader::parsePath(std::string line){
    // truncate to path
    line.resize(line.find(" HTTP/"));

    // remove GET and prepend htdocs
    std::string path = "htdocs" + std::string(line, 4);

    // send index if no file is specified
    if(path.length() == 7)
        path = "htdocs/index.html";

    return path;
}

bool HttpHeader::parseCompleteHeader(std::string& header, std::vector<HttpHeader>& headerList)
{
    if(header.find("GET") != 0)
        return false;

    size_t delimiterPosition;
    while((delimiterPosition = header.find("\r\n")) != std::string::npos){
        std::string line = header.substr(0, delimiterPosition);
        if(line.find("GET ") == 0)
            headerList.push_back(HttpHeader("GET", parsePath(line)));
        else
            headerList.push_back(HttpHeader(line));
        header.erase(0, delimiterPosition + 2);
    }
    return headerList.size() > 0;
}
