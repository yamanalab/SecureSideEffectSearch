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

#ifndef SSES_SRV2CLIPARAM_HPP
#define SSES_SRV2CLIPARAM_HPP

#include <iostream>
#include <vector>

namespace sses_share
{

/**
 * @brief Enumeration for result status of server.
 */
enum ServerResultStatus_t : int32_t
{
    kServerResultStatusNil = -1,
    kServerResultStatusFailed = 0,
    kServerResultStatusSuccess = 1,
};

/**
 * @brief This class is used to hold the results for each chunk sent from server to client.
 */
struct S2CChunkResultParam
{
    ServerResultStatus_t status;
    int32_t key_id;
};

std::ostream& operator<<(std::ostream& os, const S2CChunkResultParam& param);
std::istream& operator>>(std::istream& is, S2CChunkResultParam& param);

/**
 * @brief This class is used to hold the results for each chunk sent from server to client.
 */
struct S2CResultParam
{
    size_t numRes;
    std::vector<size_t> numMeds;
    std::vector<size_t> numSides;
    
    std::vector<int32_t> recordIds;
    std::vector<std::vector<int32_t>> medIds;
    std::vector<std::vector<int32_t>> sideIds;
};

std::ostream& operator<<(std::ostream& os, const S2CResultParam& param);
std::istream& operator>>(std::istream& is, S2CResultParam& param);
    

} /* namespace sses_share */

#endif /* SSES_SRV2CLIPARAM_HPP */
