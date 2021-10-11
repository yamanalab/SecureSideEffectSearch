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

#ifndef SSES_SERVER_QUERY_HPP
#define SSES_SERVER_QUERY_HPP

#include <cstdint>
#include <vector>
#include <memory>

#include "FHE.h"
#include "EncryptedArray.h"

#include <stdsc/stdsc_buffer.hpp>

#include <sses_share/sses_cli2srvparam.hpp>
#include <sses_share/sses_concurrent_mapqueue.hpp>
#include <sses_share/sses_fhectxt_buffer.hpp>

namespace sses_share
{
class FHEKeyContainer;
}

namespace sses_server
{

class DB;

/**
 * @brief This class is used to hold the query data.
 */
struct Query
{
    Query() = default;
    /**
     * Constructor
     * @param[in] key_id key ID
     * @param[in] comp_param computation parameter
     * @param[in] encmask encryption mask
     * @param[in] key_container_p FHE key container
     * @param[in] db_p DB
     */
    Query(const int32_t key_id,
          const sses_share::ComputationParam& param,
          const sses_share::FHECtxtBuffer& encmask,
          sses_share::FHEKeyContainer* key_container_p,
          DB* db_p);
    virtual ~Query() = default;

    /**
     * Copy constructor
     * @param[in] q query
     */
    Query(const Query& q)
        : key_id_(q.key_id_),
          param_(q.param_),
          encmask_(q.encmask_),
          key_container_p_(q.key_container_p_),
          db_p_(q.db_p_)
    {}

    int32_t key_id_;
    sses_share::ComputationParam param_;
    sses_share::FHECtxtBuffer encmask_;
    sses_share::FHEKeyContainer* key_container_p_;
    sses_server::DB* db_p_;
};

/**
 * @brief This class is used to hold the queue of queries.
 */
struct QueryQueue
  : public sses_share::ConcurrentMapQueue<int32_t, sses_server::Query>
{
    using super = sses_share::ConcurrentMapQueue<int32_t, sses_server::Query>;

    QueryQueue() = default;
    virtual ~QueryQueue() = default;

    /**
     * Push query in queue
     * @param[in] data query
     */
    virtual int32_t push(const Query& data);
};

} /* namespace sses_server */

#endif /* SSES_SERVER_QUERY_HPP */
