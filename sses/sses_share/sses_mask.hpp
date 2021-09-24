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

#ifndef SSES_MASK_HPP
#define SSES_MASK_HPP

#include <string>

namespace sses_share
{

/**
 * Compute mask
 * @param[in] age age
 * @param[in] gender gender
 */
size_t sses_mask_compute_mask(const size_t age, const std::string& gender);

} /* namespace sses_share */

#endif /* SSES_MASK_HPP */
