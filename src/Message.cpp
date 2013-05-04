#include "Message.h"

#include <json/json.h>

#include <string>
#include <sstream>
using namespace std;

inline Message::Message(const std::string& value, const VectorTimestamp& vt)
{
  this->value = value;
  this->vt = vt;
}

string Message::ToJSON() const
{
  stringstream ss;
  ss << "{\"value\": \"" << value << "\", \"vt\": " << vt.ToJSON() << "}";
  return ss.str();
}

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
