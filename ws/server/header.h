#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <vector>

class Header
{
public:
    Header();
    Header(std::string line);
    Header(std::string field, std::string value);
    static bool parseCompleteHeader(std::string& header, std::vector<Header>& headerList);
    std::string getField() const;
    std::string getValue() const;
private:
    static std::string parsePath(std::string line);
    std::string mField;
    std::string mValue;
};

#endif // HEADER_H
