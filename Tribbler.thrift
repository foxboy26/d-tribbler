# RPC Interface between Tribbler client and Tribbler server.
# Malveeka Tewari (malveeka@cs.ucsd.edu)

namespace cpp Tribbler

/**
 * Enum type for returning status of a RPC request made to the
 * the Tribbler server.
 */

enum TribbleStatus {
    OK = 1,
    EEXISTS = 2,
    INVALID_USER = 3,
    INVALID_SUBSCRIBETO = 4,
    NOT_IMPLEMENTED = 5,
    STORE_FAILED = 6,
    INTERNAL_FAILURE = 7
}

/**
 * Tribble data type used by the server to represent user posts.
 */

struct Tribble {
    1: string userid,
    2: list<i64> posted,
    3: string contents
}

/**
 * Return type for a RPC call to the Tribbler server request user's tribbles.
 */

struct TribbleResponse {
    1: list<Tribble> tribbles,
    2: TribbleStatus status
}

/**
 * Return type for a RPC call to the Tribbler server request user's list of
 * subscriptions.
 */

struct SubscriptionResponse {
    1: list<string> subscriptions,
    2: TribbleStatus status
}


/**
 * RPC services offered by the Tribbler server.
 */

service Tribbler {
  TribbleStatus CreateUser(1:string userid),
  TribbleStatus AddSubscription(1:string userid, 2:string subscribeto),
  TribbleStatus RemoveSubscription(1:string userid, 2:string subscribeto),
  TribbleStatus PostTribble(1:string userid, 2:string tribbleContents),
  TribbleResponse GetTribbles(1:string userid),
  TribbleResponse GetTribblesBySubscription(1:string userid),
  SubscriptionResponse GetSubscriptions(1: string userid)
}
