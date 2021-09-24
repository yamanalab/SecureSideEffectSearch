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

#ifndef SSES_SERVER_CALLBACK_PARAM_HPP
#define SSES_SERVER_CALLBACK_PARAM_HPP

#include <memory>
#include <string>
#include <vector>

namespace sses_share
{
class FHEKeyContainer;
}

namespace sses_server
{

class CalcManager;
class DB;

/**
 * @brief This class is used to hold the callback parameters for Server.
 * This parameter is managed independently for each connections.
 */
struct CallbackParam
{
    CallbackParam(void);
    virtual ~CallbackParam(void) = default;

    void set_chunks(const std::vector<std::vector<int>>& chunks);
    void clear_chunks();
    const std::vector<std::vector<int>>& chunks() const;
    
private:
    std::vector<std::vector<int>> chunks_;
};

/**
 * @brief This class is used to hold the callback parameters for Server.
 * This parameter to shared on all connections.
 */
struct CommonCallbackParam
{
    CommonCallbackParam(CalcManager& calc_manager,
                        sses_share::FHEKeyContainer& key_container,
                        sses_server::DB& db,
                        const char* db_src_filepath)
        : calc_manager_(calc_manager),
          key_container_(key_container),
          db_(db),
          db_src_filepath_(db_src_filepath)
    {}
    virtual ~CommonCallbackParam(void) = default;
    
    CalcManager& calc_manager_;
    sses_share::FHEKeyContainer& key_container_;
    sses_server::DB& db_;
    std::string db_src_filepath_;
};

} /* namespace sses_server */

#endif /* SSES_SERVER_CALLBACK_PARAM_HPP */
