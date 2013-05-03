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

  VectorTimestamp vt(-1, root.size());
  for (int i = 0; i < root.size(); i++)
  {
    vt.vt[i] = root[i].asInt();
  }

  return vt;
}

class Message
{
  public:
    string value;
    VectorTimestamp vt;

    Message() {}
    Message(const string& value, const VectorTimestamp& vt)
    {
      this->value = value;
      this->vt = vt;
    }

    string ToJSON() const
    {
      stringstream ss;
      ss << "{\"value\": \"" << value << "\", \"vt\": " << vt.ToJSON() << "}";
      return ss.str();
    }

    friend Message ToMessage(const string& json);
};

Message ToMessage(const string& json)
{
  Json::Value root;
  Json::Reader reader;
  bool isSuccess = reader.parse(json, root);
  if (!isSuccess)
  {
    cout << "error in json syntax" << endl;
  }

  Message msg;
  msg.value = root["value"].asString();
  int size = root["vt"].size();
  for (int i = 0; i < size; i++)
    msg.vt.vt.push_back(root["vt"][i].asInt());

  return msg;
}

ostream& operator<< (ostream& out, const Message& m) 
{
  out << m.ToJSON();

  return out;
}

int main()
{
  VectorTimestamp test;

  VectorTimestamp vt(1, 10);

  Message m("xxx", vt);

  Message m2 = ToMessage("{\"value\":\"lzh\", \"vt\":[232,13,2,3]}");

  cout << m2 << endl;

  return 0;
}
