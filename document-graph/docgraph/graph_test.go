package docgraph_test

import (
	"testing"

	"github.com/hashed-io/document-graph/docgraph"
)

func TestGraph(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	docgraph.LoadGraph(env.ctx, &env.api, env.Docs)
}
