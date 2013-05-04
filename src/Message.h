#pragma once

#include "VectorTimestamp.h"

class Message
{
  public:
    std::string value;
    VectorTimestamp vt;

    Message() {}
    Message(const std::string& value, const VectorTimestamp& vt);

    std::string ToJSON() const;

    friend Message ToMessage(const std::string& json);
    friend std::ostream& operator<< (std::ostream& out, const Message& m);
};

Message ToMessage(const std::string& json);
std::ostream& operator<< (std::ostream& out, const Message& m);
