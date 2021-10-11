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
#include <fstream>
#include <vector>

#include "FHE.h"
#include "EncryptedArray.h"

#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>

#include <sses_share/sses_define.hpp>
#include <sses_share/sses_utility.hpp>
#include <sses_server/sses_server_result.hpp>
#include <sses_server/sses_server_calcmanager.hpp>
#include <sses_server/sses_server_calcthread.hpp>
#include <sses_server/sses_server_query.hpp>

namespace sses_server
{
    
struct CalcManager::Impl
{
    Impl(const uint32_t max_concurrent_queries,
         const uint32_t max_results,
         const uint32_t result_lifetime_sec,
         const uint32_t num_threads)
      : max_concurrent_queries_(max_concurrent_queries),
        max_results_(max_results),
        result_lifetime_sec_(result_lifetime_sec)
    {
    }

    const uint32_t max_concurrent_queries_;
    const uint32_t max_results_;
    const uint32_t result_lifetime_sec_;
    QueryQueue qque_;
    ResultQueue rque_;
    std::vector<std::shared_ptr<CalcThread>> threads_;
};

CalcManager::CalcManager(const uint32_t max_concurrent_queries,
                         const uint32_t max_results,
                         const uint32_t result_lifetime_sec,
                         const uint32_t num_threads)
    : pimpl_(new Impl(max_concurrent_queries, max_results, result_lifetime_sec, num_threads))
{
}

void CalcManager::start_threads(const uint32_t thread_pool_size)
{
    STDSC_LOG_INFO("Start calculation threads. (n:%d)", thread_pool_size);
    pimpl_->threads_.clear();
    for (size_t i = 0; i < thread_pool_size; ++i)
    {
        pimpl_->threads_.emplace_back(
          std::make_shared<CalcThread>(pimpl_->qque_, pimpl_->rque_));
    }
    
    for (const auto& thread : pimpl_->threads_)
    {
        thread->start();
    }
}

void CalcManager::stop_threads()
{
    STDSC_LOG_INFO("Stop calculation threads.");
}

void CalcManager::regist_enckeys(const int32_t key_id,
                                 const FHEcontext& context,
                                 const FHEPubKey& pubkey)
{
}

int32_t CalcManager::push_query(const Query& query)
{
    int32_t query_id = -1;

    if (pimpl_->qque_.size() < pimpl_->max_concurrent_queries_ &&
        pimpl_->rque_.size() < pimpl_->max_results_)
    {
        try
        {
            query_id = pimpl_->qque_.push(query);
        }
        catch (stdsc::AbstractException& ex)
        {
            STDSC_LOG_WARN(ex.what());
        }
    }

    return query_id;
}

void CalcManager::pop_result(const int32_t query_id, Result& result,
                             const uint32_t retry_interval_msec) const
{
    while (!pimpl_->rque_.pop(query_id, result))
    {
        usleep(retry_interval_msec * 1000);
    }
}

void CalcManager::cleanup_results()
{
    if (pimpl_->rque_.size() >= pimpl_->max_results_)
    {
        std::vector<int32_t> query_ids;
        for (const auto& pair : pimpl_->rque_)
        {
            const auto& query_id = pair.first;
            const auto& result = pair.second;
            if (result.elapsed_time() >= pimpl_->result_lifetime_sec_)
            {
                STDSC_LOG_INFO(
                  "Deleted the results of query%d because it has expired.",
                  query_id);
                query_ids.push_back(query_id);
            }
        }
        Result tmp;
        for (const auto& id : query_ids)
        {
            pimpl_->rque_.pop(id, tmp);
        }
    }
}

} /* namespace sses_server */
