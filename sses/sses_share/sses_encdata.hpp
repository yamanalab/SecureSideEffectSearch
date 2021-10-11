/*
 * Copyright 2020  Yamana Laboratory, Waseda University
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

#ifndef SSES_ENCDATA_HPP
#define SSES_ENCDATA_HPP

#include <memory>
#include <vector>

#include <sses_share/sses_basicdata.hpp>

#include <FHE.h>
#include <EncryptedArray.h>

namespace sses_share
{

/**
 * @brief This class is used to hold the encrypted data.
 */
struct EncData : public sses_share::BasicData<Ctxt>
{
    /**
     * Constructor
     * @param[in] pubkey public key
     */
    explicit EncData(const FHEPubKey& pubkey);

    /**
     * Constructor
     * @param[in] pubkey public key
     * @param[in] ctxt ciphertext
     */
    EncData(const FHEPubKey& pubkey, const Ctxt& ctxt);
    
    /**
     * Constructor
     * @param[in] pubkey public key
     * @param[in] ctxts ciphertexts
     */
    EncData(const FHEPubKey& pubkey, const std::vector<Ctxt>& ctxts);

    virtual ~EncData(void) = default;

    /**
     * Encrypt value
     * @param[in] input_value input value
     * @param[in] pubkey public key
     */
    void encrypt(const int64_t input_value, const FHEPubKey& pubkey);

    /**
     * Save ciphertexts to stream
     * @param[out] os output stream
     * @return saved size (bytes)
     */
    virtual size_t save(std::ostream& os) const override;

    /**
     * Load ciphertexts from stream
     * @param[in] is input stream
     * @return loaded size (bytes)
     */
    virtual size_t load(std::istream& is) override;

    /**
     * Save ciphertexts to file
     * @param[in] filepath filepath
     */
    void save_to_file(const std::string& filepath) const;

    /**
     * Load ciphertexts from file
     * @param[in] filepath filepath
     */
    void load_from_file(const std::string& filepath);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace sses_share */

#endif /* SSES_ENCDATA_HPP */
