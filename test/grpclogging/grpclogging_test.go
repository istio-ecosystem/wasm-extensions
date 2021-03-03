package grpclogging

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/istio-ecosystem/wasm-extensions/test"
	"github.com/istio-ecosystem/wasm-extensions/test/grpclogging/testserver"
	pb "github.com/istio-ecosystem/wasm-extensions/test/grpclogging/testserver/proto"

	"istio.io/proxy/test/envoye2e/driver"
	"istio.io/proxy/test/envoye2e/env"
	"istio.io/proxy/testdata"
)

func TestGrpcLogging(t *testing.T) {
	params := driver.NewTestParams(t, map[string]string{
		"GrpcLoggingWasmFile": filepath.Join(env.GetBazelBinOrDie(), "extensions/grpc_logging/grpc_logging.wasm"),
		"ServerMetadata":      driver.LoadTestData("test/grpclogging/testdata/node_metadata.yaml.tmpl"),
	}, test.ExtensionE2ETests)

	params.Vars["LoggingPort"] = fmt.Sprintf("%d", params.Ports.Max+1)
	params.Vars["ServerHTTPFilters"] = params.LoadTestData("test/grpclogging/testdata/server_filter.yaml.tmpl")

	srv := &testserver.Server{Port: params.Ports.Max + 1}
	if err := (&driver.Scenario{
		Steps: []driver.Step{
			&driver.XDS{},
			srv,
			&driver.Update{
				Node: "server", Version: "0", Listeners: []string{string(testdata.MustAsset("listener/server.yaml.tmpl"))},
			},
			&driver.Envoy{
				Bootstrap:       params.FillTestData(string(testdata.MustAsset("bootstrap/server.yaml.tmpl"))),
				DownloadVersion: os.Getenv("ISTIO_TEST_VERSION"),
			},
			&driver.Sleep{1 * time.Second},
			&driver.HTTPCall{
				Port:   params.Ports.ServerPort,
				Method: "GET",
			},
			&testserver.VerifyLogs{
				WantRequest: &pb.WriteLogRequest{
					LogEntries: []*pb.WriteLogRequest_LogEntry{
						{
							DestinationWorkload:  "echo-server",
							DestinationNamespace: "test",
							DestinationAddress:   "127.0.0.1:20243",
							Host:                 "127.0.0.1:20243",
							Path:                 "/",
							ResponseCode:         200,
						},
					},
				},
				Server: srv,
			},
		}}).Run(params); err != nil {
		t.Fatal(err)
	}
}
