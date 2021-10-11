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

#include <sses_share/sses_encdata.hpp>
#include <sses_share/sses_packet.hpp>
#include <sses_client/sses_client.hpp>
#include <sses_client/sses_client_result_thread.hpp>
#include <sses_client/sses_client_record.hpp>

namespace sses_client
{

struct ResultThread::Impl
{
    std::shared_ptr<stdsc::ThreadException> te_;

    Impl(const Client& client,
         const FHEPubKey& pubkey,
         cbfunc_t cbfunc, void* cbargs)
      : client_(client),
        pubkey_(pubkey),
        cbfunc_(cbfunc),
        cbargs_(cbargs)
    {
        te_ = stdsc::ThreadException::create();
    }

    void exec(ResultThreadParam& args,
              std::shared_ptr<stdsc::ThreadException> te)
    {
        try
        {
            STDSC_LOG_DEBUG("Launched result thread for query %d",
                            args.query_id);

            bool status = false;
            std::vector<Record> records;
            client_.recv_results(args.query_id, status, records);

            STDSC_LOG_INFO("Invoke callback function for query #%d",
                           args.query_id);
            cbfunc_(args.query_id, status, records, cbargs_);
        }
        catch (const stdsc::AbstractException& e)
        {
            STDSC_LOG_ERR("An error occurred in the result thread. [%s]", e.what());
            te->set_current_exception();
        }
    }

private:
    const Client& client_;
    const FHEPubKey& pubkey_;
    cbfunc_t cbfunc_;
    void* cbargs_;
};

ResultThread::ResultThread(const Client& client,
                           const FHEPubKey& pubkey,
                           cbfunc_t cbfunc, void* cbargs)
  : pimpl_(new Impl(client, pubkey, cbfunc, cbargs))
{
}

ResultThread::~ResultThread(void)
{
    super::join();
}

void ResultThread::start(ResultThreadParam& param)
{
    STDSC_LOG_DEBUG("Start result thread.");
    super::start(param, pimpl_->te_);
}

void ResultThread::wait(void)
{
    STDSC_LOG_DEBUG("Waiting for finish of result thread.");
    super::join();
    pimpl_->te_->rethrow_if_has_exception();
}

void ResultThread::exec(ResultThreadParam& args,
                        std::shared_ptr<stdsc::ThreadException> te) const
{
    try
    {
        pimpl_->exec(args, te);
    }
    catch (...)
    {
        te->set_current_exception();
    }
}

} /* namespace sses_client */
