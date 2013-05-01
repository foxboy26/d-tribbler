// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "KeyValueStore.h"
#include <transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include <sstream>
#include <json/json.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;
using namespace std;

using namespace  ::KeyValueStore;

struct KVServerStatus
{
  enum type
  {
    NOTJOINED,
    STARTING,
    RUNNING,
    DIED
  };
};

class Value
{
  public:
    string value;
    int64_t timestamp;

    Value(string value, int64_t timestamp)
    {
      this->value = value;
      this->timestamp = timestamp;
    }

    string ToJSON()
    {
      stringstream ss;
      ss << "{\"value\": " << value << ", \"timestamp\": " << timestamp << "}";
      return ss.str();
    }
};

class KeyValueStoreHandler : virtual public KeyValueStoreIf {
 public:
  KeyValueStoreHandler(int argc, char** argv) {
    // Your initialization goes here
    _id = string(argv[1]);

    for(int i = 3; i+1 < argc; i += 2) {
      string peer_ip(argv[i]);
      int peer_port = atoi(argv[i+1]);
      _backendServerVector.push_back(make_pair(peer_ip, peer_port));
      cout << "Backend server at: " << peer_ip << " on port: " << peer_port << endl;
    }

    cout << "Starting server " << argv[1] << "...";
    KVStoreStatus::type status;
    status = _Put("syscmd_server_status", "starting");
    _status = KVServerStatus::STARTING;
    //step1: find a running server;
    int target = findRunningServer();

    //step2: get data from running server if necessary
    if (target != -1)
    {
      status = GetDataFromServer(target);
      //if (status != KVStoreStatus::OK)
      //  return status;

      //step3: tell other server I am ready.
      status = joinCluster();
      //if (status != KVStoreStatus::OK)
      //  return status;
    }


    status = _Put("syscmd_server_status", "running");
    _status = KVServerStatus::RUNNING;

    cout << "success!" << endl;
  }

  int findRunningServer()
  {
    GetResponse rp;
    int target = -1;

    int size = _backendServerVector.size();
    for (int i = 0; i < size; i++)
    {
      rp = RPC_Get(i, "syscmd_server_status");
      if (rp.status == KVStoreStatus::OK)
      {
        if (rp.value == "running")
        {
          _runningServer.insert(i);
          if (target == -1)
            target = i;
        }
      }
    }

    return target;
  }

  KVStoreStatus::type joinCluster()
  {
    KVStoreStatus::type status;
    for(set<int>::iterator it = _runningServer.begin(); it != _runningServer.end(); it++)
    {
      status = RPC_Put(*it, "syscmd", "join cluster");
      if (status != KVStoreStatus::OK)
        return status;
    }
    return KVStoreStatus::OK;
  }

  KVStoreStatus::type GetDataFromServer(const int server)
  {
    GetListResponse lrp;
    lrp = RPC_GetList(server, "user_list");
    if (lrp.status != KVStoreStatus::OK)
      return lrp.status;

    KVStoreStatus::type status;
    int size = lrp.values.size();
    for (int i = 0; i < size; i++)
    {
      status = _AddToList("user_list", lrp.values[i]);
      if (status != KVStoreStatus::OK)
        return status;

      status = _Put(lrp.values[i], lrp.values[i]);
      if (status != KVStoreStatus::OK)
        return status;
    }

    return KVStoreStatus::OK;
  }

  void Get(GetResponse& _return, const std::string& key) {
    // Your implementation goes here
    if (acceptRequest())
    {
      printf("Get\n");

      _Get(_return, key);
      //TODO: check timestamp
    }
    else
    {
      printf("Request rejected\n");

      _return.status = KVStoreStatus::INTERNAL_FAILURE;
    }
  }

  void GetList(GetListResponse& _return, const std::string& key) {
    // Your implementation goes here
    if (acceptRequest())
    {
      printf("GetList\n");

      _GetList(_return, key);
    }
    else
    {
      printf("Request rejected\n");

      _return.status = KVStoreStatus::INTERNAL_FAILURE;
    }
  }

  KVStoreStatus::type Put(const std::string& key, const std::string& value, const std::string& clientid) {
    // Your implementation goes here
    if (acceptRequest())
    {
      printf("Put\n");

      //TODO: forward request from tribble server
      if (clientid == "tribbleserver")
      {
        KVStoreStatus::type status;
        for (set<int>::iterator it = _runningServer.begin(); it != _runningServer.end(); it++)
        {
          status = RPC_Put(*it, key, value);
          if (status == KVStoreStatus::INTERNAL_FAILURE)
            continue;
          else if (status != KVStoreStatus::OK)
            return status;
        }
        return _Put(key, value);
      }
      else //TODO: process request from other KV server
      {
        if (key.find("syscmd_"))
        {
          //process system commad

          return KVStoreStatus::OK;
        }
        else
         return _Put(key, value);
      }
    }
    else
    {
      printf("Request rejected\n");

      return KVStoreStatus::INTERNAL_FAILURE;
    }
  }

  KVStoreStatus::type AddToList(const std::string& key, const std::string& value, const std::string& clientid) {
    // Your implementation goes here
    if (acceptRequest())
    {
      printf("AddToList\n");

      //TODO: forward request from tribble server
      if (clientid == "tribbleserver")
      {
        KVStoreStatus::type status;
        for (set<int>::iterator it = _runningServer.begin(); it != _runningServer.end(); it++)
        {
          status = RPC_RemoveFromList(*it, key, value);
          if (status == KVStoreStatus::INTERNAL_FAILURE)
            continue;
          else if (status != KVStoreStatus::OK)
            return status;
        }
        return _AddToList(key, value);
      }
      else
      {
        //TODO: process request from other KV server
        return _AddToList(key, value);
      }
    }
    else
      return KVStoreStatus::INTERNAL_FAILURE;

  }

  KVStoreStatus::type RemoveFromList(const std::string& key, const std::string& value, const std::string& clientid) {
    // Your implementation goes here
    if (acceptRequest())
    {
      printf("RemoveFromList\n");
      //TODO: forward request from tribble server
      if (clientid == "tribbleserver")
      {
        KVStoreStatus::type status;
        for (set<int>::iterator it = _runningServer.begin(); it != _runningServer.end(); it++)
        {
          status = RPC_RemoveFromList(*it, key, value);
          if (status == KVStoreStatus::INTERNAL_FAILURE)
            continue;
          else if (status != KVStoreStatus::OK)
            return status;
        }
        return _RemoveFromList(key, value);
      }
      else
      {
        //TODO: process request from other KV server
        return _RemoveFromList(key, value);
      }
    }
    else
      return KVStoreStatus::INTERNAL_FAILURE;
  }

  KeyValueStore::GetResponse RPC_Get(const int index, const std::string key)
  {
    KeyValueStore::GetResponse response;
    // Making the RPC Call to the kv server
    try
    {
      boost::shared_ptr<TSocket> socket(new TSocket(_backendServerVector[index].first, _backendServerVector[index].second));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      KeyValueStoreClient client(protocol);
      transport->open();
      client.Get(response, key);
      transport->close();
    }
    catch (TException &te)
    {
      _runningServer.erase(index);
      response.status = KVStoreStatus::INTERNAL_FAILURE;
    }

    return response;
  }

  KeyValueStore::GetListResponse RPC_GetList(const int index, const std::string key)
  {
    KeyValueStore::GetListResponse response;
    // Making the RPC Call to the Storage server
    try
    {
      boost::shared_ptr<TSocket> socket(new TSocket(_backendServerVector[index].first, _backendServerVector[index].second));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      KeyValueStoreClient client(protocol);
      transport->open();
      client.GetList(response, key);
      transport->close();
    }
    catch (TException &te)
    {
      _runningServer.erase(index);
      response.status = KVStoreStatus::INTERNAL_FAILURE;
    }

    return response;
  }

  KVStoreStatus::type RPC_Put(const int index, const std::string& key, const std::string& value)
  {
    try
    {
      boost::shared_ptr<TSocket> socket(new TSocket(_backendServerVector[index].first, _backendServerVector[index].second));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      KeyValueStoreClient client(protocol);
      KVStoreStatus::type st;
      transport->open();
      st = client.Put(key, value, this->_id);
      transport->close();
      return st;
    }
    catch (TException &tx)
    {
      _runningServer.erase(index);
      return KVStoreStatus::INTERNAL_FAILURE;
    }
  }

  KVStoreStatus::type RPC_AddToList(const int index, const std::string& key, const std::string& value)
  {
    try
    {
      boost::shared_ptr<TSocket> socket(new TSocket(_backendServerVector[index].first, _backendServerVector[index].second));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      KeyValueStoreClient client(protocol);
      KVStoreStatus::type st;
      transport->open();
      st = client.AddToList(key, value, this->_id);
      transport->close();
      return st;
    }
    catch (TException &tx)
    {
      _runningServer.erase(index);
      return KVStoreStatus::INTERNAL_FAILURE;
    }
  }

  KVStoreStatus::type RPC_RemoveFromList(const int index, const std::string& key, const std::string& value)
  {
    try
    {
      boost::shared_ptr<TSocket> socket(new TSocket(_backendServerVector[index].first, _backendServerVector[index].second));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      KeyValueStoreClient client(protocol);
      KVStoreStatus::type st;
      transport->open();
      st = client.RemoveFromList(key, value, this->_id);
      transport->close();
      return st;
    }
    catch (TException &tx)
    {
      _runningServer.erase(index);
      return KVStoreStatus::INTERNAL_FAILURE;
    }
  }
  
  void _Get(GetResponse& _return, const std::string& key)
  {
    std::map<std::string, std::string>::iterator it;
    it = _simpleStorage.find(key);
    if (it == _simpleStorage.end())
      _return.status = KVStoreStatus::EKEYNOTFOUND;
    else
    {
      _return.value = it->second;
      _return.status = KVStoreStatus::OK;
    }
  }

  void _GetList(GetListResponse& _return, const std::string& key)
  {
    std::map<std::string, std::set<std::string> >::iterator it;
    it = _listStorage.find(key);
    if (it == _listStorage.end())
      _return.status = KVStoreStatus::EKEYNOTFOUND;

    for (std::set<std::string>::iterator s_it = it->second.begin(); s_it != it->second.end(); s_it++)
    {
      _return.values.push_back(*s_it);
    }

    _return.status =  KVStoreStatus::OK;
  }


  KVStoreStatus::type _Put(const std::string& key, const std::string& value)
  {
    std::pair<std::map<std::string,std::string>::iterator, bool> ret;
    ret = _simpleStorage.insert(std::pair<string, string>(key, value));
    
    if (ret.second == false)
      return KVStoreStatus::EPUTFAILED;
    else
      return KVStoreStatus::OK;
  }

  KVStoreStatus::type _AddToList(const std::string& key, const std::string& value)
  {
    std::map<std::string, std::set<std::string> >::iterator it;
    it = _listStorage.find(key);
    if (it == _listStorage.end())
    {
      set<string> valueSet;
      valueSet.insert(value);
      _listStorage.insert(pair<string, set<string> >(key, valueSet));
      return KVStoreStatus::OK;
    }
    else
    {
      std::pair<std::set<std::string>::iterator, bool> ret;
      ret = it->second.insert(value);
      if (ret.second == false)
        return KVStoreStatus::EITEMEXISTS;
      else
        return KVStoreStatus::OK;
    }
  }

  KVStoreStatus::type _RemoveFromList(const std::string& key, const std::string& value)
  {
    std::map<std::string, std::set<std::string> >::iterator it;
    it = _listStorage.find(key);
    if (it == _listStorage.end())
      return KVStoreStatus::EKEYNOTFOUND;
    
    int count = it->second.erase(value);
    if (count == 0)
      return KVStoreStatus::EITEMNOTFOUND;
    else
      return KVStoreStatus::OK;
  }

  private:
    string _id;
    KVServerStatus::type _status;
    set<int> _runningServer;
    vector < pair<string, int> > _backendServerVector;

    map<string, string> _simpleStorage;
    map<string, set<string> > _listStorage;

    bool acceptRequest()
    {
      return _status == KVServerStatus::RUNNING;
    }
};

int main(int argc, char **argv) {
  if((argc < 3) || !(argc % 2)) {
      cerr << "Usage: " << argv[0] << " id localport peer1 port1..." << endl;
      exit(1);
  }
  int port = atoi(argv[2]);
  shared_ptr<KeyValueStoreHandler> handler(new KeyValueStoreHandler(argc, argv));
  shared_ptr<TProcessor> processor(new KeyValueStoreProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}