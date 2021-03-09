package test

default allow = false

allow = true {
  input.request_method == "GET"
  input.source_principal == "spiffe://cluster.local/ns/default/sa/client"
  input.destination_workload == "echo-server"
  input.request_url_path == "/echo"
}
