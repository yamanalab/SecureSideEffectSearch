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

#include <sstream>

#include <stdsc/stdsc_callback_function_container.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_server.hpp>

#include <sses_share/sses_fhekey_container.hpp>
#include <sses_server/sses_server.hpp>
#include <sses_server/sses_server_calcmanager.hpp>
#include <sses_server/sses_server_callback_param.hpp>
#include <sses_server/sses_server_db.hpp>

namespace sses_server
{

struct Server::Impl
{
public:
    Impl(const char* port,
         stdsc::CallbackFunctionContainer& callback,
         stdsc::StateContext& state,
         const char* db_src_filepath,
         const char* db_basedir,
         const uint32_t max_concurrent_queries,
         const uint32_t max_results,
         const uint32_t result_lifetime_sec,
         const uint32_t num_threads)
        : calc_manager_(new CalcManager(max_concurrent_queries, max_results,
                                        result_lifetime_sec, num_threads)),
          key_container_(new sses_share::FHEKeyContainer()),
          db_(new sses_server::DB(db_basedir)),
          param_(new CallbackParam()),
          cparam_(new CommonCallbackParam(*calc_manager_,
                                          *key_container_,
                                          *db_, db_src_filepath))
    {
        STDSC_LOG_INFO("Initialized computation server with port #%s", port);
        callback.set_commondata(static_cast<void*>(param_.get()),
                                sizeof(*param_));
        callback.set_commondata(
            static_cast<void*>(cparam_.get()), sizeof(*cparam_),
            stdsc::CommonDataKind_t::kCommonDataOnAllConnection);
        server_ = std::make_shared<stdsc::Server<>>(port, state, callback);
    }

    ~Impl(void) = default;

    void start()
    {
        const bool enable_async_mode = true;
        server_->start(enable_async_mode);

        const uint32_t thread_num = 2;
        calc_manager_->start_threads(thread_num);
    }

    void stop(void)
    {
        server_->stop();
    }

    void wait(void)
    {
        server_->wait();
    }

private:
    std::string dec_host_;
    std::string dec_port_;
    std::shared_ptr<CalcManager> calc_manager_;
    std::shared_ptr<sses_share::FHEKeyContainer> key_container_;
    std::shared_ptr<sses_server::DB> db_;
    std::shared_ptr<CallbackParam> param_;
    std::shared_ptr<CommonCallbackParam> cparam_;
    std::shared_ptr<stdsc::Server<>> server_;
};

Server::Server(const char* port, stdsc::CallbackFunctionContainer& callback,
               stdsc::StateContext& state,
               const char* db_src_filepath,
               const char* db_basedir,
               const uint32_t max_concurrent_queries,
               const uint32_t max_results,
               const uint32_t result_lifetime_sec,
               const uint32_t num_threads)
    : pimpl_(new Impl(port, callback,
                      state,
                      db_src_filepath, db_basedir,
                      max_concurrent_queries, max_results,
                      result_lifetime_sec,
                      num_threads))
{
}

void Server::start()
{
    STDSC_LOG_INFO("Start server.");
    pimpl_->start();
}

void Server::stop(void)
{
    STDSC_LOG_INFO("Stop server.");
    pimpl_->stop();
}

void Server::wait(void)
{
    STDSC_LOG_DEBUG("Waiting for server to stop.");
    pimpl_->wait();
}

} /* namespace sses_server */
