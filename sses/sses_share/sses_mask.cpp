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

#include <stdsc/stdsc_exception.hpp>
#include <sses_share/sses_mask.hpp>
#include <sses_share/sses_types.hpp>

namespace sses_share
{

size_t sses_mask_compute_mask(const size_t age, const std::string& gender)
{
    size_t miss_match_count = 0;
    for (auto itr=GENDER_CHAR_MAP.begin(); itr!=GENDER_CHAR_MAP.end(); ++itr) {
        if (gender != itr->first) {
            ++miss_match_count;
        }
    }
    if (miss_match_count >= GENDER_CHAR_MAP.size()) {
        std::ostringstream oss;
        oss << "Invalid parameter. (gender: " << gender << ")";
        oss << " (" << GENDER_CHAR_MAP.at("m");
        oss << " <= gender <= " << GENDER_CHAR_MAP.at("o") << ")";
        STDSC_THROW_INVPARAM(oss.str());
    }

    return age + GENDER_CHAR_MAP.at(gender) * 128 + 5;
}

} /* namespace sses_share */
