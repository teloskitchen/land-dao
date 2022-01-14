#include <document_editor.hpp>

#include "document_graph/content.cpp"
#include "document_graph/document.cpp"
#include "document_graph/edge.cpp"
#include "document_graph/util.cpp"
#include "document_graph/content_wrapper.cpp"
#include "document_graph/document_graph.cpp"

// document_editor::~document_editor() {}

document_editor::document_editor(name self, name code, datastream<const char *> ds) : contract(self, code, ds) {}
document_editor::~document_editor() {}

ACTION document_editor::reset () {
  require_auth(get_self()); 

  document_table d_t(_self, _self.value);
  auto ditr = d_t.begin();
  while (ditr != d_t.end()) {
    ditr = d_t.erase(ditr);
  }

  edge_table e_t(_self, get_self().value);
  auto eitr = e_t.begin();
  while (eitr != e_t.end()) {
    eitr = e_t.erase(eitr);
  }

}


ACTION document_editor::createdoc (const name &creator) {

  require_auth( has_auth(creator) ? creator : get_self() );

  // creates the node
  hashed::ContentGroup document_node {
    hashed::ContentGroup {
      hashed::Content(hashed::CONTENT_GROUP_LABEL, FIXED_DETAILS),
      hashed::Content(TYPE, graph::DOCUMENT),
      hashed::Content(OWNER, creator)
    }
  };

  hashed::Document document(get_self(), creator, std::move(document_node));
  
}

ACTION document_editor::editdoc (const uint64_t &documentID, hashed::ContentGroups &content_groups) {

  hashed::Document document = get_node(documentID);

  hashed::ContentWrapper document_content = document.getContentWrapper();

  name owner = document_content.getOrFail(FIXED_DETAILS, OWNER) -> getAs<name>();
  
  require_auth( has_auth(owner) ? owner : get_self() );

  hashed::ContentGroups & doc_cg = document.getContentGroups();

  doc_cg = content_groups;

  document.modify();

}

ACTION document_editor::extenddoc (const name &creator, const uint64_t &fromNode) { // (creates a new document and a new edge)
  
  require_auth( has_auth(creator) ? creator : get_self() );

  hashed::Document document = get_node(fromNode);

  hashed::ContentWrapper document_content = document.getContentWrapper();

  name owner = document_content.getOrFail(FIXED_DETAILS, OWNER) -> getAs<name>();

  hashed::ContentGroup document_node {
    hashed::ContentGroup {
      hashed::Content(hashed::CONTENT_GROUP_LABEL, FIXED_DETAILS),
      hashed::Content(TYPE, graph::DOCUMENT),
      hashed::Content(OWNER, creator),
      hashed::Content(EXTENDED_OF, owner),
    }
  };

  hashed::Document deltas(get_self(), creator, std::move(document_node));

  deltas.merge(document, deltas);

  m_documentGraph.replaceNode(fromNode, deltas.getId());

  hashed::Edge::write(get_self(), creator, fromNode, deltas.getId(), graph::FORKED);


}
 
ACTION document_editor::deletedoc (const uint64_t &documentID) {

  
  document_table d_t(_self, _self.value);
  
  auto ditr = d_t.find(documentID);
  check(ditr != d_t.end(), "Document not found");

  hashed::Document document = get_node(documentID);

  hashed::ContentWrapper document_content = document.getContentWrapper();

  name creator = document_content.getOrFail(FIXED_DETAILS, OWNER) -> getAs<name>();
  
  require_auth( has_auth(creator) ? creator : get_self() );

  m_documentGraph.removeEdges(documentID);
  d_t.erase(ditr);


}

ACTION document_editor::createedge (const name &creator, const uint64_t &fromNode, const uint64_t &toNode, const name &edgeName) {
  

  document_table d_t(_self, _self.value);
  
  auto dfitr = d_t.find(fromNode);
  check(dfitr != d_t.end(), "First document not found");

  auto dtitr = d_t.find(toNode);
  check(dtitr != d_t.end(), "Second document not found");

  require_auth( creator);

  hashed::Edge::write(get_self(), creator, fromNode, toNode, edgeName);

}

ACTION document_editor::deleteedge (const uint64_t &fromNode, const uint64_t &toNode, const name &edgeName) {

  edge_table e_t(_self, _self.value);
  
  auto eitr = e_t.find( hashed::concatID(fromNode, toNode, edgeName) );
  check(eitr != e_t.end(), "Edge not found");

  require_auth( has_auth(eitr->creator) ? eitr->creator : get_self() );

  e_t.erase(eitr);

}


hashed::Document document_editor::get_node (const uint64_t &documentID) {
  document_table d_t(get_self(), get_self().value);

  auto ditr = d_t.find(documentID);
  check(ditr != d_t.end(), "Document not found");

  hashed::Document document(get_self(), ditr -> getHash());
  return document;
}

