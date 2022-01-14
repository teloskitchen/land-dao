#include <document_graph/document_graph.hpp>
#include <document_graph/document.hpp>
#include <document_graph/util.hpp>
#include <logger/logger.hpp>

namespace hashed
{
    std::vector<Edge> DocumentGraph::getEdges(const uint64_t &fromNode, const uint64_t &toNode)
    {
        std::vector<Edge> edges;

        // this index uniquely identifies all edges that share this fromNode and toNode
        uint64_t index = concatID(fromNode, toNode);
        Edge::edge_table e_t(m_contract, m_contract.value);
        auto from_name_index = e_t.get_index<eosio::name("byfromto")>();
        auto itr = from_name_index.find(index);

        while (itr != from_name_index.end() && itr->by_from_node_to_node_index() == index)
        {
            edges.push_back(*itr);
            itr++;
        }

        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesOrFail(const uint64_t &fromNode, const uint64_t &toNode)
    {
        std::vector<Edge> edges = getEdges(fromNode, toNode);
        EOS_CHECK(edges.size() > 0, "no edges exist: from " + std::to_string(fromNode) + " to " + std::to_string(toNode));
        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesFrom(const uint64_t &fromNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges;

        // this index uniquely identifies all edges that share this fromNode and edgeName
        uint64_t index = concatID(fromNode, edgeName);
        Edge::edge_table e_t(m_contract, m_contract.value);
        auto from_name_index = e_t.get_index<eosio::name("byfromname")>();
        auto itr = from_name_index.find(index);

        while (itr != from_name_index.end() && itr->by_from_node_edge_name_index() == index)
        {
            edges.push_back(*itr);
            itr++;
        }

        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesFromOrFail(const uint64_t &fromNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges = getEdgesFrom(fromNode, edgeName);
        EOS_CHECK(edges.size() > 0, "no edges exist: from " + std::to_string(fromNode) + " with name " + edgeName.to_string());
        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesTo(const uint64_t &toNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges;

        // this index uniquely identifies all edges that share this toNode and edgeName
        uint64_t index = concatID(toNode, edgeName);
        Edge::edge_table e_t(m_contract, m_contract.value);
        auto from_name_index = e_t.get_index<eosio::name("bytoname")>();
        auto itr = from_name_index.find(index);

        while (itr != from_name_index.end() && itr->by_to_node_edge_name_index() == index)
        {
            edges.push_back(*itr);
            itr++;
        }

        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesToOrFail(const uint64_t &toNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges = getEdgesTo(toNode, edgeName);
        EOS_CHECK(edges.size() > 0, "no edges exist: to " + std::to_string(toNode) + " with name " + edgeName.to_string());
        return edges;
    }

    // since we are removing multiple edges here, we do not call erase on each edge, which
    // would instantiate the table on each call.  This is faster execution.
    void DocumentGraph::removeEdges(const uint64_t &node)
    {
        Edge::edge_table e_t(m_contract, m_contract.value);

        auto from_node_index = e_t.get_index<eosio::name("fromnode")>();
        auto from_itr = from_node_index.find(node);

        while (from_itr != from_node_index.end() && from_itr->from_node == node)
        {
            from_itr = from_node_index.erase(from_itr);
        }

        auto to_node_index = e_t.get_index<eosio::name("tonode")>();
        auto to_itr = to_node_index.find(node);

        while (to_itr != to_node_index.end() && to_itr->to_node == node)
        {
            to_itr = to_node_index.erase(to_itr);
        }
    }

    bool DocumentGraph::hasEdges(const uint64_t &node)
    {
        Edge::edge_table e_t(m_contract, m_contract.value);
        
        auto from_node_index = e_t.get_index<eosio::name("fromnode")>();
        if (from_node_index.find(node) != from_node_index.end()) 
        {
            return true;
        }

        auto to_node_index = e_t.get_index<eosio::name("tonode")>();
        if (to_node_index.find(node) != to_node_index.end())
        {
            return true;
        }

        return false;
    }

    void DocumentGraph::replaceNode(const uint64_t &oldNode, const uint64_t &newNode)
    {
        Edge::edge_table e_t(m_contract, m_contract.value);

        auto from_node_index = e_t.get_index<eosio::name("fromnode")>();
        auto from_itr = from_node_index.find(oldNode);

        while (from_itr != from_node_index.end() && from_itr->from_node == oldNode)
        {
            // create the new edge record
            Edge newEdge(m_contract, m_contract, newNode, from_itr->to_node, from_itr->edge_name);

            // erase the old edge record
            from_itr = from_node_index.erase(from_itr);
        }

        auto to_node_index = e_t.get_index<eosio::name("tonode")>();
        auto to_itr = to_node_index.find(oldNode);

        while (to_itr != to_node_index.end() && to_itr->to_node == oldNode)
        {
            // create the new edge record
            Edge newEdge(m_contract, m_contract, to_itr->from_node, newNode, to_itr->edge_name);

            // erase the old edge record
            to_itr = to_node_index.erase(to_itr);
        }
    }

    // for now, permissions should be handled in the contract action rather than this class
    void DocumentGraph::eraseDocument(const eosio::checksum256 &documentHash, const bool includeEdges)
    {
        Document::document_table d_t(m_contract, m_contract.value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(documentHash);

        EOS_CHECK(h_itr != hash_index.end(), "Cannot erase document; does not exist: " + readableHash(documentHash));

        if (includeEdges)
        {
            removeEdges(h_itr->primary_key());
        }

        hash_index.erase(h_itr);
    }

    void DocumentGraph::eraseDocument(const eosio::checksum256 &documentHash)
    {
        TRACE_FUNCTION()
        return eraseDocument(documentHash, true);
    }
} // namespace hashed