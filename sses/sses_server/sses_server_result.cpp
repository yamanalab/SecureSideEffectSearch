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

#include <sses_server/sses_server_result.hpp>

namespace sses_server
{

// Result
Result::Result(const int32_t key_id,
               const int32_t query_id,
               const bool status,
               const sses_share::FHECtxtBuffer& chunk_res,
               const std::vector<std::vector<int>>& chunks)
    : key_id_(key_id),
      query_id_(query_id),
      status_(status),
      chunk_res_(chunk_res)
{
    chunks_.resize(chunks.size());
    for (size_t i=0; i<chunks.size(); ++i) {
        auto& dst = chunks_[i];
        const auto& src = chunks[i];
        
        dst.resize(src.size());
        std::copy(src.begin(), src.end(), dst.begin());
    }
    
    created_time_ = std::chrono::system_clock::now();
}

double Result::elapsed_time() const
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - created_time_)
      .count();
}

} /* namespace sses_server */
