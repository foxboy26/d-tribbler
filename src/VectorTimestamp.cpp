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

//TODO: update
void VectorTimestamp::Update(const VectorTimestamp& vt)
{
  int size = vt.vt.size();
  for (int i = 0; i < size; i++)
    this->vt[i] = max(this->vt[i], vt.vt[i]);
}

//TODO: serialize
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

  VectorTimestamp vt(1, root.size());
  for (int i = 0; i < root.size(); i++)
  {
    vt.vt[i] = root[i].asInt();
  }

  return vt;
}

int main()
{
  VectorTimestamp vt(1, 10);
  cout << vt << endl;
  vt.Inc();
  cout << vt << endl;
  
  VectorTimestamp vt2(3, 10);
  cout << vt2 << endl;
  vt2.Inc();
  cout << vt2 << endl;

  cout << (vt < vt2) << endl;

  VectorTimestamp vt3 = ToVectorTimestamp(vt.ToJSON());
  cout << vt3 << endl;

  return 0;
}
