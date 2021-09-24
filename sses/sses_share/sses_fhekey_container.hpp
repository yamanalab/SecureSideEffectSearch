/*
 * Copyright 2020 Yamana Laboratory, Waseda University
 * Supported by JST CREST Grant Number JPMJCR1503, Japan.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE‐2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SSES_FHEKEY_CONTAINER_HPP
#define SSES_FHEKEY_CONTAINER_HPP

#include <memory>

#include "FHE.h"
#include "EncryptedArray.h"

//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#include <sses_share/sses_fhe_debug.hpp>
#endif

namespace sses_share
{

enum KeyKind_t : int32_t
{
    kKindUnknown = -1,
    kKindPubKey = 0,
    kKindSecKey = 1,
    kKindContext = 2,
    kNumOfKind,
};

/**
 * @brief This class is used to hold the SEAL keys.
 */
struct FHEKeyContainer
{
    /**
     * Constructor
     * @param[in] dir directory to manage files
     */
    explicit FHEKeyContainer(const char* dir = ".");
    virtual ~FHEKeyContainer() = default;

    /**
     * Generate new keys.
     * @param[in] power power
     * @param[in] level level
     * @return key ID
     */
    int32_t create(const std::string& conffile);

    /**
     * Load keys.
     * @param[in] key ID
     */
    void setup(const int32_t key_id, const bool enable_file_check=false);

    /**
     * Delete keys.
     * @param[in] key_id key ID
     */
    void remove(const int32_t key_id);

    /**
     * get context.
     * @param[in] key_id key ID
     */
    FHEcontext get_context(const int32_t key_id) const;
    
    /**
     * get keys.
     * @param[in] key_id key ID
     * @param[in] kind key kind
     * @param[out] data key data
     */
    template <class T>
    void get(const int32_t key_id, const KeyKind_t kind, T& data) const;

    /**
     * get data size
     * @param[in] key_id key ID
     * @param[in] kind key kind
     */
    size_t data_size(const int32_t key_id, const KeyKind_t kind) const;

    /**
     * get filepath
     * @param[in] key_id key ID
     * @param[in] kind key kind
     */
    const std::string filepath(const int32_t key_id, const KeyKind_t kind) const;

#ifdef ENABLE_DEBUG
    void decrypt_and_print(const int32_t key_id,
                           const Ctxt& encdata,
                           const size_t max_print_num = 0)
    {
        auto context_filepath = filepath(key_id, KeyKind_t::kKindContext);
        auto pubkey_filepath = filepath(key_id, KeyKind_t::kKindPubKey);
        auto seckey_filepath = filepath(key_id, KeyKind_t::kKindSecKey);

        fhe_debug::decrypt_and_print(encdata,
                                     context_filepath,
                                     pubkey_filepath,
                                     seckey_filepath,
                                     max_print_num);
    }

    void decrypt_and_print(const int32_t key_id,
                           const std::string& encdata_filepath,
                           const size_t max_print_num = 0)
    {
        auto context_filepath = filepath(key_id, KeyKind_t::kKindContext);
        auto pubkey_filepath = filepath(key_id, KeyKind_t::kKindPubKey);
        auto seckey_filepath = filepath(key_id, KeyKind_t::kKindSecKey);

        fhe_debug::decrypt_and_print(encdata_filepath,
                                     context_filepath,
                                     pubkey_filepath,
                                     seckey_filepath,
                                     max_print_num);
    }
    
#endif
    
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace sses_share */

#endif /* SSES_FHEKEY_CONTAINER_HPP */
