#pragma once

#include<string>
#include<vector>

class VectorTimestamp
{
  public:
    std::vector<int64_t> vt;
    unsigned int processor;

    VectorTimestamp() {}
    VectorTimestamp(unsigned int p, unsigned int n);

    void Inc();
    void Update(const VectorTimestamp& vt);
    std::string ToJSON() const;

    bool operator< (const VectorTimestamp& vt);
    bool operator> (const VectorTimestamp& vt);

    friend VectorTimestamp ToVectorTimestamp(const std::string& json);
};
