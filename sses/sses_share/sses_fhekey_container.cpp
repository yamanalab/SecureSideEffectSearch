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
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "EncryptedArray.h"
#include "FHE.h"

#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>

#include <sses_share/sses_utility.hpp>
#include <sses_share/sses_fhekey_filemanager.hpp>
#include <sses_share/sses_fhekey_container.hpp>

#define CHECK_KIND(k)                                                    \
    do                                                                   \
    {                                                                    \
        if (!((k) < kNumOfKind))                                         \
        {                                                                \
            std::ostringstream oss;                                      \
            oss << "Err: Invalid securekey kind. (kind: " << (k) << ")"; \
            STDSC_THROW_INVPARAM(oss.str().c_str());                     \
        }                                                                \
    } while (0)

namespace sses_share
{

struct FHEKeyContainer::Impl
{
    struct KeyFilepathes
    {
        KeyFilepathes(const int32_t id, const char* dir = ".")
        {
            std::ostringstream oss;
            oss << dir << "/";
            filepathes_.emplace(kKindPubKey,
                                oss.str() + std::string("pk_") + std::to_string(id) + std::string(".bin"));
            filepathes_.emplace(kKindSecKey,
                                oss.str() + std::string("sk_") + std::to_string(id) + std::string(".bin"));
            filepathes_.emplace(kKindContext,
                                oss.str() + std::string("ctxt_") + std::to_string(id) + std::string(".bin"));
        }

        std::string filepath(const KeyKind_t kind) const
        {
            CHECK_KIND(kind);
            return filepathes_.at(kind);
        }

        std::unordered_map<KeyKind_t, std::string> filepathes_;
    };

    Impl(const char* dir)
        : dir_(dir)
    {
    }

    int32_t create(const std::string& conffile)
    {
        int32_t key_id = sses_share::utility::gen_uuid();
        map_.emplace(key_id, KeyFilepathes(key_id, dir_.c_str()));

        if (!sses_share::utility::file_exist(conffile))
        {
            std::ostringstream oss;
            oss << "File is not found. (" << conffile << ")";
            STDSC_THROW_FILE(oss.str());
        }

        long p, r, L, c, w, d, security;
        {
            std::ifstream fin(conffile);
            fin >> p >> r >> L >> c >> w >> d >> security;
        }
        
        const KeyFilepathes& filepathes = map_.at(key_id);
        std::shared_ptr<sses_share::FHEKeyFileManager> skm(
            new sses_share::FHEKeyFileManager(filepathes.filepath(KeyKind_t::kKindPubKey),
                                              filepathes.filepath(KeyKind_t::kKindSecKey),
                                              filepathes.filepath(KeyKind_t::kKindContext)));
        skm->generate(p, r, L, c, w, d, security);
        
        return key_id;
    }

    void setup_keys(const int32_t key_id, const bool enable_file_check)
    {
        map_.emplace(key_id, KeyFilepathes(key_id, dir_.c_str()));
        
        const KeyFilepathes& filepathes = map_.at(key_id);

        if (enable_file_check) {
            int32_t bgn = static_cast<int32_t>(KeyKind_t::kKindPubKey);
            int32_t end = static_cast<int32_t>(KeyKind_t::kNumOfKind);

            for (auto i = bgn; i < end; ++i)
            {
                const auto key = static_cast<KeyKind_t>(i);
                const auto& filepath = filepathes.filepath(key);
                auto ret = sses_share::utility::file_exist(filepath);
                if (!ret)
                {
                    map_.erase(key_id);
                    std::ostringstream oss;
                    oss << "File does not exist. (" << filepath << ")";
                    STDSC_THROW_FILE(oss.str());
                }
            }
        }
    }
    
    void remove(const int32_t key_id)
    {
        remove_keyfiles(map_.at(key_id));
        map_.erase(key_id);
    }

    FHEcontext get_context(const int32_t key_id) const
    {
        unsigned long m, p, r;
        std::vector<long> gens, ords;
                    
        std::ifstream ifs(filepath(key_id, kKindContext), std::ios::binary);
        readContextBase(ifs, m, p, r, gens, ords);
        
        FHEcontext context(m, p, r, gens, ords);
        ifs >> context;
        ifs.close();

        return context;
    }
    
    template <class T>
    void get(const int32_t key_id, const KeyKind_t kind, T& data) const
    {
        CHECK_KIND(kind);
        std::ifstream ifs(filepath(key_id, kind), std::ios::binary);
        ifs >> data;
        ifs.close();
    }
    
    size_t data_size(const int32_t key_id, const KeyKind_t kind) const
    {
        const auto& filepath = map_.at(key_id).filepath(kind);
        if (!sses_share::utility::file_exist(filepath))
        {
            std::ostringstream oss;
            oss << "File is not found. (" << filepath << ")";
            STDSC_THROW_FILE(oss.str());
        }
        return sses_share::utility::file_size(filepath);
    }

    const std::string filepath(const int32_t key_id, const KeyKind_t kind) const
    {
        CHECK_KIND(kind);
        
        if (map_.count(key_id) == 0) {
            std::ostringstream oss;
            oss << "Err: keyID " << key_id;
            oss << "information not found.";
            STDSC_THROW_FAILURE(oss.str().c_str());
        }
        
        const auto& filepath = map_.at(key_id).filepath(kind);
        return filepath.c_str();
    }
    
private:

    void remove_keyfiles(const KeyFilepathes& filepathes)
    {
        int32_t bgn = static_cast<int32_t>(KeyKind_t::kKindPubKey);
        int32_t end = static_cast<int32_t>(KeyKind_t::kNumOfKind);

        for (auto i = bgn; i < end; ++i)
        {
            const auto key = static_cast<KeyKind_t>(i);
            const auto& filepath = filepathes.filepath(key);
            auto ret = sses_share::utility::remove_file(filepath);
            if (!ret)
            {
                std::ostringstream oss;
                oss << "Failed to remove file. (" << filepath << ")";
                STDSC_THROW_FILE(oss.str());
            }
        }
    }

private:
    const std::string dir_;
    std::unordered_map<int32_t, KeyFilepathes> map_;
};

FHEKeyContainer::FHEKeyContainer(const char* dir)
    : pimpl_(new Impl(dir))
{
}

int32_t FHEKeyContainer::create(const std::string& conffile)
{
    auto key_id = pimpl_->create(conffile);
    STDSC_LOG_INFO("Generated new keys. (key ID: %d)", key_id);
    return key_id;
}

void FHEKeyContainer::setup(const int32_t key_id, const bool enable_file_check)
{
    STDSC_LOG_DEBUG("Load keys. (key ID: %d)", key_id);
    pimpl_->setup_keys(key_id, enable_file_check);
}

void FHEKeyContainer::remove(const int32_t key_id)
{
    pimpl_->remove(key_id);
    STDSC_LOG_DEBUG("Deleted key #d.", key_id);
}

FHEcontext FHEKeyContainer::get_context(const int32_t key_id) const
{
    return pimpl_->get_context(key_id);
}

#define DEF_GET_WITH_TYPE(type, name)                                      \
    template <>                                                            \
    void FHEKeyContainer::get(const int32_t key_id, const KeyKind_t kind,  \
                           type& data) const                               \
    {                                                                      \
        STDSC_LOG_DEBUG("Get keys. (key ID: %d, kind: %s)", key_id, name); \
        pimpl_->get(key_id, kind, data);                                   \
    }

DEF_GET_WITH_TYPE(FHEPubKey, "public key");
DEF_GET_WITH_TYPE(FHESecKey, "secret key");

#undef DEF_GET_WITH_TYPE

size_t FHEKeyContainer::data_size(const int32_t key_id, const KeyKind_t kind) const
{
    return pimpl_->data_size(key_id, kind);
}

const std::string FHEKeyContainer::filepath(const int32_t key_id, const KeyKind_t kind) const
{
    return pimpl_->filepath(key_id, kind);
}

} /* namespace sses_share */
