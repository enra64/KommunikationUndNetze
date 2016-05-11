#ifndef HEADER_H
#define HEADER_H

#include <string>

class Header
{
public:
    Header();
    Header(std::string field, std::string value);
    std::string getField() const;
    std::string getValue() const;
private:
    std::string mField;
    std::string mValue;
};

#endif // HEADER_H
