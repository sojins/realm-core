/*************************************************************************
 *
 * Copyright 2022 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#ifndef REALM_ERROR_CODES_H
#define REALM_ERROR_CODES_H

typedef enum realm_error_category {
    RLM_ERR_CAT_LOGIC = 0x0002,
    RLM_ERR_CAT_RUNTIME = 0x0004,
    RLM_ERR_CAT_INVALID_ARG = 0x0008,
    RLM_ERR_CAT_FILE_ACCESS = 0x0010,
    RLM_ERR_CAT_SYSTEM_ERROR = 0x0020,
    RLM_ERR_CAT_APP_ERROR = 0x0040,
    RLM_ERR_CAT_CLIENT_ERROR = 0x0080,
    RLM_ERR_CAT_JSON_ERROR = 0x0100,
    RLM_ERR_CAT_SERVICE_ERROR = 0x0200,
    RLM_ERR_CAT_HTTP_ERROR = 0x0400,
    RLM_ERR_CAT_CUSTOM_ERROR = 0x0800,
} realm_error_category_e;

typedef enum realm_errno {
    RLM_ERR_NONE,
    RLM_ERR_OTHER_EXCEPTION,
    RLM_ERR_RUNTIME,
    RLM_ERR_RANGE_ERROR,
    RLM_ERR_BROKEN_INVARIANT,
    RLM_ERR_OUT_OF_MEMORY,
    RLM_ERR_OUT_OF_DISK_SPACE,
    RLM_ERR_ADDRESS_SPACE_EXHAUSTED,
    RLM_ERR_MAXIMUM_FILE_SIZE_EXCEEDED,
    RLM_ERR_INCOMPATIBLE_SESSION,
    RLM_ERR_INCOMPATIBLE_LOCK_FILE,
    RLM_ERR_INVALID_QUERY,
    RLM_ERR_DUPLICATE_PRIMARY_KEY_VALUE,
    RLM_ERR_UNSUPPORTED_FILE_FORMAT_VERSION,
    RLM_ERR_MULTIPLE_SYNC_AGENTS,
    RLM_ERR_OBJECT_ALREADY_EXISTS,
    RLM_ERR_NOT_CLONABLE,
    RLM_ERR_BAD_CHANGESET,
    RLM_ERR_SUBSCRIPTION_FAILED,
    RLM_ERR_FILE_OPERATION_FAILED,
    RLM_ERR_FILE_PERMISSION_DENIED,
    RLM_ERR_FILE_NOT_FOUND,
    RLM_ERR_FILE_ALREADY_EXISTS,
    RLM_ERR_INVALID_DATABASE,
    RLM_ERR_DECRYPTION_FAILED,
    RLM_ERR_INCOMPATIBLE_HISTORIES,
    RLM_ERR_FILE_FORMAT_UPGRADE_REQUIRED,

    RLM_ERR_SYSTEM_ERROR,

    RLM_ERR_LOGIC,
    RLM_ERR_NOT_SUPPORTED,
    RLM_ERR_BROKEN_PROMISE,
    RLM_ERR_CROSS_TABLE_LINK_TARGET,
    RLM_ERR_KEY_ALREADY_USED,
    RLM_ERR_WRONG_TRANSACTION_STATE,
    RLM_ERR_WRONG_THREAD,
    RLM_ERR_ILLEGAL_OPERATION,
    RLM_ERR_SERIALIZATION_ERROR,
    RLM_ERR_STALE_ACCESSOR,
    RLM_ERR_INVALIDATED_OBJECT,
    RLM_ERR_READ_ONLY_DB,
    RLM_ERR_DELETE_OPENED_REALM,
    RLM_ERR_MISMATCHED_CONFIG,
    RLM_ERR_CLOSED_REALM,
    RLM_ERR_INVALID_TABLE_REF,
    RLM_ERR_SCHEMA_VALIDATION_FAILED,
    RLM_ERR_SCHEMA_MISMATCH,
    RLM_ERR_INVALID_SCHEMA_VERSION,
    RLM_ERR_INVALID_ADDITIVE_SCHEMA_CHANGE,
    RLM_ERR_INVALID_READ_ONLY_SCHEMA_CHANGE,
    RLM_ERR_INVALID_EXTERNAL_SCHEMA_CHANGE,

    RLM_ERR_INVALID_ARGUMENT,
    RLM_ERR_PROPERTY_TYPE_MISMATCH,
    RLM_ERR_PROPERTY_NOT_NULLABLE,
    RLM_ERR_READ_ONLY_PROPERTY,
    RLM_ERR_MISSING_PROPERTY_VALUE,
    RLM_ERR_MISSING_PRIMARY_KEY,
    RLM_ERR_UNEXPECTED_PRIMARY_KEY,
    RLM_ERR_MODIFY_PRIMARY_KEY,
    RLM_ERR_INVALID_QUERY_STRING,
    RLM_ERR_INVALID_PROPERTY,
    RLM_ERR_INVALID_NAME,
    RLM_ERR_INVALID_DICTIONARY_KEY,
    RLM_ERR_INVALID_DICTIONARY_VALUE,
    RLM_ERR_INVALID_SORT_DESCRIPTOR,
    RLM_ERR_INVALID_ENCRYPTION_KEY,
    RLM_ERR_INVALID_QUERY_ARG,
    RLM_ERR_NO_SUCH_OBJECT,
    RLM_ERR_INDEX_OUT_OF_BOUNDS,
    RLM_ERR_LIMIT_EXCEEDED,
    RLM_ERR_OBJECT_TYPE_MISMATCH,
    RLM_ERR_NO_SUCH_TABLE,
    RLM_ERR_TABLE_NAME_IN_USE,
    RLM_ERR_ILLEGAL_COMBINATION,

    RLM_ERR_CUSTOM_ERROR,

    RLM_ERR_CLIENT_USER_NOT_FOUND,
    RLM_ERR_CLIENT_USER_NOT_LOGGED_IN,
    RLM_ERR_CLIENT_APP_DEALLOCATED,

    RLM_ERR_BAD_TOKEN,
    RLM_ERR_MALFORMED_JSON,
    RLM_ERR_MISSING_JSON_KEY,
    RLM_ERR_BAD_BSON_PARSE,

    RLM_ERR_MISSING_AUTH_REQ,
    /// Invalid session, expired, no associated user, or app domain mismatch
    RLM_ERR_INVALID_SESSION,
    RLM_ERR_USER_APP_DOMAIN_MISMATCH,
    RLM_ERR_DOMAIN_NOT_ALLOWED,
    RLM_ERR_READ_SIZE_LIMIT_EXCEEDED,
    RLM_ERR_INVALID_PARAMETER,
    RLM_ERR_MISSING_PARAMETER,
    RLM_ERR_TWILIO_ERROR,
    RLM_ERR_GCM_ERROR,
    RLM_ERR_HTTP_ERROR,
    RLM_ERR_AWS_ERROR,
    RLM_ERR_MONGODB_ERROR,
    RLM_ERR_ARGUMENTS_NOT_ALLOWED,
    RLM_ERR_FUNCTION_EXECUTION_ERROR,
    RLM_ERR_NO_MATCHING_RULE,
    RLM_ERR_INTERNAL_SERVER_ERROR,
    RLM_ERR_AUTH_PROVIDER_NOT_FOUND,
    RLM_ERR_AUTH_PROVIDER_ALREADY_EXISTS,
    RLM_ERR_SERVICE_NOT_FOUND,
    RLM_ERR_SERVICE_TYPE_NOT_FOUND,
    RLM_ERR_SERVICE_ALREADY_EXISTS,
    RLM_ERR_SERVICE_COMMAND_NOT_FOUND,
    RLM_ERR_VALUE_NOT_FOUND,
    RLM_ERR_VALUE_ALREADY_EXISTS,
    RLM_ERR_VALUE_DUPLICATE_NAME,
    RLM_ERR_FUNCTION_NOT_FOUND,
    RLM_ERR_FUNCTION_ALREADY_EXISTS,
    RLM_ERR_FUNCTION_DUPLICATE_NAME,
    RLM_ERR_FUNCTION_SYNTAX_ERROR,
    RLM_ERR_FUNCTION_INVALID,
    RLM_ERR_INCOMING_WEBHOOK_NOT_FOUND,
    RLM_ERR_INCOMING_WEBHOOK_ALREADY_EXISTS,
    RLM_ERR_INCOMING_WEBHOOK_DUPLICATE_NAME,
    RLM_ERR_RULE_NOT_FOUND,
    RLM_ERR_API_KEY_NOT_FOUND,
    RLM_ERR_RULE_ALREADY_EXISTS,
    RLM_ERR_RULE_DUPLICATE_NAME,
    RLM_ERR_AUTH_PROVIDER_DUPLICATE_NAME,
    RLM_ERR_RESTRICTED_HOST,
    RLM_ERR_API_KEY_ALREADY_EXISTS,
    RLM_ERR_INCOMING_WEBHOOK_AUTH_FAILED,
    RLM_ERR_EXECUTION_TIME_LIMIT_EXCEEDED,
    RLM_ERR_NOT_CALLABLE,
    RLM_ERR_USER_ALREADY_CONFIRMED,
    RLM_ERR_USER_NOT_FOUND,
    RLM_ERR_USER_DISABLED,
    RLM_ERR_AUTH_ERROR,
    RLM_ERR_BAD_REQUEST,
    RLM_ERR_ACCOUNT_NAME_IN_USE,
    RLM_ERR_INVALID_PASSWORD,
    RLM_ERR_SCHEMA_VALIDATION_FAILED_WRITE,
    RLM_ERR_APP_UNKNOWN,

    RLM_ERR_CALLBACK = 1000000, /**< A user-provided callback failed. */
    RLM_ERR_UNKNOWN = 2000000   /* Should not be used in code */
} realm_errno_e;

typedef enum realm_sync_errno_client {
    RLM_SYNC_ERR_CLIENT_CONNECTION_CLOSED = 100,
    RLM_SYNC_ERR_CLIENT_UNKNOWN_MESSAGE = 101,
    RLM_SYNC_ERR_CLIENT_BAD_SYNTAX = 102,
    RLM_SYNC_ERR_CLIENT_LIMITS_EXCEEDED = 103,
    RLM_SYNC_ERR_CLIENT_BAD_SESSION_IDENT = 104,
    RLM_SYNC_ERR_CLIENT_BAD_MESSAGE_ORDER = 105,
    RLM_SYNC_ERR_CLIENT_BAD_CLIENT_FILE_IDENT = 106,
    RLM_SYNC_ERR_CLIENT_BAD_PROGRESS = 107,
    RLM_SYNC_ERR_CLIENT_BAD_CHANGESET_HEADER_SYNTAX = 108,
    RLM_SYNC_ERR_CLIENT_BAD_CHANGESET_SIZE = 109,
    RLM_SYNC_ERR_CLIENT_BAD_ORIGIN_FILE_IDENT = 110,
    RLM_SYNC_ERR_CLIENT_BAD_SERVER_VERSION = 111,
    RLM_SYNC_ERR_CLIENT_BAD_CHANGESET = 112,
    RLM_SYNC_ERR_CLIENT_BAD_REQUEST_IDENT = 113,
    RLM_SYNC_ERR_CLIENT_BAD_ERROR_CODE = 114,
    RLM_SYNC_ERR_CLIENT_BAD_COMPRESSION = 115,
    RLM_SYNC_ERR_CLIENT_BAD_CLIENT_VERSION = 116,
    RLM_SYNC_ERR_CLIENT_SSL_SERVER_CERT_REJECTED = 117,
    RLM_SYNC_ERR_CLIENT_PONG_TIMEOUT = 118,
    RLM_SYNC_ERR_CLIENT_BAD_CLIENT_FILE_IDENT_SALT = 119,
    RLM_SYNC_ERR_CLIENT_BAD_FILE_IDENT = 120,
    RLM_SYNC_ERR_CLIENT_CONNECT_TIMEOUT = 121,
    RLM_SYNC_ERR_CLIENT_BAD_TIMESTAMP = 122,
    RLM_SYNC_ERR_CLIENT_BAD_PROTOCOL_FROM_SERVER = 123,
    RLM_SYNC_ERR_CLIENT_CLIENT_TOO_OLD_FOR_SERVER = 124,
    RLM_SYNC_ERR_CLIENT_CLIENT_TOO_NEW_FOR_SERVER = 125,
    RLM_SYNC_ERR_CLIENT_PROTOCOL_MISMATCH = 126,
    RLM_SYNC_ERR_CLIENT_BAD_STATE_MESSAGE = 127,
    RLM_SYNC_ERR_CLIENT_MISSING_PROTOCOL_FEATURE = 128,
    RLM_SYNC_ERR_CLIENT_HTTP_TUNNEL_FAILED = 131,
    RLM_SYNC_ERR_CLIENT_AUTO_CLIENT_RESET_FAILURE = 132,
} realm_sync_errno_client_e;

typedef enum realm_sync_errno_connection {
    RLM_SYNC_ERR_CONNECTION_CONNECTION_CLOSED = 100,
    RLM_SYNC_ERR_CONNECTION_OTHER_ERROR = 101,
    RLM_SYNC_ERR_CONNECTION_UNKNOWN_MESSAGE = 102,
    RLM_SYNC_ERR_CONNECTION_BAD_SYNTAX = 103,
    RLM_SYNC_ERR_CONNECTION_LIMITS_EXCEEDED = 104,
    RLM_SYNC_ERR_CONNECTION_WRONG_PROTOCOL_VERSION = 105,
    RLM_SYNC_ERR_CONNECTION_BAD_SESSION_IDENT = 106,
    RLM_SYNC_ERR_CONNECTION_REUSE_OF_SESSION_IDENT = 107,
    RLM_SYNC_ERR_CONNECTION_BOUND_IN_OTHER_SESSION = 108,
    RLM_SYNC_ERR_CONNECTION_BAD_MESSAGE_ORDER = 109,
    RLM_SYNC_ERR_CONNECTION_BAD_DECOMPRESSION = 110,
    RLM_SYNC_ERR_CONNECTION_BAD_CHANGESET_HEADER_SYNTAX = 111,
    RLM_SYNC_ERR_CONNECTION_BAD_CHANGESET_SIZE = 112,
    RLM_SYNC_ERR_CONNECTION_SWITCH_TO_FLX_SYNC = 113,
    RLM_SYNC_ERR_CONNECTION_SWITCH_TO_PBS = 114,
} realm_sync_errno_connection_e;

typedef enum realm_sync_errno_session {
    RLM_SYNC_ERR_SESSION_SESSION_CLOSED = 200,
    RLM_SYNC_ERR_SESSION_OTHER_SESSION_ERROR = 201,
    RLM_SYNC_ERR_SESSION_TOKEN_EXPIRED = 202,
    RLM_SYNC_ERR_SESSION_BAD_AUTHENTICATION = 203,
    RLM_SYNC_ERR_SESSION_ILLEGAL_REALM_PATH = 204,
    RLM_SYNC_ERR_SESSION_NO_SUCH_REALM = 205,
    RLM_SYNC_ERR_SESSION_PERMISSION_DENIED = 206,
    RLM_SYNC_ERR_SESSION_BAD_SERVER_FILE_IDENT = 207,
    RLM_SYNC_ERR_SESSION_BAD_CLIENT_FILE_IDENT = 208,
    RLM_SYNC_ERR_SESSION_BAD_SERVER_VERSION = 209,
    RLM_SYNC_ERR_SESSION_BAD_CLIENT_VERSION = 210,
    RLM_SYNC_ERR_SESSION_DIVERGING_HISTORIES = 211,
    RLM_SYNC_ERR_SESSION_BAD_CHANGESET = 212,
    RLM_SYNC_ERR_SESSION_PARTIAL_SYNC_DISABLED = 214,
    RLM_SYNC_ERR_SESSION_UNSUPPORTED_SESSION_FEATURE = 215,
    RLM_SYNC_ERR_SESSION_BAD_ORIGIN_FILE_IDENT = 216,
    RLM_SYNC_ERR_SESSION_BAD_CLIENT_FILE = 217,
    RLM_SYNC_ERR_SESSION_SERVER_FILE_DELETED = 218,
    RLM_SYNC_ERR_SESSION_CLIENT_FILE_BLACKLISTED = 219,
    RLM_SYNC_ERR_SESSION_USER_BLACKLISTED = 220,
    RLM_SYNC_ERR_SESSION_TRANSACT_BEFORE_UPLOAD = 221,
    RLM_SYNC_ERR_SESSION_CLIENT_FILE_EXPIRED = 222,
    RLM_SYNC_ERR_SESSION_USER_MISMATCH = 223,
    RLM_SYNC_ERR_SESSION_TOO_MANY_SESSIONS = 224,
    RLM_SYNC_ERR_SESSION_INVALID_SCHEMA_CHANGE = 225,
    RLM_SYNC_ERR_SESSION_BAD_QUERY = 226,
    RLM_SYNC_ERR_SESSION_OBJECT_ALREADY_EXISTS = 227,
    RLM_SYNC_ERR_SESSION_SERVER_PERMISSIONS_CHANGED = 228,
    RLM_SYNC_ERR_SESSION_INITIAL_SYNC_NOT_COMPLETED = 229,
    RLM_SYNC_ERR_SESSION_WRITE_NOT_ALLOWED = 230,
} realm_sync_errno_session_e;

#endif /* REALM_ERROR_CODES_H */
