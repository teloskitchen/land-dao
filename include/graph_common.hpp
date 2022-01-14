#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;

namespace graph {

  // document type
  static constexpr name DOCUMENT = name("document");

  // graph edges
  static constexpr name VARIABLE = name("variable");
  
  static constexpr name OWNED_BY = name("ownedby");
  static constexpr name OWNS = name("owns");

  static constexpr name FORKED = name("forked");

  #define NOT_FOUND -1

  #define CREATOR "creator"
  #define EXTENDED_OF "extended_of"
  #define FIXED_DETAILS "fixed_details"
  #define IDENTIFIER_DETAILS "identifier_details"
  #define NODE_ID "node_id"
  #define OWNER "owner"
  #define TITLE "title"
  #define TYPE "type"
  #define VARIABLE_DETAILS "variable_details"
}
