#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

#include <graph_common.hpp>
#include <document_graph/content.hpp>
#include <document_graph/document.hpp>
#include <document_graph/edge.hpp>
#include <document_graph/util.hpp>
#include <document_graph/content_wrapper.hpp>
#include <document_graph/document_graph.hpp>

using namespace eosio;

CONTRACT document_editor : public contract {
  public:

    using contract::contract;

    document_editor(name self, name code, datastream<const char *> ds);
      ~document_editor();
    
    DECLARE_DOCUMENT_GRAPH(document_editor)

    ACTION reset ();
    ACTION createdoc (const name &creator);
    ACTION editdoc (const uint64_t &documentID, hashed::ContentGroups &content_groups);
    ACTION extenddoc (const name &creator, const uint64_t &fromNode);
    ACTION deletedoc (const uint64_t &documentID);
    ACTION createedge (const name &creator, const uint64_t &fromNode, const uint64_t &toNode, const name &edgeName);
    ACTION deleteedge (const uint64_t &fromNode, const uint64_t &toNode, const name &edgeName);

    // ACTION hi( name nm );



    // using hi_action = action_wrapper<"hi"_n, &document_editor::hi>;


    private:

    hashed::DocumentGraph m_documentGraph = hashed::DocumentGraph(get_self());

    hashed::Document get_node (const uint64_t &documentID);

};

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  switch (action) {
    EOSIO_DISPATCH_HELPER(document_editor, (reset)
      (createdoc)(editdoc)(extenddoc)(deletedoc)(createedge)(deleteedge)
    )
  }
}