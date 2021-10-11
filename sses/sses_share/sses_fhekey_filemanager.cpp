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

#include <sstream>

#include "FHE.h"
#include "EncryptedArray.h"

#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <sses_share/sses_utility.hpp>
#include <sses_share/sses_fhekey_filemanager.hpp>

namespace sses_share
{

struct FHEKeyFileManager::Impl
{
    Impl(const std::string& pubkey_filepath,
         const std::string& seckey_filepath,
         const std::string& context_filepath)
      : pubkey_filepath_(pubkey_filepath),
        seckey_filepath_(seckey_filepath),
        context_filepath_(context_filepath)
    {
    }
    ~Impl(void) = default;

    void generate(const long p,
                  const long r,
                  const long L,
                  const long c,
                  const long w,
                  const long d,
                  const long security)
    {
        auto m = FindM(security, L, c, p, d, 0, 0);

        FHEcontext context(m, p, r);
        buildModChain(context, L, c);

        auto num_slots = context.zMStar.getNSlots();

        STDSC_LOG_INFO("Generating keys ... [security:%ld, m:%ld, Nslots:%ld]",
                       context.securityLevel(),
                       m,
                       num_slots);

        FHESecKey secretKey(context);
        const FHEPubKey& publicKey = secretKey;

        NTL::ZZX G = context.alMod.getFactorsOverZZ()[0];

        secretKey.GenSecKey(w);
        addSome1DMatrices(secretKey);

        std::ofstream fpk(pubkey_filepath_, std::ios::binary);
        fpk << publicKey << std::endl;
        fpk.close();

        std::ofstream fsk(seckey_filepath_, std::ios::binary);
        fsk << secretKey << std::endl;
        fsk.close();

        std::ofstream fctxt(context_filepath_, std::ios::binary);
        writeContextBase(fctxt, context);
        fctxt << context;
        fctxt.close();

        STDSC_LOG_INFO("Key generation was successful.");
        STDSC_LOG_INFO("  [pk: %s]", pubkey_filepath_.c_str());
        STDSC_LOG_INFO("  [sk: %s]", seckey_filepath_.c_str());
        STDSC_LOG_INFO("  [context: %s]", context_filepath_.c_str());
    }
    
    size_t pubkey_size(void) const
    {
        return sses_share::utility::file_size(pubkey_filepath_);
    }
    
    size_t seckey_size(void) const
    {
        return sses_share::utility::file_size(seckey_filepath_);
    }
    
    size_t context_size(void) const
    {
        return sses_share::utility::file_size(context_filepath_);
    }

    void pubkey_data(void* buffer)
    {
        size_t size = pubkey_size();
        std::ifstream ifs(pubkey_filepath_, std::ios::binary);
        if (!ifs.is_open())
        {
            std::ostringstream oss;
            oss << "failed to open. (" << pubkey_filepath_ << ")";
            STDSC_THROW_FILE(oss.str());
        }
        else
        {
            ifs.read(reinterpret_cast<char*>(buffer), size);
        }
        ifs.close();
    }

    void seckey_data(void* buffer)
    {
        size_t size = seckey_size();
        std::ifstream ifs(seckey_filepath_, std::ios::binary);
        if (!ifs.is_open())
        {
            std::ostringstream oss;
            oss << "failed to open. (" << seckey_filepath_ << ")";
            STDSC_THROW_FILE(oss.str());
        }
        else
        {
            ifs.read(reinterpret_cast<char*>(buffer), size);
        }
        ifs.close();
    }

    void context_data(void* buffer)
    {
        size_t size = context_size();
        std::ifstream ifs(context_filepath_, std::ios::binary);
        if (!ifs.is_open())
        {
            std::ostringstream oss;
            oss << "failed to open. (" << context_filepath_ << ")";
            STDSC_THROW_FILE(oss.str());
        }
        else
        {
            ifs.read(reinterpret_cast<char*>(buffer), size);
        }
        ifs.close();
    }

    bool is_exist_pubkey(void) const
    {
        std::ifstream ifs(pubkey_filepath_);
        return ifs.is_open();
    }

    bool is_exist_seckey(void) const
    {
        std::ifstream ifs(seckey_filepath_);
        return ifs.is_open();
    }
    
    bool is_exist_context(void) const
    {
        std::ifstream ifs(context_filepath_);
        return ifs.is_open();
    }

    std::string pubkey_filepath(void) const
    {
        return pubkey_filepath_;
    }

    std::string seckey_filepath(void) const
    {
        return seckey_filepath_;
    }
    
    std::string context_filepath(void) const
    {
        return context_filepath_;
    }
    
private:
    std::string pubkey_filepath_;
    std::string seckey_filepath_;
    std::string context_filepath_;
};

FHEKeyFileManager::FHEKeyFileManager(const std::string& pubkey_filepath,
                                           const std::string& seckey_filepath,
                                           const std::string& context_filepath)
    : pimpl_(new Impl(pubkey_filepath, seckey_filepath, context_filepath))
{
}

void FHEKeyFileManager::generate(const long p,
                                    const long r,
                                    const long L,
                                    const long c,
                                    const long w,
                                    const long d,
                                    const long security)
{
    pimpl_->generate(p, r, L, c, w, d, security);
}

size_t FHEKeyFileManager::pubkey_size(void) const
{
    return pimpl_->pubkey_size();
}

size_t FHEKeyFileManager::seckey_size(void) const
{
    return pimpl_->seckey_size();
}

void FHEKeyFileManager::pubkey_data(void* buffer)
{
    pimpl_->pubkey_data(buffer);
}

void FHEKeyFileManager::seckey_data(void* buffer)
{
    pimpl_->seckey_data(buffer);
}

void FHEKeyFileManager::context_data(void* buffer)
{
    pimpl_->context_data(buffer);
}

bool FHEKeyFileManager::is_exist_pubkey(void) const
{
    return pimpl_->is_exist_pubkey();
}

bool FHEKeyFileManager::is_exist_seckey(void) const
{
    return pimpl_->is_exist_seckey();
}

bool FHEKeyFileManager::is_exist_context(void) const
{
    return pimpl_->is_exist_context();
}

std::string FHEKeyFileManager::pubkey_filepath(void) const
{
    return pimpl_->pubkey_filepath();
}

std::string FHEKeyFileManager::seckey_filepath(void) const
{
    return pimpl_->seckey_filepath();
}
    
std::string FHEKeyFileManager::context_filepath(void) const
{
    return pimpl_->context_filepath();
}

} /* namespace sses_share */
