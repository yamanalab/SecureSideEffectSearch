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

#ifndef SSES_SERVER_CALCMANAGER_HPP
#define SSES_SERVER_CALCMANAGER_HPP

#include <cstdbool>
#include <memory>
#include <string>

class FHEcontext;
class FHEPubKey;

namespace sses_server
{

class Query;
class Result;

class CalcManager
{
public:
    /**
     * Constructor
     * @param[in] max_concurrent_queries max number of concurrent queries
     * @param[in] max_results max        result number to hold
     * @param[in] result_lifetime_sec    lifetime to hold (sec)
     * @param[in] num_threads            number of threads used for calculations per query
     */
    CalcManager(const uint32_t max_concurrent_queries,
                const uint32_t max_results,
                const uint32_t result_lifetime_sec,
                const uint32_t num_threads);
    virtual ~CalcManager() = default;

    /**
     * Start calculation threads
     * @param[in] thread_pool_size number of threads in the thread pool
     */
    void start_threads(const uint32_t thread_pool_size);

    /**
     * Stop calculation threads
     */
    void stop_threads();

    /**
     * Regist encryption keys
     * @param[in] key_id key ID
     * @param[in] context context
     * @param[in] pubkey public key
     */
    void regist_enckeys(const int32_t key_id,
                        const FHEcontext& context,
                        const FHEPubKey& pubkey);

    /**
     * Set queries
     * @param[in] query query
     * @return query ID
     */
    int32_t push_query(const Query& query);

    /**
     * Get results of query
     * @paran[in] query_id query ID
     * @param[out] result result
     * @param[in] retry_interval_usec retry interval (usec)
     */
    void pop_result(const int32_t query_id, Result& result,
                    const uint32_t retry_interval_msec = 100) const;

    /**
     * Delete results if number of results grater than max number and expired
     * lifetime
     */
    void cleanup_results();

private:
    class Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace sses_server */

#endif /* SSES_SERVER_CALCMANAGER_HPP */
