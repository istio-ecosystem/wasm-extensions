// serde
use serde::Deserialize;

// log
use log::{info, debug};

// base64
use {base64::engine::general_purpose::URL_SAFE_NO_PAD as base64engine_urlsafe, base64::Engine as _};

// jwt_simple
use jwt_simple::{self, prelude::{RSAPublicKeyLike, VerificationOptions, JWTClaims}, Error, claims::NoCustomClaims};

/// [OpenID Connect Discovery Response](https://openid.net/specs/openid-connect-discovery-1_0.html#ProviderConfig)
#[derive(Deserialize, Debug)]
pub struct OidcDiscoveryResponse {
    /// The issuer of the OpenID Connect Provider
    pub issuer: String,
    /// The authorization endpoint to start the code flow
    pub authorization_endpoint: String,
    /// The token endpoint to exchange the code for a token
    pub token_endpoint: String,
    /// The jwks uri to load the jwks response from
    pub jwks_uri: String,
}

#[derive(Deserialize, Debug)]
/// [JWKs response](https://tools.ietf.org/html/rfc7517)
/// Contains a list of keys that are retrieved from the jwks uri
pub struct JWKsResponse {
    /// The keys of the jwks response, see `JWK`
    pub keys: Vec<JsonWebKey>,
}

/// [JWK](https://tools.ietf.org/html/rfc7517)
/// Define the structure of each key type that are retrieved from the jwks uri
#[derive(Deserialize, Debug)]
#[serde(tag = "alg")]
pub enum JsonWebKey {
    /// A RSA Key of 256 bits
    RS256 {
        /// The key type like RSA
        kty: String,
        /// The Public Keys Component n, the modulus
        n: String,
        /// The Public Keys Component e, the exponent
        e: String,
    },
    // Add more key types here
}

/// Enum that holds the public keys that will be used for the validation of the ID Token
/// Essentially a wrapper to connect the `JWKsResponse` with the `jwt_simple` crate to use
/// the `verify_token` function
#[derive(Clone, Debug)]
pub enum SigningKey {
    /// A RSA Key of 256 bits
    RS256PublicKey(jwt_simple::algorithms::RS256PublicKey),
    // Add more key types here
}

/// A public key that can be used for the validation of the ID Token
impl SigningKey {

    /// Function that calls the `verify_token` function of the `jwt_simple` crate for each key type
    pub fn verify_token(&self, token: &str, options: VerificationOptions) -> Result<JWTClaims<NoCustomClaims>, Error> {

        match self {
            // RSA Key of 256 bits
            SigningKey::RS256PublicKey(key) => key.verify_token(token, Some(options)),
            // Add more key types here
        }
    }
}

/// Implementation of the `From` trait for the `SigningKey` enum to convert the `JsonWebKey` into
/// the `SigningKey` enum
impl From<JsonWebKey> for SigningKey {

    /// Function that converts the `JsonWebKey` into the `SigningKey` enum
    fn from(key: JsonWebKey) -> Self {
        match key {
            // RSA Key of 256 bits
            JsonWebKey::RS256 { kty, n, e, .. } => {

                // Check if the key is of type RSA
                if kty != "RSA" {
                    debug!("key is not of type RSA although alg is RS256");
                }

                // Decode and parse the public key components
                let n_dec = base64engine_urlsafe.decode(&n).unwrap();
                let e_dec = base64engine_urlsafe.decode(&e).unwrap();

                info!("loaded RS256 public key");

                return SigningKey::RS256PublicKey(
                    jwt_simple::algorithms::RS256PublicKey::from_components(&n_dec, &e_dec)
                .unwrap());
            },
            // Add more key types here
        }
    }
}

/// Struct that defines how the callback looks like to serialize it better
#[derive(Deserialize, Debug)]
pub struct Callback {
    /// The code that is returned from the authorization endpoint
    pub code: String,
}
