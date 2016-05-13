#include "header.h"

Header::Header(std::string field, std::string value) :
    mField(field),
    mValue(value){}

std::string Header::parsePath(std::string line){
    // find end of requested file path
    size_t httpLoc = line.find(" HTTP/");

    // truncate to path
    line.resize(httpLoc);

    // remove GET and prepend htdocs
    std::string path = "htdocs" + std::string(line, 4);

    // send index if no file is specified
    if(path.length() == 7)
        path = "htdocs/index.html";

    return path;
}

bool Header::parseCompleteHeader(std::string& header, std::vector<Header>& headerList)
{
    if(header.find("GET") != 0)
        return false;

    size_t delimiterPosition;
    while((delimiterPosition = header.find("\r\n")) != std::string::npos){
        std::string line = header.substr(0, delimiterPosition);
        if(line.find("GET ") == 0)
            headerList.push_back(Header("GET", parsePath(line)));
        else
            headerList.push_back(Header(line));
        header.erase(0, delimiterPosition + 2);
    }
    return headerList.size() > 0;
}

Header::Header(){

}

Header::Header(std::string line)
{
    size_t seperatorPosition = line.find(":");
    if(seperatorPosition != std::string::npos){
        mField = line.substr(0, seperatorPosition);
        mValue = line.substr(seperatorPosition + 2, std::string::npos);
    }
}

std::string Header::getField() const {
    return mField;
}

std::string Header::getValue() const {
    return mValue;
}
