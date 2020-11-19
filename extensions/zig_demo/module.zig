const std = @import("std");
const Allocator = std.mem.Allocator;
const HashMap = std.HashMap;

/// Core enumeration types.
const Result = packed enum(i32) {
    ok,
    not_found,
    bad_argument,
    serialization_failure,
    parse_failure,
    bad_expression,
    invalid_memory_access,
    empty,
    cas_mismatch,
    result_mismatch,
    internal_failure,
    broken_connection,
    unimplemented,
};
const LogLevel = packed enum(i32) { trace, debug, info, warn, err, critical };
const FilterHeadersStatus = packed enum(i32) { ok, stop, continue_and_end_stream, stop_all_iteration_and_buffer, stop_all_iteration_and_watermark };
const FilterMetadataStatus = packed enum(i32) { ok };
const FilterTrailersStatus = packed enum(i32) { ok, stop };
const FilterDataStatus = packed enum(i32) { ok, stop_iteration_and_buffer, stop_iteration_and_watermark, stop_iteration_no_buffer };
const BufferType = packed enum(i32) {
    http_request_body,
    http_response_body,
    network_downstream_data,
    network_upstream_data,
    http_call_response_body,
    grpc_receive_buffer,
    vm_configuration,
    plugin_configuration,
    call_data,
};
const HeaderType = packed enum(i32) {
    request_headers,
    request_trailers,
    response_headers,
    response_trailers,
    grpc_receive_initial_metadata,
    grpc_receive_trailing_metadata,
    http_call_response_headers,
    http_call_response_trailers,
};

/// ABI compatibility stub.
export fn proxy_abi_version_0_2_0() void {}

/// Core functions to accept data from the host.
var buf: [64 * 1024]u8 = undefined;
var buf_len: usize = undefined;
export fn malloc(len: usize) [*]u8 {
    if (len > buf.len) {
        @panic("malloc buffer overrun");
    }
    buf_len = len;
    return &buf;
}

// Buffer is overwritten after each ABI call. Copy if needed.
fn buffer() []const u8 {
    return buf[0..buf_len];
}
fn bufferCopy(allocator: *Allocator) ![]const u8 {
    return std.mem.dupe(allocator, u8, buf[0..buf_len]);
}
fn parsePairs(pairs: []const u8, map: anytype) Result {
    var n = @bitCast(usize, pairs[0..4].*);
    var offset: usize = 4;
    var i: usize = 0;
    var slice: []const u8 = undefined;
    var keys: [1024]usize = undefined;
    var values: [1024]usize = undefined;
    while (i < n) {
        slice = pairs[offset .. offset + 4];
        keys[i] = @bitCast(usize, slice[0..4].*);
        offset += 4;
        slice = pairs[offset .. offset + 4];
        values[i] = @bitCast(usize, slice[0..4].*);
        offset += 4;
        i += 1;
    }
    i = 0;
    while (i < n) {
        var key = pairs[offset .. offset + keys[i]];
        offset += keys[i] + 1;
        var value = pairs[offset .. offset + values[i]];
        offset += values[i] + 1;
        map.put(key, value) catch |_| return .parse_failure;
        i += 1;
    }
    return .ok;
}

/// Logging.
extern "env" fn proxy_log(LogLevel, [*]const u8, usize) Result;
fn log(comptime l: LogLevel, comptime fmt: []const u8, args: anytype) void {
    var log_buf: [1024]u8 = undefined;
    const msg = std.fmt.bufPrint(log_buf[0..], fmt, args) catch |_| {
        const err: []const u8 = "buffer too small";
        _ = proxy_log(.trace, err.ptr, err.len);
        return;
    };
    _ = proxy_log(l, msg.ptr, msg.len);
}

/// Access and set host properties.
extern "env" fn proxy_get_property([*]const u8, usize, *i32, *i32) Result;
fn getProperty(comptime T: type, comptime parts: anytype) ?T {
    comptime var size: usize = 0;
    inline for (parts) |part| {
        size += part.len + 1;
    }

    comptime var path: [size]u8 = undefined;
    comptime var i: usize = 0;
    inline for (parts) |part| {
        @memcpy(path[i..], part, part.len);
        i = i + part.len + 1;
        path[i - 1] = 0;
    }

    var addr1: i32 = undefined;
    var addr2: i32 = undefined;
    if (proxy_get_property(&path, size, &addr1, &addr2) != .ok) {
        return null;
    }
    switch (T) {
        i64, u64 => {
            std.debug.assert(buf_len == @sizeOf(T));
            return @bitCast(T, buf[0..@sizeOf(T)].*);
        },
        []const u8 => return buffer(),
        else => return null,
    }
}
extern "env" fn proxy_set_property([*]const u8, usize, [*]const u8, usize) Result;
fn setProperty(key: []const u8, value: []const u8) Result {
    return proxy_set_property(key.ptr, key.len, value.ptr, value.len);
}

/// Timer.
extern "env" fn proxy_set_tick_period_milliseconds(millis: u32) Result;
export fn proxy_on_tick(id: u32) void {}

/// Stream processing callbacks.
export fn proxy_on_context_create(id: u32, parent_id: u32) void {
    log(.info, "context create {} from {}, memory size pages (64kB) {}", .{ id, parent_id, @wasmMemorySize(0) });
}
export fn proxy_on_request_headers(id: u32, headers: u32, end_stream: u32) FilterHeadersStatus {
    log(.info, "request headers {} {} {}", .{ id, headers, end_stream });
    _ = addHeader(.request_headers, "wasm-test", "wasm-value");
    return .ok;
}
export fn proxy_on_request_body(id: u32, body_buffer_length: u32, end_stream: u32) FilterDataStatus {
    return .ok;
}
export fn proxy_on_request_trailers(id: u32, trailers: u32) FilterTrailersStatus {
    return .ok;
}
export fn proxy_on_request_metadata(id: u32, nelements: u32) FilterMetadataStatus {
    return .ok;
}
export fn proxy_on_response_headers(id: u32, headers: u32, end_stream: u32) FilterHeadersStatus {
    log(.info, "response headers {} {} {}", .{ id, headers, end_stream });
    return .ok;
}
export fn proxy_on_response_body(id: u32, body_buffer_length: u32, end_stream: u32) FilterDataStatus {
    return .ok;
}
export fn proxy_on_response_trailers(id: u32, trailers: u32) FilterTrailersStatus {
    return .ok;
}
export fn proxy_on_response_metadata(id: u32, nelements: u32) FilterMetadataStatus {
    return .ok;
}
export fn proxy_on_log(id: u32) void {
    log(.info, "log {}", .{id});
    log(.info, "request size {}, with headers {}, header value {}, which is same as {}", .{
        getProperty(i64, .{ "request", "size" }),
        getProperty(i64, .{ "request", "total_size" }),
        getHeader(.request_headers, "wasm-test"),
        getProperty([]const u8, .{ "request", "headers", "wasm-test" }),
    });
    var map_buffer: [4096]u8 = undefined;
    var allocator = &std.heap.FixedBufferAllocator.init(&map_buffer).allocator;
    var map = std.StringHashMap([]const u8).init(allocator);
    _ = getAllHeaders(.request_headers, &map);
    var it = map.iterator();
    while (it.next()) |next| {
        log(.info, "{}: {}", .{ next.key, next.value });
    }
}
/// Buffers.
extern "env" fn proxy_get_buffer_bytes(t: BufferType, start: u32, len: u32, *i32, *i32) Result;

/// Headers.
extern "env" fn proxy_add_header_map_value(t: HeaderType, [*]const u8, usize, [*]const u8, usize) Result;
fn addHeader(t: HeaderType, key: []const u8, value: []const u8) Result {
    return proxy_add_header_map_value(t, key.ptr, key.len, value.ptr, value.len);
}
extern "env" fn proxy_get_header_map_value(t: HeaderType, [*]const u8, usize, *i32, *i32) Result;
fn getHeader(t: HeaderType, key: []const u8) ?[]const u8 {
    var addr1: i32 = undefined;
    var addr2: i32 = undefined;
    if (proxy_get_header_map_value(t, key.ptr, key.len, &addr1, &addr2) == .ok) {
        return buffer();
    }
    return null;
}
extern "env" fn proxy_replace_header_map_value(t: HeaderType, [*]const u8, usize, [*]const u8, usize) Result;
fn replaceHeader(t: HeaderType, key: []const u8, value: []const u8) Result {
    return proxy_replace_header_map_value(t, key.ptr, key.len, value.ptr, value.len);
}
extern "env" fn proxy_remove_header_map_value(t: HeaderType, [*]const u8, usize) Result;
fn removeHeader(t: HeaderType, key: []const u8) Result {
    return proxy_remove_header_map_value(t, key.ptr, key.len);
}
extern "env" fn proxy_get_header_map_pairs(t: HeaderType, *i32, *i32) Result;
fn getAllHeaders(t: HeaderType, map: anytype) Result {
    var addr1: i32 = undefined;
    var addr2: i32 = undefined;
    const result = proxy_get_header_map_pairs(t, &addr1, &addr2);
    if (result == .ok) {
        return parsePairs(buffer(), map);
    }
    return result;
}
extern "env" fn proxy_set_header_map_pairs(t: HeaderType, [*]const u8, usize) Result;
extern "env" fn proxy_get_header_map_size(t: HeaderType, *i32) Result;

/// Lifecycle.
extern "env" fn proxy_set_effective_context(id: u32) Result;

export fn proxy_on_vm_start(id: u32, len: u32) bool {
    log(.info, "vm start {}", .{id});
    return true;
}

export fn proxy_on_configure(id: u32, len: u32) bool {
    var addr1: i32 = undefined;
    var addr2: i32 = undefined;
    if (proxy_get_buffer_bytes(.plugin_configuration, 0, len, &addr1, &addr2) == .ok) {
        log(.info, "configure {} {}", .{ id, buffer() });
    }
    return true;
}
