package localratelimit

import (
	"os"
	"path/filepath"
	"testing"
	"time"

	"istio.io/proxy/test/envoye2e/driver"
	"istio.io/proxy/test/envoye2e/env"
	"istio.io/proxy/testdata"

	"github.com/istio-ecosystem/wasm-extensions/test"
)

func TestLocalRateLimit(t *testing.T) {
	params := driver.NewTestParams(t, map[string]string{
		"LocalRateLimitWasmFile": filepath.Join(env.GetBazelBinOrDie(), "extensions/local_rate_limit/local_rate_limit.wasm"),
	}, test.ExtensionE2ETests)
	params.Vars["ServerHTTPFilters"] = params.LoadTestData("test/localratelimit/testdata/server_filter.yaml.tmpl")
	if err := (&driver.Scenario{
		Steps: []driver.Step{
			&driver.XDS{},
			&driver.Update{
				Node: "server", Version: "0", Listeners: []string{string(testdata.MustAsset("listener/server.yaml.tmpl"))},
			},
			&driver.Envoy{
				Bootstrap:       params.FillTestData(string(testdata.MustAsset("bootstrap/server.yaml.tmpl"))),
				DownloadVersion: os.Getenv("ISTIO_TEST_VERSION"),
			},
			&driver.Sleep{Duration: 3 * time.Second},
			// Test with max token 20, per refill 10, and refill interval 1s.
			// The first 20 request will get 200, then next 10 request will get 429.
			&driver.Repeat{
				N: 20,
				Step: &driver.HTTPCall{
					Port:         params.Ports.ServerPort,
					ResponseCode: 200,
				},
			},
			&driver.Repeat{
				N: 10,
				Step: &driver.HTTPCall{
					Port:         params.Ports.ServerPort,
					ResponseCode: 429,
					Body:         "rate_limited",
				},
			},
			// Sleep 1s, and token bucket should be refilled with 10 tokens.
			&driver.Sleep{Duration: 1 * time.Second},
			&driver.Repeat{
				N: 10,
				Step: &driver.HTTPCall{
					Port:         params.Ports.ServerPort,
					ResponseCode: 200,
				},
			},
			&driver.Repeat{
				N: 10,
				Step: &driver.HTTPCall{
					Port:         params.Ports.ServerPort,
					ResponseCode: 429,
					Body:         "rate_limited",
				},
			},
		},
	}).Run(params); err != nil {
		t.Fatal(err)
	}
}
