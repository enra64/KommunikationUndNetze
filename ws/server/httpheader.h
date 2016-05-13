#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <vector>

class HttpHeader
{
public:
    HttpHeader(const std::string& line);
    HttpHeader(const std::string& field, const std::string& value);
    static bool parseCompleteHeader(std::string& header, std::vector<HttpHeader>& headerList);

    inline std::string getField() const{
        return mField;
    }

    inline std::string getValue() const{
        return mValue;
    }

private:
    static std::string parsePath(std::string line);
    std::string mField;
    std::string mValue;
};

#endif // HEADER_H
