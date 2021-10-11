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

#include <cstring>
#include <fstream>
#include <memory>
#include <vector>

#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_client.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_packet.hpp>

#include <sses_share/sses_utility.hpp>
#include <sses_share/sses_cli2srvparam.hpp>
#include <sses_share/sses_srv2cliparam.hpp>
#include <sses_share/sses_plaindata.hpp>
#include <sses_share/sses_packet.hpp>
#include <sses_share/sses_fhe_utility.hpp>
#include <sses_share/sses_encdata.hpp>
#include <sses_client/sses_client.hpp>
#include <sses_client/sses_client_result_thread.hpp>
#include <sses_client/sses_client_record.hpp>

//#define ENABLE_LOCAL_DEBUG
#ifdef ENABLE_LOCAL_DEBUG
#include <sses_share/sses_fhe_debug.hpp>
#endif

namespace sses_client
{

struct ResultCallback
{
    std::shared_ptr<ResultThread> thread;
    ResultThreadParam param;
};

struct Client::Impl
{
    Impl(const char* host, const char* port,
         FHEcontext& context,
         FHEPubKey& pubkey,
         FHESecKey& seckey)
        : host_(host),
          port_(port),
          context_(context),
          pubkey_(pubkey),
          seckey_(seckey),
          client_()
    {
    }

    ~Impl(void)
    {
        disconnect();
    }

    void connect(const uint32_t retry_interval_usec, const uint32_t timeout_sec)
    {
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
    }

    void disconnect(void)
    {
        client_.close();
    }

    void register_enckeys(const int32_t key_id,
                          const std::string& context_filepath,
                          const std::string& pubkey_filepath)
    {
        SSES_UTILITY_NEWLINE;
        STDSC_LOG_INFO("Start encryption key registration.");
        sses_share::PlainData<sses_share::C2SEnckeyParam> splaindata;
        sses_share::C2SEnckeyParam c2s_param;
        c2s_param.key_id = key_id;
        c2s_param.context_stream_sz = sses_share::utility::file_size(context_filepath);
        c2s_param.pubkey_stream_sz = sses_share::utility::file_size(pubkey_filepath);
        splaindata.push(c2s_param);

        auto sz = (splaindata.stream_size()
                   + c2s_param.context_stream_sz
                   + c2s_param.pubkey_stream_sz);
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save(stream);
        sses_share::fhe_utility::write_to_binary_stream(
            stream, sbuffstream.data(), context_filepath);
        sses_share::fhe_utility::write_to_binary_stream(
            stream, sbuffstream.data(), pubkey_filepath);

        stdsc::Buffer* sbuffer = &sbuffstream;
        client_.send_data_blocking(sses_share::kControlCodeDataEncKeys, *sbuffer);

        STDSC_LOG_INFO("Finish encryption key registration. "
                       "[keyID:%d, context_sz:%ld, pubkey_sz:%ld]",
                       key_id, c2s_param.context_stream_sz, c2s_param.pubkey_stream_sz);
    }
    
    int32_t send_query(const int32_t key_id,
                       const size_t age,
                       const std::string& gender,
                       const std::string& meds,
                       const std::string& sides,
                       const sses_share::EncData& encdata)
    {
        SSES_UTILITY_NEWLINE;
        STDSC_LOG_INFO("Start sending query.");

        sses_share::ComputationParam param;
        param.age = age;
        std::strncpy(param.gender, gender.c_str(), SSES_DEFAULT_SIZE_OF_STR_FOR_TRANSDATA);
        std::strncpy(param.meds, meds.c_str(), SSES_DEFAULT_SIZE_OF_STR_FOR_TRANSDATA);
        std::strncpy(param.sides, sides.c_str(), SSES_DEFAULT_SIZE_OF_STR_FOR_TRANSDATA);
        

        STDSC_LOG_INFO("Setup param for Query. [age:%lu, gender:%s, meds:%s, sides:%s]",
                        param.age,
                        param.gender,
                        param.meds,
                        param.sides);
        
        sses_share::PlainData<sses_share::C2SQueryParam> splaindata;
        sses_share::C2SQueryParam c2s_param;
        c2s_param.comp_param = param;
        c2s_param.key_id = key_id;
        c2s_param.encdata_stream_sz = encdata.stream_size();
        splaindata.push(c2s_param);
    
        auto sz = splaindata.stream_size() + c2s_param.encdata_stream_sz;
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);
    
        splaindata.save(stream);
        encdata.save(stream);
    
        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(
          sses_share::kControlCodeUpDownloadQuery, *sbuffer, rbuffer);
    
        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);
        sses_share::PlainData<int32_t> rplaindata;
        rplaindata.load(rstream);

        STDSC_LOG_INFO("Finish sending query.");
        return rplaindata.data();
    }
    
    void recv_results(const int32_t query_id, bool& status, std::vector<Record>& records)
    {
        SSES_UTILITY_NEWLINE;

        STDSC_LOG_INFO("Start subscribing to the results of each chunk.");

        sses_share::PlainData<sses_share::C2SChunkResreqParam> splaindata;
        sses_share::C2SChunkResreqParam c2s_param;
        c2s_param.query_id = query_id;
        splaindata.push(c2s_param);
    
        auto sz = splaindata.stream_size();
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);
    
        splaindata.save(stream);
    
        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(
          sses_share::kControlCodeUpDownloadChunkResult, *sbuffer, rbuffer);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);
    
        sses_share::PlainData<sses_share::S2CChunkResultParam> rplaindata;
        rplaindata.load(rstream);
        auto& s2c_param = rplaindata.data();

        sses_share::EncData enc_data(pubkey_);
        enc_data.load(rstream);
        
        STDSC_LOG_INFO("Received result of each chunk for queryID %d. [status:%d, Nresults:%lu]",
                       query_id, s2c_param.status, enc_data.vdata().size());
        
        if (s2c_param.status == sses_share::kServerResultStatusSuccess)
        {
            STDSC_LOG_INFO("Start proccesing result of each chunk for queryID %d.", query_id);
            
            auto& chunk_res = enc_data.vdata();

#ifdef ENABLE_LOCAL_DEBUG
            STDSC_LOG_INFO("[DBG] Num of chunk_res (%lu) : ", chunk_res.size());
            for (auto& ctxt : chunk_res) {
                MYDBG_DECRYPT(ctxt);
            }
#endif
            NTL::ZZX G = context_.alMod.getFactorsOverZZ()[0];
            EncryptedArray ea(context_, G);

            std::vector<std::pair<int, int>> ret;
            for (size_t i = 0; i < chunk_res.size(); ++i)
            {
                std::vector<long> decrypted;
                ea.decrypt(chunk_res[i], seckey_, decrypted);

#ifdef ENABLE_LOCAL_DEBUG
                printf("[DBG] Num of decrypted chunk_res (%lu) : ", decrypted.size());
                for (size_t j=0; j<5; ++j) {
                    printf("%ld ", decrypted[j]);
                }
                printf("\n");
#endif

                for (size_t j = 0; j < decrypted.size(); ++j) {
                    if (decrypted[j] == 0) {
                        ret.push_back(std::make_pair(i, j));
                    }
                }
            }

            STDSC_LOG_INFO("Decrypt the result. find 0's inside. [N:%lu]", ret.size());

            STDSC_LOG_INFO("Finish proccesing result of each chunk for queryID %d.", query_id);

            
            STDSC_LOG_INFO("Start subscribing to the results.");            

            sses_share::PlainData<sses_share::C2SResreqParam> splaindata_param;
            sses_share::C2SResreqParam c2s_param;
            c2s_param.query_id = query_id;
            c2s_param.key_id = s2c_param.key_id;
            splaindata_param.push(c2s_param);
            
            sses_share::PlainData<sses_share::C2SSelectedInfo> splaindata_selinfo;
            for (const auto& pair : ret) {
                sses_share::C2SSelectedInfo c2s_selinfo;
                c2s_selinfo.chunk_id = pair.first;
                c2s_selinfo.pos_id = pair.second;
                splaindata_selinfo.push(c2s_selinfo);
            }

#ifdef ENABLE_LOCAL_DEBUG
            printf("[DBG] selinfo (%lu) : ", splaindata_selinfo.vdata().size());
            for (const auto& v : splaindata_selinfo.vdata()) {
                printf("(%d,%d) ", v.chunk_id, v.pos_id);
            }
            printf("\n");
#endif
            
            auto sz = splaindata_param.stream_size() + splaindata_selinfo.stream_size();
            stdsc::BufferStream sbuffstream(sz);
            std::iostream stream(&sbuffstream);
            
            splaindata_param.save(stream);
            splaindata_selinfo.save(stream);
            
            stdsc::Buffer* sbuffer = &sbuffstream;
            stdsc::Buffer rbuffer;
            client_.send_recv_data_blocking(
                sses_share::kControlCodeUpDownloadResult, *sbuffer, rbuffer);

            stdsc::BufferStream rbuffstream(rbuffer);
            std::iostream rstream(&rbuffstream);
    
            sses_share::S2CResultParam s2c_param;
            rstream >> s2c_param;

#ifdef ENABLE_LOCAL_DEBUG
            printf("[DBG] s2c_param:\n");
            std::cout << s2c_param;
#endif
            
            for (size_t i=0; i<s2c_param.numRes; ++i) {
                Record record;
                record.id_ = s2c_param.recordIds[i];
                
                record.medicinIds_.resize(s2c_param.numMeds[i]);
                record.symptomIds_.resize(s2c_param.numSides[i]);
                
                const auto& srcMed = s2c_param.medIds[i];
                const auto& srcSide = s2c_param.sideIds[i];

                std::copy(srcMed.begin(), srcMed.end(), record.medicinIds_.begin());
                std::copy(srcSide.begin(), srcSide.end(), record.symptomIds_.begin());

                records.push_back(record);
            }
            
        } else {
            STDSC_LOG_WARN("The status of result for queryID %d is NOT success.", query_id);
        }
    }

    void wait(const int32_t query_id) const
    {
        if (cbmap_.count(query_id))
        {
            auto& rcb = cbmap_.at(query_id);
            rcb.thread->wait();
        }
    }

    const char* host_;
    const char* port_;
    const FHEcontext& context_;
    const FHEPubKey& pubkey_;
    const FHESecKey& seckey_;
    stdsc::Client client_;
    std::unordered_map<int32_t, ResultCallback> cbmap_;
};

Client::Client(const char* host, const char* port,
               FHEcontext& context,
               FHEPubKey& pubkey,
               FHESecKey& seckey)
    : pimpl_(new Impl(host, port, context, pubkey, seckey))
{
}

void Client::connect(const uint32_t retry_interval_usec,
                     const uint32_t timeout_sec)
{
    STDSC_LOG_INFO("Connect to server.");
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void Client::disconnect(void)
{
    STDSC_LOG_INFO("Disconnect from server.");
    pimpl_->disconnect();
}

void Client::register_enckeys(const int32_t key_id,
                              const std::string& context_filepath,
                              const std::string& pubkey_filepath) const
{
    pimpl_->register_enckeys(key_id, context_filepath, pubkey_filepath);
}

int32_t Client::send_query(const int32_t key_id,
                           const size_t age,
                           const std::string& gender,
                           const std::string& meds,
                           const std::string& sides,
                           const sses_share::EncData& encdata) const
{
    auto query_id = pimpl_->send_query(key_id, age, gender, meds, sides, encdata);
    return query_id;
}

int32_t Client::send_query(const int32_t key_id,
                           const size_t age,
                           const std::string& gender,
                           const std::string& meds,
                           const std::string& sides,
                           const sses_share::EncData& enc_inputs,
                           cbfunc_t cbfunc, void* cbfunc_args) const
{
    int32_t query_id = pimpl_->send_query(key_id, age, gender, meds, sides, enc_inputs);
    STDSC_LOG_DEBUG("Set callback function for query %d", query_id);
    set_callback(query_id, cbfunc, cbfunc_args);
    return query_id;
}

void Client::recv_results(const int32_t query_id, bool& status, std::vector<Record>& records) const
{
    pimpl_->recv_results(query_id, status, records);
}

void Client::set_callback(const int32_t query_id, cbfunc_t func,
                          void* args) const
{
    ResultCallback rcb;
    rcb.thread =
        std::make_shared<ResultThread>(*this, pimpl_->pubkey_, func, args);
    rcb.param = {query_id};
    pimpl_->cbmap_[query_id] = rcb;
    pimpl_->cbmap_[query_id].thread->start(pimpl_->cbmap_[query_id].param);
}

void Client::wait(const int32_t query_id) const
{
    pimpl_->wait(query_id);
}

} /* namespace sses_client */
