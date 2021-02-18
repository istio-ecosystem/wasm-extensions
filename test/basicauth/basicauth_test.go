package basicauth

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

var TestCases = []struct {
	Name            string
	Method          string
	Path            string
	Realm           string
	RequestHeaders  map[string]string
	ResponseHeaders map[string]string
	ResponseCode    int
}{
	{
		Name:            "CorrectCredentials",
		Method:          "GET",
		Path:            "/api",
		RequestHeaders:  map[string]string{"Authorization": "Basic b2s6dGVzdA=="},
		ResponseHeaders: map[string]string{},
		ResponseCode:    200,
	},
	{
		Name:            "IncorrectCredentials",
		Method:          "POST",
		Path:            "/api/reviews/pay",
		RequestHeaders:  map[string]string{"Authorization": "Basic AtRtaW46YWRtaW4="},
		ResponseHeaders: map[string]string{"WWW-Authenticate": "Basic realm=istio"},
		ResponseCode:    401,
	},
	{
		Name:            "Base64Credentials",
		Method:          "POST",
		Path:            "/api/reviews/pay",
		RequestHeaders:  map[string]string{"Authorization": "Basic YWRtaW4zOmFkbWluMw=="},
		ResponseHeaders: map[string]string{},
		ResponseCode:    200,
	},
	{
		Name:            "MissingCredentials",
		Method:          "GET",
		Path:            "/api/reviews/pay",
		ResponseHeaders: map[string]string{"WWW-Authenticate": "Basic realm=istio"},
		ResponseCode:    401,
	},
	{
		Name:            "NoPathMatch",
		Path:            "/secret",
		ResponseHeaders: map[string]string{},
		ResponseCode:    200,
	},
	{
		Name:            "NoMethodMatch",
		Method:          "DELETE",
		Path:            "/api/reviews/pay",
		ResponseHeaders: map[string]string{},
		ResponseCode:    200,
	},
	{
		Name:            "NoConfigurationCredentialsProvided",
		Method:          "POST",
		Path:            "/api/reviews/pay",
		ResponseHeaders: map[string]string{"WWW-Authenticate": "Basic realm=istio"},
		ResponseCode:    401,
	},
	{
		Name:            "Realm",
		Method:          "POST",
		Path:            "/api/reviews/pay",
		Realm:           "test",
		ResponseHeaders: map[string]string{"WWW-Authenticate": "Basic realm=test"},
		ResponseCode:    401,
	},
}

func TestBasicAuth(t *testing.T) {
	for _, testCase := range TestCases {
		t.Run(testCase.Name, func(t *testing.T) {
			params := driver.NewTestParams(t, map[string]string{
				"BasicAuthWasmFile": filepath.Join(env.GetBazelBinOrDie(), "extensions/basic_auth/basic_auth.wasm"),
			}, test.ExtensionE2ETests)
			if testCase.Realm != "" {
				params.Vars["Realm"] = testCase.Realm
			}
			params.Vars["ServerHTTPFilters"] = params.LoadTestData("test/basicauth/testdata/server_filter.yaml.tmpl")
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
					&driver.Sleep{Duration: 1 * time.Second},
					&driver.HTTPCall{
						Port:            params.Ports.ServerPort,
						Method:          testCase.Method,
						Path:            testCase.Path,
						RequestHeaders:  testCase.RequestHeaders,
						ResponseHeaders: testCase.ResponseHeaders,
						ResponseCode:    testCase.ResponseCode,
					},
				},
			}).Run(params); err != nil {
				t.Fatal(err)
			}
		})
	}
}
