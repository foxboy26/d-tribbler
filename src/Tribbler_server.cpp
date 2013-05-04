// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <queue>
#include <time.h>
#include <cstdlib>

#include "Tribbler.h"
#include "KeyValueStore.h"
#include "Message.h"
#include <transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace std;
using namespace  ::Tribbler;
using namespace  ::KeyValueStore;

bool Tribble::operator < (const Tribble & t) const
{
  return this->posted < t.posted;
}

class TribblerHandler : virtual public TribblerIf {
 public:

  TribblerHandler(std::string kvServer, int kvServerPort) {
    // Your initialization goes here
    _kvServer = kvServer;
    _kvServerPort = kvServerPort;
  }

  TribbleStatus::type CreateUser(const std::string& userid) {
    // Your implementation goes here
    printf("CreateUser\n");

    //const string _userid = "u_" + userid;

    GetResponse response;
    response = Get(userid);
    if (response.status != KVStoreStatus::EKEYNOTFOUND)
      return TribbleStatus::EEXISTS;

    KVStoreStatus::type status;
    status = AddToList("user_list", userid);
    status = Put(userid, userid);
    if (status == KVStoreStatus::EPUTFAILED)
      return TribbleStatus::STORE_FAILED;

    return TribbleStatus::OK;
  }

  TribbleStatus::type AddSubscription(const std::string& userid, const std::string& subscribeto) {
    // Your implementation goes here
    printf("AddSubscription\n");

    //const string _userid = "u_" + userid;
    if (!CheckUser(userid))
      return TribbleStatus::INVALID_USER;

    //const string _subscribeto = "u_" + userid;
    if (!CheckUser(subscribeto) || userid == subscribeto)
      return TribbleStatus::INVALID_SUBSCRIBETO;

    KVStoreStatus::type status = AddToList(userid + "_sub", subscribeto);

    if (status == KVStoreStatus::EITEMEXISTS)
      return TribbleStatus::EEXISTS;
    if (status == KVStoreStatus::EPUTFAILED)
      return TribbleStatus::STORE_FAILED;

    return TribbleStatus::OK;
  }

  TribbleStatus::type RemoveSubscription(const std::string& userid, const std::string& subscribeto) {
    // Your implementation goes here
    printf("RemoveSubscription\n");

    //const string _userid = "u_" + userid;
    if (!CheckUser(userid))
      return TribbleStatus::INVALID_USER;

    //const string _subscribeto = "u_" + userid;
    if (!CheckUser(subscribeto))
      return TribbleStatus::INVALID_SUBSCRIBETO;
    
    KVStoreStatus::type status = RemoveFromList(userid + "_sub", subscribeto);
    if (status != KVStoreStatus::OK)
      return TribbleStatus::STORE_FAILED;

    return TribbleStatus::OK;
  }

  TribbleStatus::type PostTribble(const std::string& userid, const std::string& tribbleContents) {
    // Your implementation goes here
    printf("PostTribble\n");

    //const string _userid = "u_" + userid;
    if (!CheckUser(userid))
      return TribbleStatus::INVALID_USER;

    KVStoreStatus::type status;

    status = AddToList(userid + "_tribble", tribbleContents);
    if (status == KVStoreStatus::EITEMEXISTS)
      return TribbleStatus::EEXISTS;

    return TribbleStatus::OK;
  }

  void GetTribbles(TribbleResponse& _return, const std::string& userid) {
    // Your implementation goes here
    printf("GetTribbles\n");

    if (!CheckUser(userid))
      _return.status = TribbleStatus::INVALID_USER;
    else
    {
      GetListResponse listResponse = GetList(userid + "_tribble");
      if (listResponse.status == KVStoreStatus::OK)
      {
        Tribble t;
        Message m; 
        int num = min(maxTribbles, static_cast<int>(listResponse.values.size()));
        for (int i = 0; i < num; i++)
        {
          m = ToMessage(listResponse.values[i]);
          t.userid = userid;
          t.posted = m.vt.vt;
          t.contents = m.value;
          _return.tribbles.push_back(t);
        }

        /*priority_queue<string, vector<string>, greater<string> > pq;

        for (size_t i = 0; i < num; i++)
        {
          pq.push(listResponse.values[i]);
          if (i >= maxTribbles)
          {
            pq.pop();
          }
        }
       
        GetResponse response;
        Tribble t;
        while (!pq.empty())
        {
          string posted = pq.top();
          pq.pop();

          response = Get(userid + posted);
          if (response.status != KVStoreStatus::OK)
            continue;

          t.userid = userid;
          //t.posted = atol(posted.c_str());
          t.posted.push_back(atol(posted.c_str()));
          t.contents = response.value;
          _return.tribbles.push_back(t);
        }*/
        reverse(_return.tribbles.begin(), _return.tribbles.end());

        _return.status = TribbleStatus::OK;
      }
      else if (listResponse.status == KVStoreStatus::EKEYNOTFOUND)
        _return.status = TribbleStatus::OK;
      else
        _return.status = TribbleStatus::INTERNAL_FAILURE;
    }
  }

  void GetTribblesBySubscription(TribbleResponse& _return, const std::string& userid) {
    // Your implementation goes here
    printf("GetTribblesBySubscription\n");

    if (!CheckUser(userid))
      _return.status = TribbleStatus::INVALID_USER;
    else
    {
      SubscriptionResponse subResponse;
      GetSubscriptions(subResponse, userid);
      if (subResponse.status == TribbleStatus::OK)
      {
        priority_queue<Tribble> pq;

        Tribble t;
        Message m;
        int num = subResponse.subscriptions.size();
        GetListResponse* listResponse = new GetListResponse[num];
        map<string, pair<int, int> > idx_map;

        for (int i = 0; i < num; i++)
        {
          listResponse[i] = GetList(subResponse.subscriptions[i] + "_tribble");
        }
        
        for (int i = 0; i < num; i++)
        {
          if (listResponse[i].status == KVStoreStatus::OK)
          {
            m = ToMessage(listResponse[i].values[0]);
            t.userid = subResponse.subscriptions[i];
            t.posted = m.vt.vt;
            t.contents = m.value;
            pq.push(t);
            idx_map[subResponse.subscriptions[i]] = make_pair<int, int>(i, 1);
          }
        }

        for (int i = 0; i < maxTribbles && !pq.empty(); i++)
        {
          t = pq.top();
          pq.pop();

          map<string, pair<int, int> >::iterator it = idx_map.find(t.userid);
          m = ToMessage(listResponse[it->second.first].values[it->second.second]);
          t.userid = subResponse.subscriptions[i];
          t.posted = m.vt.vt;
          t.contents = m.value;
          pq.push(t);
          (it->second.second)++;

          _return.tribbles.push_back(t);
        }

        delete [] listResponse;

        /*for (int i = 0; i < num; i++)
        {
          listResponse = GetList(subResponse.subscriptions[i] + "_tribble");
          if (listResponse.status == KVStoreStatus::OK)
          {
            int len = listResponse.values.size();
            for (int j = 0; j < len; j++)
            {
              ut.userid = subResponse.subscriptions[i];
              ut.posted = listResponse.values[j];
              count++;
              pq.push(ut);
              if (count > maxvTribbles)
                pq.pop();
            }
          }
        }

        GetResponse response;
        Tribble t;
        while (!pq.empty())
        {
          ut = pq.top();
          pq.pop();

          response = Get(ut.userid + ut.posted);
          if (response.status != KVStoreStatus::OK)
            continue;

          t.userid = ut.userid;
          //t.posted = atol(ut.posted.c_str());
          t.posted.push_back(atol(ut.posted.c_str()));
          t.contents = response.value;
          _return.tribbles.push_back(t);
        }
        reverse(_return.tribbles.begin(), _return.tribbles.end());*/

        _return.status = TribbleStatus::OK;
      }
      else
        _return.status = TribbleStatus::INTERNAL_FAILURE;
    }
  }

  void GetSubscriptions(SubscriptionResponse& _return, const std::string& userid) {
    // Your implementation goes here
    printf("GetSubscriptions\n");

    if (!CheckUser(userid))
      _return.status = TribbleStatus::INVALID_USER;
    else
    {
      GetListResponse listResponse = GetList(userid + "_sub");
      if (listResponse.status == KVStoreStatus::OK)
      {
        int num = listResponse.values.size();
        GetResponse response;
        for (int i = 0; i < num; i++)
        {
           _return.subscriptions.push_back(listResponse.values[i]);
        }
        _return.status = TribbleStatus::OK;
      }
      else if (listResponse.status == KVStoreStatus::EKEYNOTFOUND)
        _return.status = TribbleStatus::OK;
      else
        _return.status = TribbleStatus::INTERNAL_FAILURE;
    }
  }

  // Functions from interacting with the kv RPC server
  KVStoreStatus::type AddToList(std::string key, std::string value) {
    boost::shared_ptr<TSocket> socket(new TSocket(_kvServer, _kvServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient kv_client(protocol);
    // Making the RPC Call
    KVStoreStatus::type st;
    transport->open();
    string clientid("tribbleserver");
    st = kv_client.AddToList(key, value, clientid);
    transport->close();
    return st;
  }

  KVStoreStatus::type RemoveFromList(std::string key, std::string value) {
    // Making the RPC Call to the kv server
    boost::shared_ptr<TSocket> socket(new TSocket(_kvServer, _kvServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    KVStoreStatus::type st;
    transport->open();
    string clientid("tribbleserver");
    st = client.RemoveFromList(key, value, clientid);
    transport->close();
    return st;
  }

  KVStoreStatus::type Put(std::string key, std::string value) {
    // Making the RPC Call to the kv server
    boost::shared_ptr<TSocket> socket(new TSocket(_kvServer, _kvServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    KVStoreStatus::type st;
    transport->open();
    string clientid("tribbleserver");
    st = client.Put(key, value, clientid);
    transport->close();
    return st;
  }

  KeyValueStore::GetResponse Get(std::string key) {
    KeyValueStore::GetResponse response;
    // Making the RPC Call to the kv server
    boost::shared_ptr<TSocket> socket(new TSocket(_kvServer, _kvServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    transport->open();
    client.Get(response, key);
    transport->close();
    return response;
  }

  KeyValueStore::GetListResponse GetList(std::string key) {
    KeyValueStore::GetListResponse response;
    // Making the RPC Call to the Storage server
    boost::shared_ptr<TSocket> socket(new TSocket(_kvServer, _kvServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    transport->open();
    client.GetList(response, key);
    transport->close();
    return response;
  }

 private:
  std::string _kvServer;
  int _kvServerPort;

  static int maxTribbles;

  bool CheckUser(const std::string &userid)
  {
    KeyValueStore::GetResponse response = Get(userid);

    if (response.status == KVStoreStatus::EKEYNOTFOUND)
      return false;
    else
      return true;
  }
};

int TribblerHandler::maxTribbles = 100;

int main(int argc, char **argv) {
  if (argc != 4) {
    cerr << "Usage: " << argv[0] << " <kvServerIP> <kvServerPort> <tribbleServerPort>" << endl;
    exit(0);
  }
  std::string kvServer = std::string(argv[1]);
  int kvServerPort = atoi(argv[2]);
  int tribblerPort = atoi(argv[3]);

  shared_ptr<TribblerHandler> handler(new TribblerHandler(kvServer, kvServerPort));
  shared_ptr<TProcessor> processor(new TribblerProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(tribblerPort));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  cout << "Starting Tribbler Server" << endl;
  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}
