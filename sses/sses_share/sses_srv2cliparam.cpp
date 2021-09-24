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
#include <sses_share/sses_srv2cliparam.hpp>

namespace sses_share
{

std::ostream& operator<<(std::ostream& os, const S2CChunkResultParam& param)
{
    auto i32_status = static_cast<int32_t>(param.status);
    os << i32_status << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, S2CChunkResultParam& param)
{
    int32_t i32_status;
    is >> i32_status;
    param.status = static_cast<ServerResultStatus_t>(i32_status);
    return is;
}

std::ostream& operator<<(std::ostream& os, const S2CResultParam& param)
{
    STDSC_THROW_INVPARAM_IF_CHECK(param.recordIds.size() == param.numRes,
                                  "The size of recordIds dones NOT match 'numRes'");
    STDSC_THROW_INVPARAM_IF_CHECK(param.numMeds.size() == param.numRes,
                                  "The size of numMeds dones NOT match 'numRes'");
    STDSC_THROW_INVPARAM_IF_CHECK(param.numSides.size() == param.numRes,
                                  "The size of numSides dones NOT match 'numRes'");
    
    os << param.numRes << std::endl;
    for (size_t i=0; i<param.numRes; ++i) {
        os << param.numMeds[i] << std::endl;
    }
    for (size_t i=0; i<param.numRes; ++i) {
        os << param.numSides[i] << std::endl;
    }
    
    for (size_t i=0; i<param.numRes; ++i) {
        os << param.recordIds[i] << std::endl;
    }

    for (size_t i=0; i<param.numRes; ++i) {
        auto& med = param.medIds[i];
        for (size_t j=0; j<param.numMeds[i]; ++j) {
            os << med[j] << std::endl;
        }
    }
    for (size_t i=0; i<param.numRes; ++i) {
        auto& side = param.sideIds[i];
        for (size_t j=0; j<param.numSides[i]; ++j) {
            os << side[j] << std::endl;
        }
    }
    
    return os;
}

std::istream& operator>>(std::istream& is, S2CResultParam& param)
{
    is >> param.numRes;
    
    param.numMeds.resize(param.numRes);
    param.numSides.resize(param.numRes);
    param.recordIds.resize(param.numRes);
    param.medIds.resize(param.numRes);
    param.sideIds.resize(param.numRes);
    
    for (size_t i=0; i<param.numRes; ++i) {
        is >> param.numMeds[i];
    }

    for (size_t i=0; i<param.numRes; ++i) {
        is >> param.numSides[i];
    }

    for (size_t i=0; i<param.numRes; ++i) {
        is >> param.recordIds[i];
    }

    for (size_t i=0; i<param.numRes; ++i) {
        auto& med = param.medIds[i];
        med.resize(param.numMeds[i]);
        for (size_t j=0; j<param.numMeds[i]; ++j) {
            is >> med[j];
        }
    }

    for (size_t i=0; i<param.numRes; ++i) {
        auto& side = param.sideIds[i];
        side.resize(param.numSides[i]);
        for (size_t j=0; j<param.numSides[i]; ++j) {
            is >> side[j];
        }
    }

    return is;
}


} /* namespace sses_share */
