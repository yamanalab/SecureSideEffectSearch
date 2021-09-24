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

#include <stdsc/stdsc_callback_function.hpp>
#include <stdsc/stdsc_callback_function_container.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_state.hpp>

#include <sses_share/sses_define.hpp>
#include <sses_share/sses_packet.hpp>
#include <sses_server/sses_server_state.hpp>
#include <sses_server/sses_server_callback_function.hpp>
#include <sses_server/sses_server.hpp>

#include <share/define.hpp>

struct Option
{
    std::string port = PORT_SRV;
    std::string db_src_filepath = SSES_DEFAULT_SERVER_DB_SRC_FILEAPATH;
    std::string db_basedir = SSES_DEFAULT_SERVER_DB_BASE_DIR;
    uint32_t max_queries = SSES_DEFAULT_MAX_CONCURRENT_QUERIES;
    uint32_t max_results = SSES_DEFAULT_MAX_RESULTS;
    uint32_t max_result_lifetime_sec = SSES_DEFAULT_MAX_RESULT_LIFETIME_SEC;
    uint32_t num_threads = SSES_DEFAULT_NUM_THREADS;
};

void init(Option& option, int argc, char* argv[])
{
    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "p:d:f:q:r:l:h")) != -1)
    {
        switch (opt)
        {
            case 'p':
                option.port = optarg;
                break;
            case 'd':
                option.db_basedir = optarg;
                break;
            case 'f':
                option.db_src_filepath = optarg;
                break;
            case 'q':
                option.max_queries = std::stol(optarg);
                break;
            case 'r':
                option.max_results = std::stol(optarg);
                break;
            case 'l':
                option.max_result_lifetime_sec = std::stol(optarg);
                break;
            case 't':
                option.num_threads = std::stol(optarg);
                break;
            case 'h':
            default:
                printf(
                  "Usage: %s [-p PORT] [-q Max Queries] [-r max_results] [-l Max Result Lifetime] "
                  "[-t NThreads] [-d DB direcotry] [-f DB of medical records (CSV file)]\n",
                  argv[0]);
                exit(1);
        }
    }
}

void exec(Option& option)
{
    stdsc::StateContext state(std::make_shared<sses_server::StateConnected>());

    stdsc::CallbackFunctionContainer callback;
    {
        std::shared_ptr<stdsc::CallbackFunction> cb_enckeys(
          new sses_server::CallbackFunctionEncryptionKeys());
        callback.set(sses_share::kControlCodeDataEncKeys, cb_enckeys);

        std::shared_ptr<stdsc::CallbackFunction> cb_query(
          new sses_server::CallbackFunctionQuery());
        callback.set(sses_share::kControlCodeUpDownloadQuery, cb_query);

        std::shared_ptr<stdsc::CallbackFunction> cb_pirres(
          new sses_server::CallbackFunctionChunkResultRequest());
        callback.set(sses_share::kControlCodeUpDownloadChunkResult, cb_pirres);

        std::shared_ptr<stdsc::CallbackFunction> cb_result(
          new sses_server::CallbackFunctionResultRequest());
        callback.set(sses_share::kControlCodeUpDownloadResult, cb_result);

        std::shared_ptr<stdsc::CallbackFunction> cb_cancelquery(
          new sses_server::CallbackFunctionCancelQuery());
        callback.set(sses_share::kControlCodeDataCancelQuery, cb_cancelquery);
    }
    
    std::shared_ptr<sses_server::Server> server(new sses_server::Server(
      option.port.c_str(),
      callback,
      state,
      option.db_src_filepath.c_str(),
      option.db_basedir.c_str(),
      option.max_queries,
      option.max_results,
      option.max_result_lifetime_sec,
      option.num_threads));

    server->start();
    server->wait();
}

int main(int argc, char* argv[])
{
    STDSC_INIT_LOG();
    try
    {
        Option option;
        init(option, argc, argv);
        STDSC_LOG_INFO("Launched Server demo app.");
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
