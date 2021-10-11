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

#ifndef SSES_CLIENT_HPP
#define SSES_CLIENT_HPP

#include <memory>
#include <sses_share/sses_define.hpp>
#include <sses_client/sses_client_result_cbfunc.hpp>

namespace sses_share
{
class EncData;
class ComputationParam;
} // namespace sses_share

namespace sses_client
{

class Record;

/**
 * @brief Provides client.
 */
class Client
{
public:
    /**
     * Constructor
     * @param[in] host hostname
     * @param[in] port port number
     * @param[in] context FHE context
     * @param[in] pubkey FHE public key
     * @param[in] pubkey FHE seckey key
     */
    Client(const char* host, const char* port, FHEcontext& context, FHEPubKey& pubkey, FHESecKey& seckey);
    virtual ~Client(void) = default;

    /**
     * Connect
     * @param[in] retry_interval_usec retry interval (usec)
     * @param[in] timeout_sec timeout (sec)
     */
    void connect(const uint32_t retry_interval_usec = SSES_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = SSES_TIMEOUT_SEC);
    /**
     * Disconnect
     */
    void disconnect();

    /**
     * Register encryption keys
     * @param[in] key_id key ID
     * @param[in] context context
     * @param[in] pubkey public key
     */
    void register_enckeys(const int32_t key_id,
                          const std::string& context_filepath,
                          const std::string& pubkey_filepath) const;

    /**
     * Send query
     * @param[in] key_id key ID
     * @param[in] age age
     * @param[in] gender gender
     * @param[in] meds medicines list
     * @param[in] sides side effect list
     * @param[in] enc_input encrypted input values
     * @return queryID
     */
    int32_t send_query(const int32_t key_id,
                       const size_t age,
                       const std::string& gender,
                       const std::string& meds,
                       const std::string& sides,
                       const sses_share::EncData& encdata) const;

    /**
     * Send query
     * @param[in] key_id key ID
     * @param[in] age age
     * @param[in] gender gender
     * @param[in] meds medicines list
     * @param[in] sides side effect list
     * @param[in] enc_input encrypted input values (1 or 2)
     * @param[in] cbfunc callback function
     * @param[in] cbfunc_args arguments for callback function
     * @return queryID
     */
    int32_t send_query(const int32_t key_id,
                       const size_t age,
                       const std::string& gender,
                       const std::string& meds,
                       const std::string& sides,
                       const sses_share::EncData& enc_inputs, cbfunc_t cbfunc,
                       void* cbfunc_args) const;

    /**
     * Receive results
     * @param[in] query_id     query ID
     * @param[out] status      calcuration status
     * @param[out] records     records
     */
    void recv_results(const int32_t query_id, bool& status,
                      std::vector<Record>& records) const;

    /**
     * Set callback functions
     * @param[in] query_id queryID
     * @param[in] func callback function
     * @param[in] args arguments for callback function
     */
    void set_callback(const int32_t query_id, cbfunc_t funvc, void* args) const;

    /**
     * Wait for finish of query
     * @param[in] query_id query ID
     */
    void wait(const int32_t query_id) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace sses_client */

#endif /* SSES_CLIENT_HPP */
