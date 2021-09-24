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
#include <vector>
#include <iostream>

#include "EncryptedArray.h"
#include "FHE.h"

#include <stdsc/stdsc_buffer.hpp>

#include <sses_share/sses_encdata.hpp>
#include <sses_share/sses_fhectxt_buffer.hpp>

namespace sses_share
{

struct FHECtxtBuffer::Impl
{
    Impl()
    {}

    void serialize(const FHEPubKey& pubkey, const std::vector<Ctxt>& ctxts)
    {
        vec_.clear();

        EncData encdata(pubkey, ctxts);
        
        stdsc::BufferStream bs(encdata.stream_size());
        std::iostream ios(&bs);
        encdata.save(ios);

        stdsc::Buffer& buffer = bs;
        vec_.resize(buffer.size());
        std::memcpy(vec_.data(), buffer.data(), buffer.size());
    }

    void deserialize(const FHEPubKey& pubkey, std::vector<Ctxt>& ctxts) const
    {
        stdsc::BufferStream bs(vec_.size());
        stdsc::Buffer& buffer = bs;
        std::memcpy(bs.data(), vec_.data(), vec_.size());

        std::iostream ios(&bs);
        
        EncData encdata(pubkey);
        encdata.load(ios);

        for (const auto& ctxt : encdata.vdata()) {
            ctxts.push_back(ctxt);
        }
    }

    std::vector<uint8_t> vec_;
};

FHECtxtBuffer::FHECtxtBuffer()
    : pimpl_(new Impl())
{}

void FHECtxtBuffer::serialize(const FHEPubKey& pubkey, const std::vector<Ctxt>& ctxts)
{
    pimpl_->serialize(pubkey, ctxts);
}
    
void FHECtxtBuffer::deserialize(const FHEPubKey& pubkey, std::vector<Ctxt>& ctxts) const
{
    pimpl_->deserialize(pubkey, ctxts);
}

} /* namespace sses_share */
