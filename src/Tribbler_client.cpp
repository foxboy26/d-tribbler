// Sample client

#include "Tribbler.h"
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <iostream>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace Tribbler;
using namespace std;

void printSubscriptions(SubscriptionResponse& sr, string user) {
    unsigned int size = sr.subscriptions.size();
    if (sr.status != TribbleStatus::OK) {
       cout << "GetSubscriptions for user: "<< user << " failed. "
            << "Error code: " << sr.status << endl;
        return;
    }
    cout << user << ": has " << size << " subscribers " << endl;
    for (unsigned int i=0; i < size; i++) {
        cout << user << " is subscribed to " << sr.subscriptions[i] << endl;
    }
}


int main(int argc, char **argv) {
  if (argc != 3) {
      cerr << "Usage: " << argv[0] << " <tribblerServerIP> <tribblerServerPort>" << endl;
      exit(0);
  }
  string tribblerServer = string(argv[1]);
  int port = atoi(argv[2]);
  boost::shared_ptr<TSocket> socket(new TSocket(tribblerServer, port));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  TribblerClient client(protocol);
  try {
    transport->open();
    TribbleStatus::type ts_create;
    ts_create = client.CreateUser("alice");
    if (ts_create != TribbleStatus::OK) {
        cout << "Failed to create user: alice. Error code: " << ts_create << endl;
    }
    ts_create = client.CreateUser("bob");
    if (ts_create != TribbleStatus::OK) {
        cout << "Failed to create user: bob. Error code: " << ts_create << endl;
    }
    ts_create = client.CreateUser("cindy");
    if (ts_create != TribbleStatus::OK) {
        cout << "Failed to create user: cindy. Error code: " << ts_create << endl;
    }

    TribbleStatus::type ts_add;
    ts_add = client.AddSubscription("alice", "bob");
    if (ts_add != TribbleStatus::OK) {
        cout << "alice failed to subscribe to bob. "
             << "Error code: " << ts_add << endl;
    }
    ts_add = client.AddSubscription("alice", "cindy");
    if (ts_add != TribbleStatus::OK) {
        cout << "alice failed to subscribe to cindy. "
             << "Error code: " << ts_add << endl;
    }
    ts_add = client.AddSubscription("bob", "cindy");
    if (ts_add != TribbleStatus::OK) {
        cout << "bob failed to subscribe to cindy. "
             << "Error code: " << ts_add << endl;
    }
    ts_add = client.AddSubscription("cindy", "alice");
    if (ts_add != TribbleStatus::OK) {
        cout << "cindy failed to subscribe to alice. "
             << "Error code: " << ts_add << endl;
    }

    SubscriptionResponse sr_alice;
    SubscriptionResponse sr_bob;
    SubscriptionResponse sr_cindy;

    client.GetSubscriptions(sr_alice, "alice");
    client.GetSubscriptions(sr_bob, "bob");
    client.GetSubscriptions(sr_cindy, "cindy");

    printSubscriptions(sr_cindy, string("cindy"));
    printSubscriptions(sr_alice, string("alice"));
    printSubscriptions(sr_bob, string("bob"));

    TribbleStatus::type ts_remove;
    ts_remove = client.RemoveSubscription("alice", "cindy");
    if (ts_remove != TribbleStatus::OK) {
        cout << "alice failed to remove subscription to cindy. Error code: "
            << ts_add << endl;
    }

    client.GetSubscriptions(sr_alice, "alice");
    printSubscriptions(sr_bob, string("bob"));

    TribbleResponse tr_alice;
    TribbleStatus::type ts_post;
    ts_post = client.PostTribble("alice", "What a beautiful day!");
    if (ts_post != TribbleStatus::OK) {
        cout << "Alice could not post "
             << "Error code: " << ts_post << endl;
    }


    client.GetTribbles(tr_alice, "alice");
    if (tr_alice.status != TribbleStatus::OK) {
        cout << "Alice failed to get tribbles "
             << "Error code: " << tr_alice.status << endl;
    }

    TribbleResponse tr_bob;
    client.GetTribblesBySubscription(tr_bob, "bob");
    if (tr_bob.status != TribbleStatus::OK) {
        cout << "Bob failed to get tribbles by subscription. "
             << "Error code: " << tr_bob.status << endl;
    }

    transport->close();
  } catch (TException &tx) {
    cout << "ERROR: %s" <<  tx.what() << endl;
  }

  return 0;
}
