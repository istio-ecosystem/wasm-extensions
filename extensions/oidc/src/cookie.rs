use std::fmt::Debug;

// serde
use serde::{Deserialize, Serialize};
use serde_json;

// log
use log::{warn,debug};

// base64
use base64::{engine::general_purpose::STANDARD_NO_PAD as base64engine, Engine as _};

// aes_gcm
use aes_gcm::{Aes256Gcm, aead::{OsRng, AeadMut}, AeadCore};

/// Struct parse the cookie from the request into a struct in order to access the fields and
/// also to save the cookie on the client side
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AuthorizationState {
    /// Access token to be used for requests to the API
    pub access_token: String,
    /// Type of the access token
    pub token_type: String,
    /// Time in seconds until the access token expires
    pub expires_in: u32,
    /// Refresh token to be used to refresh the access token
    #[serde(skip_serializing_if = "Option::is_none")]
    pub refresh_token: Option<String>,
    /// ID token in JWT format
    pub id_token: String,
}

/// Struct that holds the encoded cookie and the encoded nonce as well as the access token and the id token
pub struct EncodedCookies {
    /// Encoded cookie
    pub encoded_cookie: String,
    /// Encoded nonce
    pub encoded_nonce: String,
    /// Access token
    pub access_token: String,
    /// ID token
    pub id_token: String,
}

/// Implementation of the AuthorizationState struct
impl AuthorizationState {

    /// Create a new encoded cookie from the response coming from the Token Endpoint
    pub fn create_cookie_from_response(mut cipher: Aes256Gcm, res: &[u8]) -> Result<EncodedCookies, String> {

        // Format the response into a slice and parse it in a struct
        match serde_json::from_slice::<AuthorizationState>(&res) {

            // If deserialization was successful, return the encrypted and encoded cookie
            Ok(state) => {

                // Generate nonce and encode it
                let nonce = Aes256Gcm::generate_nonce(OsRng);
                let encoded_nonce = base64engine.encode(nonce.as_slice());

                // Encrypt cookie
                let encrypted_cookie = cipher.encrypt(&nonce, serde_json::to_vec(&state).unwrap().as_slice()).unwrap();

                // Encode cookie
                let encoded_cookie = base64engine.encode(encrypted_cookie.as_slice());

                Ok(EncodedCookies {
                    encoded_cookie,
                    encoded_nonce,
                    access_token: state.access_token,
                    id_token: state.id_token,
                })
            },
            // If the cookie cannot be parsed into a struct, return an error
            Err(e) => {
                warn!("The token response is not in the required format: {}", e);
                return Err(e.to_string())
            }
        }
    }

    /// Decode cookie, parse into a struct in order to access the fields and
    /// validate the ID Token
    pub fn decode_and_decrypt_cookie(cookie: String, mut cipher: Aes256Gcm, nonce: String) -> Result<AuthorizationState, String> {

        // Decode nonce using base64
        let decoded_nonce = match base64engine.decode(nonce.as_bytes()) {
            Ok(s) => s,
            Err(e) => {
                warn!("The nonce could not be decoded: {}", e);
                return Err(e.to_string());
            }
        };
        let nonce = aes_gcm::Nonce::from_slice(decoded_nonce.as_slice());
        debug!("Nonce: {:?}", nonce);

        // Decode cookie using base64
        let decoded_cookie = match base64engine.decode(cookie.as_bytes()) {
            Ok(s) => s,
            Err(e) => {
                warn!("The cookie could not be decoded: {}", e);
                return Err(e.to_string());
            }
        };

        // Decrypt with cipher
        let decrypted_cookie = match cipher.decrypt(nonce, decoded_cookie.as_slice()) {
            Ok(s) => s,
            Err(e) => {
                warn!("The cookie could not be decrypted: {}", e);
                return Err(e.to_string());
            }
        };

        // Parse cookie into a struct
        match serde_json::from_slice::<AuthorizationState>(&decrypted_cookie) {

            // If deserialization was successful, set the cookie and resume the request
            Ok(state) => {
                debug!("State: {:?}", state);
                return Ok(state)
            },
            // If the cookie cannot be parsed into a struct, return an error
            Err(e) => {
                warn!("The cookie didn't match the expected format: {}", e);
                return Err(e.to_string())
            }
        }
    }
}
