#include "header.h"

Header::Header(std::string field, std::string value) : mField(field), mValue(value)
{

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
