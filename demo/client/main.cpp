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

#include <unistd.h>
#include <iostream>
#include <memory>
#include <string>

#include "FHE.h"
#include "EncryptedArray.h"

#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_utility.hpp>

#include <sses_share/sses_fhekey_container.hpp>
#include <sses_share/sses_computation_param.hpp>
#include <sses_share/sses_encdata.hpp>
#include <sses_share/sses_mask.hpp>
#include <sses_client/sses_client_record.hpp>
#include <sses_client/sses_client.hpp>

#include <share/define.hpp>

#include "picojson_wrapper.h"

#define SETTINGS_DIR "../../../settings"
#define DEFAULT_CONFFILE "contextsetting.txt"
#define DEFAULT_HOSTNAME "localhost"

struct Option
{
    std::string hostname = DEFAULT_HOSTNAME;
    std::string port     = PORT_SRV;
    std::string conffile = SETTINGS_DIR "/" DEFAULT_CONFFILE;
    size_t fhe_key_id    = 0;
    size_t age;
    std::string gender;
    std::string meds;
    std::string sides;
};

struct CallbackParam
{
};

void callback_func(const int32_t query_id, const bool status,
                   const std::vector<sses_client::Record>& records, void* args)
{
    STDSC_LOG_INFO("\nCallback function for query #%d", query_id);

#ifdef ENABLE_LOCAL_DEBUG
    printf("records (%lu) : \n", records.size());
    for (size_t i=0; i<records.size(); ++i) {
        
        const auto& r = records[i];
        
        printf("  [%lu] id:%d, ", i, r.id_);
        
        printf("medicinIds: ");
        for (const auto& id : r.medicinIds_) {
            printf("%d ", id);
        }
        printf(", ");

        printf("symptomIds: ");
        for (const auto& id : r.symptomIds_) {
            printf("%d ", id);
        }
        printf("\n");
    }
#endif /*ENABLE_LOCAL_DEBUG*/

    STDSC_LOG_INFO("Total record number hit: %lu.", records.size());
    
    Json json;
    json.add<sses_client::Record>("records", records,
                                  [](sses_client::Record r)
                                  {
                                      Json j;
                                      j.add("id", r.id_);
                                      j.add("medicine_ids", r.medicinIds_);
                                      j.add("symptom_ids", r.symptomIds_);
                                      return j;
                                  });

    std::cout << json.serialize() << std::endl;
    
}

static void print_usage_and_exit(const char* progname)
{
    printf(
        "Usage: %s [-i IP Address] [-p PORT] [-c FHE context setting file] [-k FHE key ID] "
        "age gender list-of-query-medicines query-side-effects\n",
        progname);
    exit(1);
}

static void print_option(const Option& option)
{
    printf("Client options: hostname:%s, port:%s, conf:%s, "
           "key_id:%lu, age:%lu, gender:%s, meds:%s, sides:%s\n",
           option.hostname.c_str(),
           option.port.c_str(),
           option.conffile.c_str(),
           option.fhe_key_id,
           option.age,
           option.gender.c_str(),
           option.meds.c_str(),
           option.sides.c_str());
}

static void init(Option& option, int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "i:p:c:k:h")) != -1)
    {
        switch (opt)
        {
            case 'i':
                option.hostname = optarg;
                break;
            case 'p':
                option.port = optarg;
                break;
            case 'c':
                option.conffile = optarg;
                break;
            case 'k':
                option.fhe_key_id = std::stoi(optarg);
                break;
            case 'h':
            default:
                print_usage_and_exit(argv[0]);
        }
    }

    if ((argc - optind) < 4) {
        print_usage_and_exit(argv[0]);
    }
    
    option.age = std::stoi(argv[optind + 0]);
    option.gender = argv[optind + 1];
    option.meds = argv[optind + 2];
    option.sides = argv[optind + 3];

    print_option(option);
}

static void exec(const Option& option)
{
    int32_t key_id = option.fhe_key_id;
    sses_share::FHEKeyContainer keycont(SETTINGS_DIR);
    if (key_id) {
        const bool enable_file_check = true;
        keycont.setup(key_id, enable_file_check);
    } else {
        key_id = keycont.create(option.conffile);
    }

    auto context = keycont.get_context(key_id);
    FHEPubKey pubkey(context);
    keycont.get(key_id, sses_share::KeyKind_t::kKindPubKey, pubkey);
    FHESecKey seckey(context);
    keycont.get(key_id, sses_share::KeyKind_t::kKindSecKey, seckey);

    STDSC_LOG_INFO("Prepared encryption keys. (key_id:%d)", key_id);

    sses_client::Client client(option.hostname.c_str(), option.port.c_str(), context, pubkey, seckey);
    client.connect();
    STDSC_LOG_INFO("Connected to %s:%s", option.hostname.c_str(), option.port.c_str());

    client.register_enckeys(key_id,
                            keycont.filepath(key_id, sses_share::KeyKind_t::kKindContext),
                            keycont.filepath(key_id, sses_share::KeyKind_t::kKindPubKey));

    auto mask = sses_share::sses_mask_compute_mask(option.age, option.gender);

    sses_share::EncData encmask(pubkey);
    encmask.encrypt(mask, pubkey);

    CallbackParam callback_param;
    auto queryID = client.send_query(key_id,
                      option.age,
                      option.gender,
                      option.meds,
                      option.sides,
                      encmask,
                      callback_func, &callback_param);

    client.wait(queryID);
}

int main(int argc, char* argv[])
{
    STDSC_INIT_LOG();
    try
    {
        Option option;
        init(option, argc, argv);
        STDSC_LOG_INFO("Launched Client demo app.");
        exec(option);
    }
    catch (stdsc::AbstractException& e)
    {
        STDSC_LOG_ERR("Err: %s", e.what());
    }
    catch (...)
    {
        STDSC_LOG_ERR("Catch unknown exception");
    }

    return 0;
}
