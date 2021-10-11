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

#ifndef SSES_TYPES_HPP
#define SSES_TYPES_HPP

#include <string>
#include <unordered_map>

namespace sses_share
{

/**
 * @brief Enumeration for gender.
 */
enum Gender_t : size_t
{
    kGenderMale = 0,
    kGenderFemale = 1,
    kGenderOther = 2
};

/**
 * @brief Table of gender
 */
const std::unordered_map<size_t, Gender_t> GENDER_INT_MAP {
    {1, Gender_t::kGenderMale},
    {2, Gender_t::kGenderFemale},
    {3, Gender_t::kGenderOther}};

/**
 * @brief Table of gender
 */
const std::unordered_map<std::string, Gender_t> GENDER_CHAR_MAP {
    {"m", Gender_t::kGenderMale},
    {"f", Gender_t::kGenderFemale},
    {"o", Gender_t::kGenderOther}};

} /* namespace sses_share */

#endif /* SSES_TYPES_HPP */
