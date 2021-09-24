/*
 * Copyright 2018 Yamana Laboratory, Waseda University
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

#ifndef SSES_FHEKEYKEY_FILEMANAGER
#define SSES_FHEKEYKEY_FILEMANAGER

#include <memory>

namespace sses_share
{

/**
 * @brief Manages secure key files. (public / secret key files)
 */
class FHEKeyFileManager
{
    static constexpr long DefaultFheM = 32768;
    static constexpr long DefaultFheP = 257;
    static constexpr long DefaultFheR = 1;
    static constexpr long DefaultFheL = 11;
    static constexpr long DefaultFheC = 3;
    static constexpr long DefaultFheW = 64;
    static constexpr long DefaultFheD = 0;
    static constexpr long DefaultFheSecurity = 128;
    
public:
    /**
     * Constructor
     * @param[in] pubkey_filepath public key filepath
     * @param[in] seckey_filepath secret key filepath
     * @param[in] context_filepath context filepath
     */
    FHEKeyFileManager(const std::string& pubkey_filepath,
                      const std::string& seckey_filepath,
                      const std::string& context_filepath);
    ~FHEKeyFileManager(void) = default;

    /**
     * Generate FHE keys
     * @param[in] p FHE p
     * @param[in] r FHE r
     * @param[in] L FHE L
     * @param[in] c FHE c
     * @param[in] w FHE w
     * @param[in] d FHE d
     * @param[in] security FHE security
     */
    void generate(const long p,
                  const long r,
                  const long L,
                  const long c,
                  const long w,
                  const long d,
                  const long security);

    /**
     * Return public key size
     * @return public key size
     */
    size_t pubkey_size(void) const;
    
    /**
     * Return secret key size
     * @return secret key size
     */
    size_t seckey_size(void) const;

    /**
     * Return context size
     * @return context size
     */
    size_t context_size(void) const;

    /**
     * Return public key data
     * @param[out] buffer public key data
     */
    void pubkey_data(void* buffer);
    
    /**
     * Return secret key data
     * @param[out] buffer secret key data
     */
    void seckey_data(void* buffer);
    
    /**
     * Return context data
     * @param[out] buffer context data
     */
    void context_data(void* buffer);

    /**
     * Whether the public key exists or not
     * @return exists or not
     */
    bool is_exist_pubkey(void) const;
    
    /**
     * Whether the secret key exists or not
     * @return exists or not
     */
    bool is_exist_seckey(void) const;
    
    /**
     * Whether the context exists or not
     * @return exists or not
     */
    bool is_exist_context(void) const;

    /**
     * Filepath of public key
     * @return filepath
     */
    std::string pubkey_filepath(void) const;

    /**
     * Filepath of secret key
     * @return filepath
     */
    std::string seckey_filepath(void) const;

    /**
     * Filepath of context key
     * @return filepath
     */
    std::string context_filepath(void) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace sses_share */

#endif /* SSES_FHEKEYKEY_FILEMANAGER */
