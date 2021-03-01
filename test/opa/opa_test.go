package opa

import (
	"os"
	"path/filepath"
	"strconv"
	"testing"
	"time"

	"github.com/istio-ecosystem/wasm-extensions/test"
	opa "github.com/istio-ecosystem/wasm-extensions/test/opa/server"

	"istio.io/proxy/test/envoye2e/driver"
	"istio.io/proxy/test/envoye2e/env"
	"istio.io/proxy/testdata"
)

func TestOPA(t *testing.T) {
	var tests = []struct {
		name         string
		method       string
		cacheHit     int
		cacheMiss    int
		requestCount int
		delay        time.Duration
		wantRespCode int
	}{
		{"allow", "GET", 9, 1, 10, 0, 200},
		{"deny", "POST", 9, 1, 10, 0, 403},
		{"cache_expire", "POST", 2, 2, 4, 4 * time.Second, 403},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			params := driver.NewTestParams(t, map[string]string{
				"ClientTLSContext":    driver.LoadTestData("test/opa/testdata/transport_socket/client_tls_context.yaml.tmpl"),
				"ServerTLSContext":    driver.LoadTestData("test/opa/testdata/transport_socket/server_tls_context.yaml.tmpl"),
				"ServerStaticCluster": driver.LoadTestData("test/opa/testdata/resource/opa_cluster.yaml.tmpl"),
				"ServerMetadata":      driver.LoadTestData("test/opa/testdata/resource/server_node_metadata.yaml.tmpl"),
				"OpaPluginFilePath":   filepath.Join(env.GetBazelBinOrDie(), "extensions/open_policy_agent/open_policy_agent.wasm"),
				"CacheHit":            strconv.Itoa(tt.cacheHit),
				"CacheMiss":           strconv.Itoa(tt.cacheMiss),
			}, test.ExtensionE2ETests)
			params.Vars["ServerHTTPFilters"] = params.LoadTestData("test/opa/testdata/resource/opa_filter.yaml.tmpl")

			if err := (&driver.Scenario{
				Steps: []driver.Step{
					&driver.XDS{},
					&driver.Update{
						Node: "server", Version: "0", Listeners: []string{string(testdata.MustAsset("listener/server.yaml.tmpl"))},
					},
					&driver.Update{
						Node: "client", Version: "0", Listeners: []string{string(testdata.MustAsset("listener/client.yaml.tmpl"))},
					},
					&opa.OpaServer{RuleFilePath: driver.TestPath("test/opa/testdata/rule/opa_rule.rego")},
					&driver.Envoy{
						Bootstrap:       params.FillTestData(string(testdata.MustAsset("bootstrap/server.yaml.tmpl"))),
						DownloadVersion: os.Getenv("ISTIO_TEST_VERSION"),
					},
					&driver.Envoy{
						Bootstrap:       params.FillTestData(string(testdata.MustAsset("bootstrap/client.yaml.tmpl"))),
						DownloadVersion: os.Getenv("ISTIO_TEST_VERSION"),
					},
					&driver.Repeat{
						N: tt.requestCount,
						Step: &driver.Scenario{
							Steps: []driver.Step{
								&driver.HTTPCall{
									Port:         params.Ports.ClientPort,
									Method:       tt.method,
									Path:         "/echo",
									ResponseCode: tt.wantRespCode,
								},
								&driver.Sleep{Duration: tt.delay},
							},
						},
					},
					&driver.Stats{
						AdminPort: params.Ports.ServerAdmin,
						Matchers: map[string]driver.StatMatcher{
							"envoy_wasm_filter_opa_filter_cache_hit_policy_cache_count": &driver.
								ExactStat{Metric: "test/opa/testdata/stats/cache_hit.yaml.tmpl"},
							"envoy_wasm_filter_opa_filter_cache_miss_policy_cache_count": &driver.
								ExactStat{Metric: "test/opa/testdata/stats/cache_miss.yaml.tmpl"},
						},
					},
				}}).Run(params); err != nil {
				t.Fatal(err)
			}
		})
	}
}
