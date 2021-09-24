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

#ifndef SSES_COMPUTATION_PARAM_HPP
#define SSES_COMPUTATION_PARAM_HPP

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <sses_share/sses_define.hpp>

namespace sses_share
{

/**
 * @brief This class is used to hold computation param
 */
struct ComputationParam
{
    size_t age;
    char gender[SSES_DEFAULT_SIZE_OF_STR_FOR_TRANSDATA];
    char meds[SSES_DEFAULT_SIZE_OF_STR_FOR_TRANSDATA];
    char sides[SSES_DEFAULT_SIZE_OF_STR_FOR_TRANSDATA];

    std::string to_string() const
    {
        std::ostringstream oss;
        oss << age << ", " << std::string(gender) << ", "
            << std::string(meds) << ", " << std::string(sides);
        return oss.str();
    }

    void get_med_ids(std::vector<int>& ids) const;
    void get_side_ids(std::vector<int>& ids) const;
};

std::ostream& operator<<(std::ostream& os, const ComputationParam& param);
std::istream& operator>>(std::istream& is, ComputationParam& param);

} /* namespace sses_share */

#endif /* SSES_COMPUTATION_PARAM_HPP */
