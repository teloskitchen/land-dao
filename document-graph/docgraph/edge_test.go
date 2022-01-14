package docgraph_test

import (
	"log"
	"testing"

	eostest "github.com/digital-scarcity/eos-go-test"
	eos "github.com/eoscanada/eos-go"
	"github.com/hashed-io/document-graph/docgraph"
	"gotest.tools/v3/assert"
)

func TestEdges(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	doc1, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[0], "examples/simplest.json")
	assert.NilError(t, err)

	doc2, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[0], "examples/each-type.json")
	assert.NilError(t, err)

	// doc3, err := CreateDocument(env.ctx, &env.api, env.Docs, suite.Accounts[3], "examples/contribution.json")
	// suite.Require().NoError(err)

	tests := []struct {
		name     string
		fromDoc  docgraph.Document
		toDoc    docgraph.Document
		creator  eos.AccountName
		edgeName eos.Name
	}{
		{
			name:     "Test Edge 1",
			fromDoc:  doc1,
			toDoc:    doc2,
			creator:  env.Creators[1],
			edgeName: "edge1",
		},
		{
			name:     "Test Edge 2",
			fromDoc:  doc2,
			toDoc:    doc1,
			creator:  env.Creators[2],
			edgeName: "edge2",
		},
	}

	for testIndex, test := range tests {
		t.Run("test edges", func(t *testing.T) {
			log.Println(test.name, "... ")

			_, err = docgraph.CreateEdge(env.ctx, &env.api, env.Docs, env.Creators[0], test.fromDoc.ID, test.toDoc.ID, test.edgeName)
			assert.NilError(t, err)

			// test number of edges
			edges, err := docgraph.GetAllEdges(env.ctx, &env.api, env.Docs)
			assert.NilError(t, err)
			assert.Equal(t, testIndex+1, len(edges))

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesFrom, err := docgraph.GetEdgesFromDocument(env.ctx, &env.api, env.Docs, test.fromDoc)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesFrom))
			assert.Equal(t, edgesFrom[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesFrom[0].FromNode, test.fromDoc.ID)
			assert.Equal(t, edgesFrom[0].ToNode, test.toDoc.ID)

			// there should be 0 edges from doc2 to doc1
			edgesTo, err := docgraph.GetEdgesToDocument(env.ctx, &env.api, env.Docs, test.toDoc)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesTo))
			assert.Equal(t, edgesTo[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesTo[0].FromNode, test.fromDoc.ID)
			assert.Equal(t, edgesTo[0].ToNode, test.toDoc.ID)

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesFromByName, err := docgraph.GetEdgesFromDocumentWithEdge(env.ctx, &env.api, env.Docs, test.fromDoc, test.edgeName)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesFromByName))
			assert.Equal(t, edgesFromByName[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesFromByName[0].FromNode, test.fromDoc.ID)
			assert.Equal(t, edgesFromByName[0].ToNode, test.toDoc.ID)

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesToByName, err := docgraph.GetEdgesToDocumentWithEdge(env.ctx, &env.api, env.Docs, test.toDoc, test.edgeName)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesToByName))
			assert.Equal(t, edgesToByName[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesToByName[0].FromNode, test.fromDoc.ID)
			assert.Equal(t, edgesToByName[0].ToNode, test.toDoc.ID)

			// there should be 0 edge from doc1 to doc2, named wrongedge
			edgesFromByName, err = docgraph.GetEdgesFromDocumentWithEdge(env.ctx, &env.api, env.Docs, test.fromDoc, eos.Name("wrongedge"))
			assert.NilError(t, err)
			assert.Equal(t, 0, len(edgesFromByName))

			// there should be 0 edge from doc1 to doc2, named edgeName
			edgesToByName, err = docgraph.GetEdgesToDocumentWithEdge(env.ctx, &env.api, env.Docs, test.toDoc, eos.Name("wrongedge"))
			assert.NilError(t, err)
			assert.Equal(t, 0, len(edgesToByName))

			doesExist, err := docgraph.EdgeExists(env.ctx, &env.api, env.Docs, test.fromDoc, test.toDoc, test.edgeName)
			assert.NilError(t, err)
			assert.Assert(t, doesExist)

			doesNotExist, err := docgraph.EdgeExists(env.ctx, &env.api, env.Docs, test.fromDoc, test.toDoc, eos.Name("doesnotexist"))
			assert.NilError(t, err)
			assert.Assert(t, !doesNotExist)
		})
	}
}

func TestRemoveEdges(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	// var docs []Document
	var err error
	docs := make([]docgraph.Document, 10)
	for i := 0; i < 10; i++ {
		docs[i], err = CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
		assert.NilError(t, err)
	}

	// ***************************  BEGIN
	// test removal of edges based on the from_node and edge_name
	for i := 0; i < 5; i++ {
		_, err = docgraph.CreateEdge(env.ctx, &env.api, env.Docs, env.Creators[1], docs[0].ID, docs[i].ID, "test")
		assert.NilError(t, err)
		eostest.Pause(chainResponsePause, "Build block...", "")
	}

	allEdges, err := docgraph.GetAllEdges(env.ctx, &env.api, env.Docs)
	assert.NilError(t, err)
	assert.Equal(t, len(allEdges), 5)

	for i := 0; i < 5; i++ {
		checkEdge(t, env, docs[0], docs[i], eos.Name("test"))
		_, err = docgraph.RemoveEdge(env.ctx, &env.api, env.Docs, docs[0].ID, docs[i].ID, eos.Name("test"))
		assert.NilError(t, err)
	}

	allEdges, err = docgraph.GetAllEdges(env.ctx, &env.api, env.Docs)
	assert.NilError(t, err)
	assert.Equal(t, len(allEdges), 0)
	// *****************************  END

	// // *****************************  BEGIN
	// // test removal of edges based on the from_node and to_node
	// for i := 0; i < 3; i++ {
	// 	_, err = docgraph.CreateEdge(env.ctx, &env.api, env.Docs, env.Creators[1], docs[0].Hash, docs[1].Hash, eos.Name("test"+strconv.Itoa(i+1)))
	// 	assert.NilError(t, err)
	// 	pause(t, chainResponsePause, "Build block...", "")
	// }

	// allEdges, err = GetAllEdges(env.ctx, &env.api, env.Docs)
	// assert.NilError(t, err)
	// assert.Equal(t, len(allEdges), 3)

	// for i := 0; i < 3; i++ {
	// 	checkEdge(t, env, docs[0], docs[1], eos.Name("test"+strconv.Itoa(i+1)))
	// }

	// // remove edges based on the from_node and edge_name
	// _, err = docgraph.RemoveEdgesFromAndTo(env.ctx, &env.api, env.Docs, docs[0].Hash, docs[1].Hash)
	// assert.NilError(t, err)

	// allEdges, err = GetAllEdges(env.ctx, &env.api, env.Docs)
	// assert.NilError(t, err)
	// assert.Equal(t, len(allEdges), 0)
	// // *****************************  END

	// // ***************************  BEGIN
	// // test removal of edges based on the testedge index action
	// for i := 0; i < 5; i++ {
	// 	_, err = docgraph.CreateEdge(env.ctx, &env.api, env.Docs, env.Creators[1], docs[0].Hash, docs[i].Hash, "test")
	// 	assert.NilError(t, err)
	// 	pause(t, chainResponsePause, "Build block...", "")
	// }

	// allEdges, err = GetAllEdges(env.ctx, &env.api, env.Docs)
	// assert.NilError(t, err)
	// assert.Equal(t, len(allEdges), 5)

	// for i := 0; i < 5; i++ {
	// 	checkEdge(t, env, docs[0], docs[i], eos.Name("test"))
	// }

	// // remove edges based on the from_node and edge_name
	// _, err = EdgeIdxTest(env.ctx, &env.api, env.Docs, docs[0].Hash, eos.Name("test"))
	// assert.NilError(t, err)

	// allEdges, err = GetAllEdges(env.ctx, &env.api, env.Docs)
	// assert.NilError(t, err)
	// assert.Equal(t, len(allEdges), 0)
	// // *****************************  END
}
