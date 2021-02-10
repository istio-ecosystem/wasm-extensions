package basicauth

import (
	"path/filepath"
	"testing"
	"time"

	"istio.io/proxy/test/envoye2e/driver"
	"istio.io/proxy/test/envoye2e/env"
	"istio.io/proxy/testdata"

	"github.com/istio-ecosystem/wasm-extensions/test"
)

func TestExamplePlugin(t *testing.T) {
	params := driver.NewTestParams(t, map[string]string{
		"ExampleWasmFile": filepath.Join(env.GetBazelBinOrDie(), "example.wasm"),
	}, test.ExtensionE2ETests)
	params.Vars["ServerHTTPFilters"] = params.LoadTestData("test/testdata/server_filter.yaml.tmpl")
	if err := (&driver.Scenario{
		Steps: []driver.Step{
			&driver.XDS{},
			&driver.Update{
				Node: "server", Version: "0", Listeners: []string{string(testdata.MustAsset("listener/server.yaml.tmpl"))},
			},
			&driver.Envoy{
				Bootstrap:       params.FillTestData(string(testdata.MustAsset("bootstrap/server.yaml.tmpl"))),
				DownloadVersion: "1.9",
			},
			&driver.Sleep{Duration: 1 * time.Second},
			&driver.HTTPCall{
				Port:            params.Ports.ServerPort,
				Method:          "GET",
				ResponseHeaders: map[string]string{"x-wasm-custom": "foo"},
				ResponseCode:    200,
			},
		},
	}).Run(params); err != nil {
		t.Fatal(err)
	}
}
