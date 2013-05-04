#include "VectorTimestamp.h"

#include <sstream>

#include <json/json.h>
using namespace std;

ostream& operator<< (ostream& out, const VectorTimestamp& vt)
{
  out << vt.ToJSON();

  return out;
}

VectorTimestamp::VectorTimestamp(unsigned int p, unsigned int n)
{
  this->processor = p;
  vt = vector<int64_t>(n, 0);
}

void VectorTimestamp::Inc()
{
  this->vt[processor]++;
}

void VectorTimestamp::Update(const VectorTimestamp& vt)
{
  int size = vt.vt.size();
  for (int i = 0; i < size; i++)
    this->vt[i] = max(this->vt[i], vt.vt[i]);
}

std::string VectorTimestamp::ToJSON() const
{
  stringstream ss;

  ss << "[";
  int size = this->vt.size();
  bool first = true;
  for (int i = 0; i < size; i++)
  {
    if (first)
    {
      ss << vt[i];
      first = false;
    }
    else
    {
      ss << ", " << vt[i];
    }
  }
  ss << "]";

  return ss.str();
}

//TODO: compare
bool VectorTimestamp::operator< (const VectorTimestamp& vt)
{
  int size = this->vt.size();
  for (int i = 0; i < size; i++)
    if (this->vt[i] > vt.vt[i])
      return false;

  return true;
}

bool VectorTimestamp::operator> (const VectorTimestamp& vt)
{
  return !(*this < vt);
}

VectorTimestamp ToVectorTimestamp(const std::string& json)
{
  Json::Value root;
  Json::Reader reader;
  bool isSuccess = reader.parse(json, root);
  if (!isSuccess)
  {
    cout << "error in json syntax" << endl;
  }
  
  int size = root.size();
  VectorTimestamp vt(-1, size);
  for (int i = 0; i < size; i++)
  {
    vt.vt[i] = root[i].asInt();
  }

  return vt;
}
