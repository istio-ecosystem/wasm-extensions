package testserver

import (
	"context"
	"errors"
	"fmt"
	"log"
	"net"
	"sync"
	"time"

	"github.com/golang/protobuf/proto"
	pb "github.com/istio-ecosystem/wasm-extensions/test/grpclogging/testserver/proto"

	"google.golang.org/grpc"
	"istio.io/proxy/test/envoye2e/driver"
)

type Server struct {
	Port    uint16
	s       *grpc.Server
	Request *pb.WriteLogRequest
	Mux     sync.Mutex
	pb.UnimplementedLoggingServiceServer
}

var _ driver.Step = &Server{}
var _ pb.LoggingServiceServer = &Server{}

func (srv *Server) runLoggingServer() {
	lis, err := net.Listen("tcp", fmt.Sprintf(":%v", srv.Port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	fmt.Printf("start listening on %v \n", srv.Port)
	srv.s = grpc.NewServer()
	pb.RegisterLoggingServiceServer(srv.s, srv)
	if err := srv.s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}

func (srv *Server) Run(p *driver.Params) error {
	go srv.runLoggingServer()
	return nil
}

func (srv *Server) Cleanup() {
	srv.s.Stop()
}

func (srv *Server) WriteLog(ctx context.Context, in *pb.WriteLogRequest) (*pb.WriteLogResponse, error) {
	srv.Mux.Lock()
	defer srv.Mux.Unlock()
	srv.Request = in
	return &pb.WriteLogResponse{}, nil
}

type VerifyLogs struct {
	WantRequest *pb.WriteLogRequest
	Server      *Server
}

func (v *VerifyLogs) Run(p *driver.Params) error {
	for i := 0; i < 20; i++ {
		// Expect only one log entry was received
		v.Server.Mux.Lock()
		if v.Server.Request != nil {
			v.Server.Request.LogEntries[0].Timestamp = nil
			v.Server.Request.LogEntries[0].Latency = nil
			v.Server.Request.LogEntries[0].SourceAddress = ""
			v.Server.Request.LogEntries[0].RequestId = ""
			if proto.Equal(v.Server.Request, v.WantRequest) {
				return nil
			}
			return fmt.Errorf("log request want %+v, got %+v", v.WantRequest, v.Server.Request)
		}
		v.Server.Mux.Unlock()
		fmt.Printf("no log entry received, retry after 1 sec")
		time.Sleep(1 * time.Second)
	}
	return errors.New("time out on waiting for log request")
}

func (v *VerifyLogs) Cleanup() {}
