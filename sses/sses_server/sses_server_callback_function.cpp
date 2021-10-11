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
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "FHE.h"
#include "EncryptedArray.h"

#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_socket.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_log.hpp>

#include <sses_share/sses_cli2srvparam.hpp>
#include <sses_share/sses_encdata.hpp>
#include <sses_share/sses_packet.hpp>
#include <sses_share/sses_plaindata.hpp>
#include <sses_share/sses_fhe_utility.hpp>
#include <sses_share/sses_fhekey_container.hpp>
#include <sses_share/sses_srv2cliparam.hpp>
#include <sses_share/sses_fhectxt_buffer.hpp>

#include <sses_server/sses_server_calcmanager.hpp>
#include <sses_server/sses_server_callback_function.hpp>
#include <sses_server/sses_server_callback_param.hpp>
#include <sses_server/sses_server_query.hpp>
#include <sses_server/sses_server_result.hpp>
#include <sses_server/sses_server_state.hpp>
#include <sses_server/sses_server_db.hpp>

//#define ENABLE_LOCAL_DEBUG

namespace sses_server
{

// CallbackFunction for Encryption keys
DEFUN_DATA(CallbackFunctionEncryptionKeys)
{
    SSES_UTILITY_NEWLINE;
    STDSC_LOG_INFO("Receive encryption keys. (current state : %s)",
                   state.current_state_str().c_str());

    STDSC_THROW_CALLBACK_IF_CHECK(
        kStateConnected <= state.current_state(),
        "Warn: must be ConnectedState to receive encryption keys.");

    STDSC_LOG_INFO("Start encryption key registration.");
                   
    DEF_CDATA_ON_ALL(sses_server::CommonCallbackParam);
    auto& key_container = cdata_a->key_container_;
    auto& db = cdata_a->db_;
    auto& db_src_filepath = cdata_a->db_src_filepath_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    sses_share::PlainData<sses_share::C2SEnckeyParam> rplaindata;
    rplaindata.load(rstream);
    const auto param = rplaindata.data();

    key_container.setup(param.key_id);
    const auto context_filepath = key_container.filepath(param.key_id,
                                                         sses_share::KeyKind_t::kKindContext);
    const auto pubkey_filepath = key_container.filepath(param.key_id,
                                                        sses_share::KeyKind_t::kKindPubKey);
    
    sses_share::fhe_utility::read_from_binary_stream(rstream, rbuffstream.data(),
                                                     param.context_stream_sz,
                                                     context_filepath);
    sses_share::fhe_utility::read_from_binary_stream(rstream, rbuffstream.data(),
                                                     param.pubkey_stream_sz,
                                                     pubkey_filepath);

    auto context = key_container.get_context(param.key_id);
    FHEPubKey pubkey(context);
    key_container.get(param.key_id, sses_share::KeyKind_t::kKindPubKey, pubkey);
    
    // setup DB
    if (!db.is_enable(param.key_id)) {
        db.setup(param.key_id, db_src_filepath, context, pubkey);
    }
                                     
    STDSC_LOG_INFO("Finish encryption key registration. "
                   "[keyID:%d, context_sz:%ld, pubkey_sz:%ld]",
                   param.key_id,
                   param.context_stream_sz,
                   param.pubkey_stream_sz);
                   
    state.set(kEventEncryptionKey);
}

// CallbackFunction for Query
DEFUN_UPDOWNLOAD(CallbackFunctionQuery)
{
    SSES_UTILITY_NEWLINE;
    STDSC_LOG_INFO("Received query. (current state : %s)",
                   state.current_state_str().c_str());

    STDSC_THROW_CALLBACK_IF_CHECK(
        kStateReady <= state.current_state(),
        "Warn: must be ReadyState to receive query.");

    STDSC_LOG_INFO("Start processing the received query.");
    
    DEF_CDATA_ON_ALL(sses_server::CommonCallbackParam);
    auto& calc_manager = cdata_a->calc_manager_;
    auto& key_container = cdata_a->key_container_;
    auto& db = cdata_a->db_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    sses_share::PlainData<sses_share::C2SQueryParam> rplaindata;
    rplaindata.load(rstream);
    const auto& param = rplaindata.data();
    
    STDSC_LOG_INFO(" Query: [age: %lu, gender: %s, meds: %s, sides: %s]",
                   param.comp_param.age,
                   param.comp_param.gender,
                   param.comp_param.meds,
                   param.comp_param.sides);
                   
    STDSC_LOG_INFO(" keyID: %d, Encryption mask sz: %lu",
                   param.key_id, param.encdata_stream_sz);

    key_container.setup(param.key_id);
    auto context = key_container.get_context(param.key_id);
    FHEPubKey pubkey(context);
    key_container.get(param.key_id, sses_share::KeyKind_t::kKindPubKey, pubkey);

    sses_share::EncData encmask(pubkey);
    encmask.load(rstream);

    sses_share::FHECtxtBuffer encmask_ctxtbuff;
    encmask_ctxtbuff.serialize(pubkey, encmask.vdata());
    Query query(param.key_id, param.comp_param, encmask_ctxtbuff, &key_container, &db);

    auto query_id = calc_manager.push_query(query);
    STDSC_LOG_INFO("Put query in Queue of computation thread. [queryID: %d]", query_id);

    STDSC_LOG_INFO("Start sending query ID. [queryID: %d]", query_id);
    
    sses_share::PlainData<int32_t> splaindata;
    splaindata.push(query_id);

    auto sz = splaindata.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    splaindata.save(sstream);

    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(
      stdsc::make_data_packet(sses_share::kControlCodeDataQueryID, sz));
    sock.send_buffer(*bsbuff);

    STDSC_LOG_INFO("Finish sending query ID. [queryID: %d]", query_id);

    STDSC_LOG_INFO("Finish processing the received query.");

    state.set(kEventQuery);
}

// CallbackFunction for Query
DEFUN_UPDOWNLOAD(CallbackFunctionChunkResultRequest)
{
    SSES_UTILITY_NEWLINE;
    STDSC_LOG_INFO("Received request for the result of each chunk. (current state : %s)",
                   state.current_state_str().c_str());

    STDSC_THROW_CALLBACK_IF_CHECK(
        kStateComputing <= state.current_state(),
        "Warn: must be ComputingState to receive chunk result request.");

    DEF_CDATA_ON_ALL(sses_server::CommonCallbackParam);
    auto& calc_manager = cdata_a->calc_manager_;
    auto& key_container = cdata_a->key_container_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    sses_share::PlainData<sses_share::C2SChunkResreqParam> rplaindata;
    rplaindata.load(rstream);
    const auto& param = rplaindata.data();

    STDSC_LOG_INFO("Start proccesing requests for the result of each chunk for queryID %d.",
                   param.query_id);

    STDSC_LOG_INFO("Waiting for each chunk for queryID %d to complete its comuptation.",
                   param.query_id);
    
    Result result;
    calc_manager.pop_result(param.query_id, result);
    
    STDSC_LOG_INFO("Get the result of each chunk for queryID %d. [keyID: %d, status: %d]",
                   param.query_id,
                   result.key_id_,
                   result.status_);

    STDSC_LOG_INFO("Start sending the result of each chunk for queryID %d", param.query_id);
    
    sses_share::PlainData<sses_share::S2CChunkResultParam> splaindata;
    sses_share::S2CChunkResultParam s2c_param;
    s2c_param.status = result.status_ ? sses_share::kServerResultStatusSuccess
                                      : sses_share::kServerResultStatusFailed;
    s2c_param.key_id = result.key_id_;
    splaindata.push(s2c_param);

    auto context = key_container.get_context(result.key_id_);
    FHEPubKey pubkey(context);
    key_container.get(result.key_id_, sses_share::KeyKind_t::kKindPubKey, pubkey);

    std::vector<Ctxt> ctxts;
    result.chunk_res_.deserialize(pubkey, ctxts);
    sses_share::EncData enc_results(pubkey, ctxts);

    auto sz = splaindata.stream_size() + enc_results.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    splaindata.save(sstream);
    enc_results.save(sstream);

    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(
      stdsc::make_data_packet(sses_share::kControlCodeDataChunkResult, sz));
    sock.send_buffer(*bsbuff);

    STDSC_LOG_INFO("Finish sending the result of each chunk for queryID %d", param.query_id);

    STDSC_LOG_INFO("Finish proccesing of request for the result of each chunk.");

#ifdef ENABLE_LOCAL_DEBUG
    printf("[DBG] chunks (%lu) : ", result.chunks_.size());
    for (size_t i=0; i<result.chunks_.size(); ++i) {
        const auto& c = result.chunks_[i];
        printf("[%ld] ", i);
        for (size_t j=0; j<c.size(); ++j) {
            printf("%d ", c[j]);
        }
        printf(" ");
    }
    printf("\n");
#endif
    
    // save chunks size for 'Computed state'
    DEF_CDATA_ON_EACH(sses_server::CallbackParam);
    cdata_e->set_chunks(result.chunks_);

    state.set(kEventChunkResult);
}

// CallbackFunction for Result Request
DEFUN_UPDOWNLOAD(CallbackFunctionResultRequest)
{
    SSES_UTILITY_NEWLINE;    
    STDSC_LOG_INFO("Received result request. (current state : %s)",
                   state.current_state_str().c_str());

    STDSC_THROW_CALLBACK_IF_CHECK(
        kStateComputed <= state.current_state(),
        "Warn: must be ComputedState to receive result request.");

    DEF_CDATA_ON_ALL(sses_server::CommonCallbackParam);
    auto& db = cdata_a->db_;

    DEF_CDATA_ON_EACH(sses_server::CallbackParam);
    const auto& chunks = cdata_e->chunks();
    
    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    sses_share::PlainData<sses_share::C2SResreqParam> rplaindata_param;
    rplaindata_param.load(rstream);
    const auto& param = rplaindata_param.data();

    sses_share::PlainData<sses_share::C2SSelectedInfo> rplaindata_selinfo;
    rplaindata_selinfo.load(rstream);
    const auto& selinfo = rplaindata_selinfo.vdata();

    STDSC_LOG_INFO("Start proccesing result request for queryID %d. [Num of selected info:%lu, keyID:%d]",
                   param.query_id, selinfo.size(), param.key_id);

#ifdef ENABLE_LOCAL_DEBUG
    printf("[DBG] selinfo (%lu) : ", selinfo.size());
    for (const auto& v: selinfo) {
        printf("(%d,%d) ", v.chunk_id, v.pos_id);
    }
    printf("\n");
#endif

    std::vector<std::pair<int, int>> choice_list;
    for (auto& v : selinfo) {
        size_t i = v.chunk_id;
        size_t j = v.pos_id;
        
        if (j < chunks[i].size()) {
            choice_list.push_back(std::make_pair(i, j));
        }
    }

#ifdef ENABLE_LOCAL_DEBUG
    printf("[DBG] choice_list (%lu) : ", choice_list.size());
    for (auto& pair : choice_list) {
        printf("(%d,%d) ", pair.first, pair.second);
    }
    printf("\n");
#endif

    sses_share::PlainData<sses_share::S2CResultParam> splaindata;
    sses_share::S2CResultParam s2c_param;

    size_t numRes = choice_list.size();
    s2c_param.numRes = numRes;
    
    s2c_param.numMeds.resize(numRes);
    s2c_param.numSides.resize(numRes);
    s2c_param.recordIds.resize(numRes);
    s2c_param.medIds.resize(numRes);
    s2c_param.sideIds.resize(numRes);

    size_t tableIndex = 0;
    std::vector<size_t>* numTable[2] = {&s2c_param.numMeds, &s2c_param.numSides};
    std::vector<std::vector<int32_t>>* idTable[2] = {&s2c_param.medIds, &s2c_param.sideIds};
    
    int record_id;
    std::vector<std::string> tmp;
    std::string auxdata_path, line, cutted_line;
    size_t begin_idx, end_idx;

    auto auxdir = db.auxdata_dirpath(param.key_id) + "/";

    for (size_t i = 0; i < numRes; ++i)
    {
        record_id = chunks[choice_list[i].first][choice_list[i].second];
        s2c_param.recordIds[i] = record_id;

        auxdata_path = auxdir + std::to_string(record_id) + ".bin";
        std::ifstream auxdata_ifs(auxdata_path, std::ios::binary);

        size_t loop = 0;
        while (std::getline(auxdata_ifs, line))
        {
            auto& nums = *numTable[tableIndex];
            auto& ids = *idTable[tableIndex];

            begin_idx = line.find("[");
            end_idx = line.find("]");
            cutted_line =
                line.substr(begin_idx + 1, end_idx - begin_idx - 1);
            boost::algorithm::split(tmp, cutted_line,
                                    boost::is_any_of(","));

            nums[i] = tmp.size();

            ids[i].resize(tmp.size());

            size_t ids_index = 0;
            for (std::string s : tmp)
            {
                size_t idx = s.find(" ");
                if (idx != std::string::npos)
                {
                    s.erase(idx, 1);
                }

                ids[i][ids_index] = stoi(s);
                ++ids_index;
            }

            tableIndex = 1 - tableIndex;

            ++loop;
        }
    }
    
#ifdef ENABLE_LOCAL_DEBUG
    printf("[DBG] s2c_param:\n");
    std::cout << s2c_param;
#endif
    
    auto sz = 0;
    {
        std::ostringstream oss;
        oss << s2c_param;
        sz = oss.str().size();
    }
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    sstream << s2c_param;

    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(
      stdsc::make_data_packet(sses_share::kControlCodeDataResult, sz));
    sock.send_buffer(*bsbuff);
    
    STDSC_LOG_INFO("Finish sending results.");

    state.set(kEventResult);
}

// CallbackFunction for Cancel query
DEFUN_DATA(CallbackFunctionCancelQuery)
{
    STDSC_LOG_INFO("Received cancel query. (current state : %s)",
                   state.current_state_str().c_str());

    STDSC_THROW_CALLBACK_IF_CHECK(
        kStateReady <= state.current_state(),
        "Warn: must be ReadyState to receive result request.");
    
    STDSC_LOG_INFO("Canceled query.");

    state.set(kEventCancelQuery);
}

} /* namespace sses_server */
