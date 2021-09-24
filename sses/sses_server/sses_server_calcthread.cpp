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

#include <sys/syscall.h> // for thread id
#include <sys/types.h>   // for thread id
#include <unistd.h>
#include <algorithm> // for sort
#include <chrono>
#include <fstream>
#include <random>
#include <map>
#include <iomanip> // put_time
#include <boost/algorithm/string.hpp>

#include <NTL/BasicThreadPool.h>
#include <NTL/ZZ.h>
#include <NTL/lzz_pXFactoring.h>

#include <stdsc/stdsc_log.hpp>

#include <sses_share/sses_utility.hpp>
#include <sses_share/sses_fhekey_container.hpp>
#include <sses_share/sses_fhectxt_buffer.hpp>

#include <sses_server/sses_server_calcthread.hpp>
#include <sses_server/sses_server_query.hpp>
#include <sses_server/sses_server_result.hpp>
#include <sses_server/sses_server_db.hpp>

//#define ENABLE_LOCAL_DEBUG
#ifdef ENABLE_LOCAL_DEBUG
#include <sses_share/sses_fhe_debug.hpp>
#endif


#define __MULTITHREADING_IN_USE__


namespace sses_server
{

#define LOGINFO(fmt, ...) \
    STDSC_LOG_INFO("[CalThr:%d, Query:%d] " fmt, th_id, query_id, ##__VA_ARGS__)

    
static std::vector<int> mergeOR(const std::map<int, std::vector<int>>& index,
                                const std::vector<int>& id)
{
    int len = id.size();

    if (len == 0)
    {
        std::vector<int> ret;
        return ret;
    }

    if (len == 1)
    {
        if (index.find(id[0]) == index.end())
        {
            std::vector<int> ret;
            return ret;
        }
        return index.at(id[0]);
    }

    const std::vector<int> id_l = std::vector<int>(id.begin(), id.begin() + len / 2);
    const std::vector<int> id_r = std::vector<int>(id.begin() + len / 2, id.end());

    const std::vector<int> res_l = mergeOR(index, id_l);
    const std::vector<int> res_r = mergeOR(index, id_r);

    int len_l = res_l.size(), p_l = 0;
    int len_r = res_r.size(), p_r = 0;

    std::vector<int> ret;
    while (p_l != len_l && p_r != len_r)
    {
        if (res_l[p_l] < res_r[p_r]) {
            ret.push_back(res_l[p_l++]);
        } else {
            if (res_l[p_l] == res_r[p_r]) {
                ++p_l;
            }
            ret.push_back(res_r[p_r++]);
        }
    }
    while (p_l != len_l) {
        ret.push_back(res_l[p_l++]);
    }
    while (p_r != len_r) {
        ret.push_back(res_r[p_r++]);
    }

    return ret;
}

static std::vector<int> merge(const std::vector<int>& medID,
                              const std::vector<int>& sideID,
                              std::map<int, std::vector<int>>& medIndex,
                              std::map<int, std::vector<int>>& sideIndex)
{
    // First use OR to merge between medIndex[medID] -> res1
    const std::vector<int> medres = mergeOR(medIndex, medID);

    // Then use OR to merge between sideIndex[sideID] -> res2
    const std::vector<int> sideres = mergeOR(sideIndex, sideID);

    // Then use AND to merge res1 and res2 -> ret

    std::vector<int> ret;

    int lenmed = medres.size(), pmed = 0;
    int lenside = sideres.size(), pside = 0;

    while (pmed != lenmed && pside != lenside)
    {
        if (medres[pmed] < sideres[pside]) {
            ++pmed;
        } else if (medres[pmed] > sideres[pside]) {
            ++pside;
        } else {
            ret.push_back(medres[pmed++]);
            ++pside;
        }
    }

    return ret;
}
    
struct CalcThread::Impl
{
    Impl(QueryQueue& in_queue, ResultQueue& out_queue, const uint32_t num_threads)
        : in_queue_(in_queue), out_queue_(out_queue)
    {
        param_.num_threads = num_threads;
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        auto th_id = syscall(SYS_gettid);
        STDSC_LOG_INFO("[CalThr:%d] Launched CalThread.", th_id);

        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 generator(seed);

        auto num_threads = args.num_threads;
        
        while (!args.force_finish)
        {
            STDSC_LOG_INFO("[CalThr:%d] Waiting for a query to be inserted into Queue.", th_id);

            int32_t query_id;
            Query query;
            while (!in_queue_.pop(query_id, query))
            {
                usleep(args.retry_interval_msec * 1000);
            }

            LOGINFO("Pop a query from Queue. [age: %lu, gender: %s, meds: %s, sides: %s]",
                    query.param_.age,
                    query.param_.gender,
                    query.param_.meds,
                    query.param_.sides);

            LOGINFO("Start processing for query %d.", query_id);

            const auto key_id = query.key_id_;
            const auto& comp_param = query.param_;
            const auto& key_container = *query.key_container_p_;
            const auto& db = *query.db_p_;

            auto dbbasicfilepath = db.dbbasic_filepath(key_id);
            DBBasicFile dbbasic(dbbasicfilepath);
            LOGINFO("Load DBBasic. [status:%d, totalRecords:%lu, totalMedicines:%lu, totalSymptoms:%lu]",
                    dbbasic.dbstatus,
                    dbbasic.totalRecordsNum,
                    dbbasic.totalMedicinesNum,
                    dbbasic.totalSymptomsNum);
            
            auto context = key_container.get_context(key_id);
            NTL::ZZX G = context.alMod.getFactorsOverZZ()[0];
            EncryptedArray ea(context, G);
            long nslots = ea.size();
            
            FHEPubKey pubkey(context);
            key_container.get(key_id, sses_share::KeyKind_t::kKindPubKey, pubkey);

            std::map<int, std::vector<int>> medIndex;
            std::map<int, std::vector<int>> sideIndex;

            std::ifstream findexmed(db.medinv_filepath(key_id), std::ios::binary);
            std::string line;
            std::vector<std::string> medInfo;
            int numMed;
            findexmed >> numMed;
            for (int i = 0; i < numMed; ++i)
            {
                findexmed >> line;
                boost::algorithm::split(medInfo, line, boost::is_any_of(":"));
                int medId = std::stoi(medInfo[0]);
                int numindex = std::stoi(medInfo[1]);
                std::vector<int> tempindex;
                while (numindex > 0)
                {
                    int temprec;
                    findexmed >> temprec;
                    tempindex.push_back(temprec);
                    numindex--;
                }
                medIndex.insert(make_pair(medId, tempindex));
            }
            findexmed.close();

            std::ifstream findexside(db.sideinv_filepath(key_id), std::ios::binary);
            std::vector<std::string> sideInfo;
            int numSide;
            findexside >> numSide;
            for (int i = 0; i < numSide; ++i)
            {
                findexside >> line;
                boost::algorithm::split(sideInfo, line, boost::is_any_of(":"));
                int sideId = std::stoi(sideInfo[0]);
                int numindex = std::stoi(sideInfo[1]);
                std::vector<int> tempindex;
                while (numindex > 0)
                {
                    int temprec;
                    findexside >> temprec;
                    tempindex.push_back(temprec);
                    numindex--;
                }
                sideIndex.insert(make_pair(sideId, tempindex));
            }
            findexside.close();

            const std::vector<long> allzero_long(nslots, 0);
            Ctxt allzero(pubkey);
            ea.encrypt(allzero, pubkey, allzero_long);

            NTL::SetNumThreads(num_threads);

            std::vector<Ctxt> ctxts;
            query.encmask_.deserialize(pubkey, ctxts);
            Ctxt& query_mask = ctxts[0];

            std::vector<int> MedID, SideID;
            comp_param.get_med_ids(MedID);
            comp_param.get_side_ids(SideID);

            std::vector<int> filteredres = merge(MedID, SideID, medIndex, sideIndex);

            int numRes = filteredres.size(), numchunks = 0;

            LOGINFO("Completed filtering.");
            
            std::vector<std::vector<int>> chunks;
            std::vector<Ctxt> chunk_res;
            for (int i = 0; i < numRes; i += 100, ++numchunks)
            {
                int end = std::min(i + 100, numRes);
                std::vector<int> chunk(filteredres.begin() + i,
                                  filteredres.begin() + end);
                chunks.push_back(chunk);
                chunk_res.push_back(allzero);
            }

            LOGINFO("Completed chunk splitting.");

#ifndef __MULTITHREADING_IN_USE__
            long first = 0, last = numchunks;
#else
            NTL_EXEC_RANGE(numchunks, first, last);
#endif            

            for (long i = first; i < last; ++i)
            {

                for (size_t j = 0; j < chunks[i].size(); ++j)
                {
                    std::vector<long> posindicator_long = allzero_long;
                    posindicator_long[j] = 1;
                    NTL::ZZX posindicator;
                    ea.encode(posindicator, posindicator_long);

                    std::string filename =
                        db.encdata_dirpath(key_id) + "/" + std::to_string(chunks[i][j]) + ".bin";
                    std::ifstream fdb(filename.c_str(), std::ios::binary);
                    Ctxt encmask(pubkey);
                    //assert(fdb >> encmask);
                    fdb >> encmask;
                    fdb.close();
                    
                    encmask.multByConstant(posindicator);
                    chunk_res[i].addCtxt(encmask, false);
                }

                chunk_res[i].addCtxt(query_mask, true);

                std::vector<Ctxt> rangemul;
                for (int diff = -5; diff <= 5; ++diff)
                {
                    Ctxt diffed(pubkey);
                    diffed = chunk_res[i];
                    diffed.addConstant(NTL::to_ZZX(diff));
                    rangemul.push_back(diffed);
                }

                while (rangemul.size() > 1)
                {
                    std::vector<Ctxt> rangemul_derived;
                    for (size_t j = 0; j < rangemul.size(); j += 2)
                    {
                        if (j + 1 < rangemul.size()) {
                            rangemul[j].multiplyBy(rangemul[j + 1]);
                        }
                        rangemul_derived.push_back(rangemul[j]);
                    }
                    rangemul = rangemul_derived;
                }

                chunk_res[i] = rangemul[0];

                std::vector<long> randlist_long;
                for (int i = 0; i < nslots; ++i) {
                    randlist_long.push_back(generator() % 256 + 1);
                }
                NTL::ZZX randlist;
                ea.encode(randlist, randlist_long);
                chunk_res[i].multByConstant(randlist);
                
            }
            
#ifdef __MULTITHREADING_IN_USE__            
            NTL_EXEC_RANGE_END;
#endif

            LOGINFO("Complete calculation.");

            sses_share::FHECtxtBuffer chunk_res_ctxtbuff;
            chunk_res_ctxtbuff.serialize(pubkey, chunk_res);
            
            Result result(key_id, query_id, true, chunk_res_ctxtbuff, chunks);
            out_queue_.push(query_id, result);
            
            LOGINFO("Push results of each chunk to Queue.");
            
            LOGINFO("Finish processing for query %d.", query_id);
        }
    }

    QueryQueue& in_queue_;
    ResultQueue& out_queue_;
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
};

CalcThread::CalcThread(QueryQueue& in_queue,
                       ResultQueue& out_queue,
                       const uint32_t num_threads)
    : pimpl_(new Impl(in_queue, out_queue, num_threads))
{
}

void CalcThread::start()
{
    pimpl_->param_.force_finish = false;
    super::start(pimpl_->param_, pimpl_->te_);
}

void CalcThread::stop()
{
    STDSC_LOG_INFO("Stop calculation thread.");
    pimpl_->param_.force_finish = true;
}

void CalcThread::exec(CalcThreadParam& args,
                      std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace sses_server */
