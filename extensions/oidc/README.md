# wasm-oidc-plugin

[![Build Status](https://github.com/antonengelhardt/wasm-oidc-plugin/actions/workflows/build.yml/badge.svg)](https://github.com/antonengelhardt/wasm-oidc-plugin/actions/workflows/build.yml) [![Documentation](https://img.shields.io/badge/docs-blue)](https://antonengelhardt.github.io/wasm-oidc-plugin/wasm_oidc_plugin/index.html#)

A plugin for the [Envoy-Proxy](https://www.envoyproxy.io/) written in [Rust](https://www.rust-lang.org). It is a HTTP Filter, that implements the OIDC Authorization Code Flow. Requests sent to the filter are checked for the presence of a valid session cookie. If the cookie is not present, the user is redirected to the `authorization_endpoint` to authenticate. After successful authentication, the user is redirected back to the original request with a code in the URL query. The plugin then exchanges the code for a token using the `token_endpoint` and stores the token in the session. If the cookie is present, the plugin validates the token and passes the request to the backend, if the token is valid (optional).

## Why this repo?

This repo is the result of a bachelor thesis in Information Systems. It is inspired by two other projects: [oidc-filter](https://github.com/dgn/oidc-filter) & [wasm-oauth-filter](https://github.com/sonhal/wasm-oauth-filter). This project has several advantages and improvements:

1. **Encryption**: The session in which the authorization state is stored is encrypted using AES-256, by providing a Key in the config and a session-based nonce. This prevents the session from being read by the user and potentially modified. If the user tries to modify the session, the decryption fails and the user is redirected to the `authorization_endpoint` to authenticate again.
2. **Configuration**: Many configuration options are available to customize the plugin to your needs. More are coming ;)
3. **Stability**: The plugin aims to be stable and ready for production. All forceful value unwraps are expected to be valid. If the value may be invalid or in the wrong format, error handling is in place.
4. **Optional validation**: The plugin can be configured to validate the token or not. If the validation is disabled, the plugin only checks for the presence of the token and passes the request to the backend. This is because the validation is taking a considerable amount of time. This time becomes worse with the length of the signing key. Cryptographic support is not fully mature in WASM yet, but [there is hope](https://github.com/WebAssembly/wasi-crypto/blob/main/docs/HighLevelGoals.md).
5. **Documentation and comments**: The code is documented and commented, so that it is easy to understand and extend.

## Install

### Install Toolchain for WASM in Rust

For developing the [Rust Toolchain](https://www.rust-lang.org/tools/install) has to be installed and the WASM target has to be enabled. E.g. for Ubuntu this can be achieved by:

```sh
# Install Build essentials
apt install build-essential
# Install Rustup
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
# Enable WASM compilation target
cargo build --target wasm32-wasi --release
```

## Run

**Shortcut** (make sure to have [make](https://www.gnu.org/software/make/) installed):

```sh
make simulate
```

---

### Detailed variant

1. **Building the plugin:**

```sh
cargo build --target wasm32-wasi --release
```

1. **Testing locally with Envoy** ([docker](https://www.docker.com/) and [docker-compose](https://docs.docker.com/compose/install/) are needed):

```sh
docker compose up
```

1. **Requests to the locally running envoy with the plugin enabled:**

```sh
curl localhost:10000
```

## Deploy to Kubernetes

To deploy the plugin to production, the following steps are needed (either manually or via a [CI/CD pipeline](./k8s/ci.yml)):

1. Build the plugin

    1.1 with `cargo build --target wasm32-wasi --release` - this can be done in a [initContainer](./k8s/deployment.yaml) (see [k8s](./k8s) folder) and then copy the binary to the path `/etc/envoy/proxy-wasm-plugins/` in the envoy container.

    1.2 by using the pre-built Docker image [antonengelhardt/wasm-oidc-plugin](https://hub.docker.com/r/antonengelhardt/wasm-oidc-plugin).
2. Run envoy as a container with the `envoy.yaml` file mounted through the [ConfigMap](./k8s/configmap.yml) as a volume.
3. Set up [Service](./k8s/service.yml), [Certificate](./k8s/certificate-production.yml), [Ingress](./k8s/ingress.yml) to expose the Envoy to the internet.

For reference, see the [k8s folder](./k8s).

## Documentation

To generate a detailed documentation, run (also [hosted on GitHub Pages](https://antonengelhardt.github.io/wasm-oidc-plugin/wasm_oidc_plugin/index.html#)):

```sh
cargo doc --document-private-items --open
```

### Configuration

The plugin is configured via the `envoy.yaml`-file. The following configuration options are required:

| Name | Type | Description | Example |
| ---- | ---- | ----------- | ------- |
| `config_endpoint` | `string` | The open id configuration endpoint. | `https://accounts.google.com/.well-known/openid-configuration` |
| `reload_interval_in_hours` | `u64` | The interval in hours, after which the OIDC configuration is reloaded. | `24` |
| `exclude_hosts` | `Vec<Regex>` | A comma separated list Hosts (in Regex expressions), that are excluded from the filter. | `["localhost:10000"]` |
| `exclude_paths` | `Vec<Regex>` | A comma separated list of paths (in Regex expressions), that are excluded from the filter. | `["/health"]` |
| `exclude_urls` | `Vec<Regex>` | A comma separated list of URLs (in Regex expressions), that are excluded from the filter. | `["localhost:10000/health"]` |
| `access_token_header_name` | `string` | If set, this name will be used to forward the access token to the backend. | `X-Access-Token` |
| `access_token_header_prefix` | `string` | The prefix of the header, that is used to forward the access token, if empty "" is used. | `Bearer ` |
| `id_token_header_name` | `string` | If set, this name will be used to forward the id token to the backend. | `X-Id-Token` |
| `id_token_header_prefix` | `string` | The prefix of the header, that is used to forward the id token, if empty "" is used. | `Bearer ` |
| `cookie_name` | `string` | The name of the cookie, that is used to store the session. | `oidcSession` |
| `cookie_duration` | `u64` | The duration in seconds, after which the session cookie expires. | `86400` |
| `token_validation` | bool | Whether to validate the token or not. | `true` |
| `aes_key` | `string` | A base64 encoded AES-256 Key: `openssl rand -base64 32` | `SFDUGDbOsRzSZbv+mvnZdu2x6+Hqe2WRaBABvfxmh3Q=` |
| `authority` | `string` | The authority of the `authorization_endpoint`. | `accounts.google.com` |
| `redirect_uri` | `string` | The redirect URI, that the `authorization_endpoint` will redirect to. | `http://localhost:10000/oidc/callback` |
| `client_id` | `string` | The client ID, for getting and exchanging the code. | `wasm-oidc-plugin` |
| `scope` | `string` | The scope, to validate | `openid email` |
| `claims` | `string` | The claims, to validate. Make sure to escape quotes with a backslash | `{\"id_token\":{\"groups\":null,\"username\":null}}` |
| `client_secret` | `string` | The client secret, that is used to authenticate with the `authorization_endpoint`. | `secret` |
| `audience` | `string` | The audience, that is used to validate the token. | `wasm-oidc-plugin` |

With these configuration options, the plugin starts and loads more information itself such as the OIDC providers public keys, issuer, etc.

### States

For that a state is used, which determines, what to load next. The following states are possible and depending on the outcome, the state is changed or not:

| State | Description |
| ---- | ----------- |
| `Uninitialized` | The plugin is not initialized yet. |
| `LoadingConfig` | The plugin is loading the configuration from the `config_endpoint`. |
| `LoadingJwks` | The plugin is loading the public keys from the `jwks_uri`. |
| `Ready` | The plugin is ready to handle requests and will reload the configuration after the `reload_interval_in_hours` has passed. |

![State Diagram](./docs/sequence-discovery.png)

### Handling a request

When a new request arrives, the root context creates a new http context with the information that has been loaded previously.

Then, one of the following cases is handled:

1. The filter is not configured yet and still loading the configuration. The request is paused and queued until the configuration is loaded. Then, the RootContext resumes the request and the Request is redirected in order to create a new context.
2. The request has the code parameter in the URL query. This means that the user has been redirected back from the `authorization_endpoint` after successful authentication. The plugin exchanges the code for a token using the `token_endpoint` and stores the token in the session. Then, the user is redirected back to the original request.
3. The request has a valid session cookie. The plugin decoded, decrypts and then validates the cookie and passes the request depending on the outcome of the validation of the token.
4. The request has no valid session cookie. The plugin redirects the user to the `authorization_endpoint` to authenticate. Once, the user returns, the second case is handled.

![Sequence Diagram](./docs/sequence-authorization-code-flow.png)

## Tools

### Gitleaks

We are using [Gitleaks](https://github.com/gitleaks/gitleaks) to protect from unwanted secret leaking and prevent security incidents by detecting passwords, secrets, API keys, tokens and more in git repos.

To run gitleaks, install it first and then run:

```bash
gitleaks protect

# To get the list of leaks
gitleaks protect --verbose
```

If you want to install a pre-commit hook - you should - install [pre-commit](https://pre-commit.com/) and run (from the root of the project):

```bash
pre-commit install
```

### Cargo-Deny

Cargo-deny checks all dependencies for security vulnerabilities and license issues.

Install cargo-deny:

```bash
cargo install --locked cargo-deny
```

And then run:

```bash
cargo-deny check licenses
cargo-deny check advisories
```

These commands are also run in the CI pipeline.
