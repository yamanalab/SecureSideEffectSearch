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

#include <sses_server/sses_server_callback_param.hpp>

namespace sses_server
{

// CallbackParam
CallbackParam::CallbackParam(void)
{}

void CallbackParam::set_chunks(const std::vector<std::vector<int>>& chunks)
{
    clear_chunks();

    chunks_.resize(chunks.size());
    for (size_t i=0; i<chunks.size(); ++i) {
        auto& dst = chunks_[i];
        const auto& src = chunks[i];
        
        dst.resize(src.size());
        std::copy(src.begin(), src.end(), dst.begin());
    }
}
    
void CallbackParam::clear_chunks()
{
    chunks_.clear();
}
    
const std::vector<std::vector<int>>& CallbackParam::chunks() const
{
    return chunks_;
}

} /* namespace sses_server */
