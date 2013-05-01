These files serve as the official distribution of source code,
documentation, executables, and compiled libraries for Lab 3.

The following files are included in the distribution

  Tribble.thrift               RPC service interface between your Tribble server and the Tribble client.
  KeyValueStore.thrift         RPC service interface between the Key Value Store and your TribbleServer.

  include:                     RPC code for message passing (Do not modify)
    KeyValueStore.*
    KeyValueStore_constants.*
    KeyValueStore_types.*
    Tribbler.*
    Tribbler_constants.*
    Tribbler_types.*

  src:
    Tribble_server.cpp       Skeleton code with empty functions that drive your Tribbler server.
    Tribble_client.cpp       Sample client program to post/view/ tribbles, add/remove subsribers.
                             This is for your testing purposes only. You can modify this file to add
                             more tests.
    KeyValue_server.cpp      Skeleton code with empty functions that drive the Key Value storage server.
    Makefile

Running the Key Value server:
     cd src/
     make clean
     make
     ./kv_server <id> <port> <peer1> <peerport1> ....

Running the Tribble server:
     ./tribble_server  <kvServerIP> <kvServerPort> <tribblerServerPort>

Running the Tribble client (c++):
    ./tribble_client <tribbleServerIP> <tribbleServerPort>
