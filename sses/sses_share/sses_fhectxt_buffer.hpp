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

#ifndef SSES_FHECTXT_BUFFER_HPP
#define SSES_FHECTXT_BUFFER_HPP

#include <memory>
#include <vector>

class Ctxt;

namespace sses_share
{

/**
 * @brief This clas is used to hold the FHE Ctxt.
 */
class FHECtxtBuffer
{
public:
    explicit FHECtxtBuffer(void);
    virtual ~FHECtxtBuffer(void) = default;

    /**
     * Serialize
     * @param[in] pubkey FHE public key
     * @param[in] ctxt ctxt
     */
    void serialize(const FHEPubKey& pubkey, const std::vector<Ctxt>& ctxts);

    /**
     * Deserialize
     * @param[in] pubkey FHE public key
     * @param[out] ctxt ctxt
     */
    void deserialize(const FHEPubKey& pubkey, std::vector<Ctxt>& ctxts) const;
    
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace sses_share */

#endif /* SSES_FHECTXT_BUFFER_HPP */
