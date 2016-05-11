#include "header.h"

Header::Header(std::string field, std::string value) : mField(field), mValue(value)
{

}

Header::Header(){

}

std::string Header::getField() const {
    return mField;
}

std::string Header::getValue() const {
    return mValue;
}
