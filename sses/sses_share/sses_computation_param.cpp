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

#include <boost/algorithm/string.hpp>

#include <stdsc/stdsc_exception.hpp>
#include <sses_share/sses_computation_param.hpp>

namespace sses_share
{

void ComputationParam::get_med_ids(std::vector<int>& ids) const
{
    std::vector<std::string> elems;

    auto str = std::string(meds);
    boost::algorithm::split(elems, str, boost::is_any_of(":"));

    for (const auto& v : elems) {
        ids.push_back(std::stoi(v));
    }
}

void ComputationParam::get_side_ids(std::vector<int>& ids) const
{
    std::vector<std::string> elems;

    auto str = std::string(sides);
    boost::algorithm::split(elems, str, boost::is_any_of(":"));

    for (const auto& v : elems) {
        ids.push_back(std::stoi(v));
    }
}
    
std::ostream& operator<<(std::ostream& os, const ComputationParam& param)
{
    os << param.age << std::endl;
    os << std::string(param.gender) << std::endl;
    os << std::string(param.meds) << std::endl;
    os << std::string(param.sides) << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, ComputationParam& param)
{
    is >> param.age;
    std::string gender, meds, sides;
    is >> gender;
    is >> meds;
    is >> sides;
    gender.copy(param.gender, gender.size());
    meds.copy(param.meds, meds.size());
    sides.copy(param.sides, sides.size());
    return is;
}

} /* namespace sses_share */
