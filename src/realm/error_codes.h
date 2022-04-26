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

#define RLM_ERR_CAT_LOGIC 2
#define RLM_ERR_CAT_RUNTIME 4
#define RLM_ERR_CAT_INVALID_ARG 8
#define RLM_ERR_CAT_FILE_ACCESS 16
#define RLM_ERR_CAT_SYSTEM_ERROR 32

typedef enum realm_errno {
    RLM_ERR_NONE = 0,
    RLM_ERR_UNKNOWN,
    RLM_ERR_OTHER_EXCEPTION,
    RLM_ERR_OUT_OF_MEMORY,
    RLM_ERR_NOT_CLONABLE,

    RLM_ERR_WRONG_TRANSACTION_STATE,
    RLM_ERR_WRONG_THREAD,

    RLM_ERR_INVALIDATED_OBJECT,
    RLM_ERR_INVALID_PROPERTY,
    RLM_ERR_MISSING_PROPERTY_VALUE,
    RLM_ERR_PROPERTY_TYPE_MISMATCH,
    RLM_ERR_MISSING_PRIMARY_KEY,
    RLM_ERR_UNEXPECTED_PRIMARY_KEY,
    RLM_ERR_MODIFY_PRIMARY_KEY,
    RLM_ERR_READ_ONLY_PROPERTY,
    RLM_ERR_PROPERTY_NOT_NULLABLE,
    RLM_ERR_INVALID_ARGUMENT,
    RLM_ERR_INVALID_NAME,

    RLM_ERR_LOGIC,
    RLM_ERR_NO_SUCH_TABLE,
    RLM_ERR_NO_SUCH_OBJECT,
    RLM_ERR_CROSS_TABLE_LINK_TARGET,
    RLM_ERR_UNSUPPORTED_FILE_FORMAT_VERSION,
    RLM_ERR_FILE_FORMAT_UPGRADE_REQUIRED,
    RLM_ERR_MULTIPLE_SYNC_AGENTS,
    RLM_ERR_ADDRESS_SPACE_EXHAUSTED,
    RLM_ERR_MAXIMUM_FILE_SIZE_EXCEEDED,
    RLM_ERR_OUT_OF_DISK_SPACE,
    RLM_ERR_KEY_ALREADY_USED,
    RLM_ERR_SERIALIZATION_ERROR,
    RLM_ERR_OBJECT_ALREADY_EXISTS,
    RLM_ERR_DUPLICATE_PRIMARY_KEY_VALUE,

    RLM_ERR_TABLE_NAME_IN_USE,
    RLM_ERR_INVALID_TABLE_REF,
    RLM_ERR_BAD_CHANGESET,
    RLM_ERR_INVALID_DICTIONARY_KEY,
    RLM_ERR_INVALID_DICTIONARY_VALUE,
    RLM_ERR_STALE_ACCESSOR,
    RLM_ERR_INCOMPATIBLE_LOCK_FILE,
    RLM_ERR_INVALID_SORT_DESCRIPTOR,
    RLM_ERR_DECRYPTION_FAILED,
    RLM_ERR_INCOMPATIBLE_SESSION,
    RLM_ERR_BROKEN_INVARIANT,
    RLM_ERR_SUBSCRIPTION_FAILED,
    RLM_ERR_RANGE_ERROR,
    RLM_ERR_LIMIT_EXCEEDED,
    RLM_ERR_INVALID_DATABASE,
    RLM_ERR_INCOMPATIBLE_HISTORIES,

    RLM_ERR_INDEX_OUT_OF_BOUNDS,

    RLM_ERR_INVALID_QUERY_STRING,
    RLM_ERR_INVALID_QUERY_ARG,
    RLM_ERR_INVALID_QUERY,

    RLM_ERR_RUNTIME,

    RLM_ERR_FILE_OPERATION_FAILED,
    RLM_ERR_FILE_PERMISSION_DENIED,
    RLM_ERR_FILE_NOT_FOUND,
    RLM_ERR_FILE_ALREADY_EXISTS,

    RLM_ERR_SYSTEM_ERROR,

    RLM_ERR_DELETE_OPENED_REALM,

    RLM_ERR_BROKEN_PROMISE,
    RLM_ERR_ILLEGAL_OPERATION,
    RLM_ERR_READ_ONLY,
    REALM_ERR_NOT_SUPPORTED,

    RLM_ERR_CALLBACK = 1000000, /**< A user-provided callback failed. */
} realm_errno_e;


#endif /* REALM_ERROR_CODES_H */
