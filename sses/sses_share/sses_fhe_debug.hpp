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

#ifndef SSES_SEAL_DEBUG_HPP
#define SSES_SEAL_DEBUG_HPP

#include <iostream>
#include <string>
#include <vector>

#include "FHE.h"
#include "EncryptedArray.h"

#include <sses_share/sses_utility.hpp>

#define MYDBG_DECRYPT(encdata) do {                     \
    sses_share::fhe_debug::decrypt_and_print((encdata), \
        "../../../settings/ctxt_1234.bin",              \
        "../../../settings/pk_1234.bin",                \
        "../../../settings/sk_1234.bin",                \
        5);                                             \
} while(0)

namespace sses_share
{

namespace fhe_debug
{

static void decrypt_and_print(const std::string& encdata_filepath,
                              const std::string& context_filepath,
                              const std::string& pubkey_filepath,
                              const std::string& seckey_filepath,
                              const size_t max_print_num = 0)
{
    unsigned long m, p, r;
    std::vector<long> gens, ords;

    std::ifstream ifs_ctxt(context_filepath, std::ios::binary);
    readContextBase(ifs_ctxt, m, p, r, gens, ords);
    FHEcontext context(m, p, r, gens, ords);
    ifs_ctxt >> context;
    ifs_ctxt.close();
                
    FHESecKey seckey(context);
    {
        std::ifstream ifs(seckey_filepath, std::ios::binary);
        ifs >> seckey;
    }

    FHEPubKey pubkey(context);
    {
        std::ifstream ifs(pubkey_filepath, std::ios::binary);
        ifs >> pubkey;
    }
    
    Ctxt encdata(pubkey);
    {
        std::ifstream ifs(encdata_filepath, std::ios::binary);
        ifs >> encdata;
    }
    
    NTL::ZZX G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);

    std::vector<long> decrypted_data;
    ea.decrypt(encdata, seckey, decrypted_data);

#define MIN(a,b) ((a) < (b)) ? (a) : (b)
    auto n = (max_print_num > 0) ? (MIN(max_print_num, decrypted_data.size())) : decrypted_data.size();
#undef MIN
    
    printf("[DBG] Decrypted (%lu) : ", decrypted_data.size());
    for (size_t i=0; i<n; ++i) {
        printf("%ld ", decrypted_data[i]);
    }
    printf("\n");
}

static void decrypt_and_print(const Ctxt& encdata,
                              const std::string& context_filepath,
                              const std::string& pubkey_filepath,
                              const std::string& seckey_filepath,
                              const size_t max_print_num = 0)
{
    std::string tmpfilepath = "tmp.bin";
    try {
        std::ofstream ofs(tmpfilepath, std::ios::binary);
        ofs << encdata;
        ofs.close();
        
        decrypt_and_print(tmpfilepath,
                          context_filepath,
                          pubkey_filepath,
                          seckey_filepath,
                          max_print_num);

        utility::remove_file(tmpfilepath);
    } catch(const std::exception& ex) {
        utility::remove_file(tmpfilepath);
    }
}

} /* namespace fhe_debug */

} /* namespace sses_share */

#endif /* SSES_SEAL_DEBUG_HPP */
