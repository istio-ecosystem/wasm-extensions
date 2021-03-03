// Code generated by protoc-gen-go-grpc. DO NOT EDIT.

package istio_ecosystem_wasm_extensions_grpc_logging

import (
	context "context"
	grpc "google.golang.org/grpc"
	codes "google.golang.org/grpc/codes"
	status "google.golang.org/grpc/status"
)

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
// Requires gRPC-Go v1.32.0 or later.
const _ = grpc.SupportPackageIsVersion7

// LoggingServiceClient is the client API for LoggingService service.
//
// For semantics around ctx use and closing/ending streaming RPCs, please refer to https://pkg.go.dev/google.golang.org/grpc/?tab=doc#ClientConn.NewStream.
type LoggingServiceClient interface {
	WriteLog(ctx context.Context, in *WriteLogRequest, opts ...grpc.CallOption) (*WriteLogResponse, error)
}

type loggingServiceClient struct {
	cc grpc.ClientConnInterface
}

func NewLoggingServiceClient(cc grpc.ClientConnInterface) LoggingServiceClient {
	return &loggingServiceClient{cc}
}

func (c *loggingServiceClient) WriteLog(ctx context.Context, in *WriteLogRequest, opts ...grpc.CallOption) (*WriteLogResponse, error) {
	out := new(WriteLogResponse)
	err := c.cc.Invoke(ctx, "/istio_ecosystem.wasm_extensions.grpc_logging.LoggingService/WriteLog", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// LoggingServiceServer is the server API for LoggingService service.
// All implementations must embed UnimplementedLoggingServiceServer
// for forward compatibility
type LoggingServiceServer interface {
	WriteLog(context.Context, *WriteLogRequest) (*WriteLogResponse, error)
	mustEmbedUnimplementedLoggingServiceServer()
}

// UnimplementedLoggingServiceServer must be embedded to have forward compatible implementations.
type UnimplementedLoggingServiceServer struct {
}

func (UnimplementedLoggingServiceServer) WriteLog(context.Context, *WriteLogRequest) (*WriteLogResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method WriteLog not implemented")
}
func (UnimplementedLoggingServiceServer) mustEmbedUnimplementedLoggingServiceServer() {}

// UnsafeLoggingServiceServer may be embedded to opt out of forward compatibility for this service.
// Use of this interface is not recommended, as added methods to LoggingServiceServer will
// result in compilation errors.
type UnsafeLoggingServiceServer interface {
	mustEmbedUnimplementedLoggingServiceServer()
}

func RegisterLoggingServiceServer(s grpc.ServiceRegistrar, srv LoggingServiceServer) {
	s.RegisterService(&LoggingService_ServiceDesc, srv)
}

func _LoggingService_WriteLog_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(WriteLogRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(LoggingServiceServer).WriteLog(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/istio_ecosystem.wasm_extensions.grpc_logging.LoggingService/WriteLog",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(LoggingServiceServer).WriteLog(ctx, req.(*WriteLogRequest))
	}
	return interceptor(ctx, in, info, handler)
}

// LoggingService_ServiceDesc is the grpc.ServiceDesc for LoggingService service.
// It's only intended for direct use with grpc.RegisterService,
// and not to be introspected or modified (even as a copy)
var LoggingService_ServiceDesc = grpc.ServiceDesc{
	ServiceName: "istio_ecosystem.wasm_extensions.grpc_logging.LoggingService",
	HandlerType: (*LoggingServiceServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "WriteLog",
			Handler:    _LoggingService_WriteLog_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "extensions/grpc_logging/log.proto",
}