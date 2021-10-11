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

#include <iomanip> // for setw
#include <vector>

#include "FHE.h"
#include "EncryptedArray.h"

#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>

#include <sses_share/sses_encdata.hpp>
#include <sses_share/sses_utility.hpp>
#include <sses_share/sses_fhe_utility.hpp>

namespace sses_share
{

struct EncData::Impl
{
    explicit Impl(const FHEPubKey& pubkey)
        : pubkey_(pubkey)
    {}

    const FHEPubKey& pubkey_;
};

EncData::EncData(const FHEPubKey& pubkey)
    : pimpl_(new Impl(pubkey))
{
}

EncData::EncData(const FHEPubKey& pubkey, const Ctxt& ctxt)
    : pimpl_(new Impl(pubkey))
{
    vec_.push_back(ctxt);
}

EncData::EncData(const FHEPubKey& pubkey, const std::vector<Ctxt>& ctxts)
    : pimpl_(new Impl(pubkey))
{
    for (const auto& ctxt : ctxts) {
        vec_.push_back(ctxt);
    }
}
    
void EncData::encrypt(const int64_t input_value, const FHEPubKey& pubkey)
{
    Ctxt encdata(pubkey);
    pubkey.Encrypt(encdata, NTL::to_ZZX(input_value));
    vec_.push_back(encdata);
}

size_t EncData::save(std::ostream& os) const
{
    size_t sz = vec_.size();
    os.write(reinterpret_cast<char*>(&sz), sizeof(sz));

    std::ostringstream sz_counter;
    for (const auto& v : vec_)
    {
        os << v;
        sz_counter << v;
    }
    return sizeof(sz) + sz_counter.str().size();
}

size_t EncData::load(std::istream& is)
{
    size_t sz;
    is.read(reinterpret_cast<char*>(&sz), sizeof(sz));

    clear();

    // TODO: Return correct size
    for (size_t i = 0; i < sz; ++i)
    {
        Ctxt ctxt(pimpl_->pubkey_);
        is >> ctxt;
        vec_.push_back(ctxt);
    }
    return 0;
}

void EncData::save_to_file(const std::string& filepath) const
{
    std::ofstream ofs(filepath);
    save(ofs);
    ofs.close();
}

void EncData::load_from_file(const std::string& filepath)
{
    if (!sses_share::utility::file_exist(filepath))
    {
        std::ostringstream oss;
        oss << "File not found. (" << filepath << ")";
        STDSC_THROW_FILE(oss.str());
    }
    std::ifstream ifs(filepath, std::ios::binary);
    load(ifs);
    ifs.close();
}

} /* namespace sses_share */
