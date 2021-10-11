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

#ifndef SSES_SEAL_UTILITY_HPP
#define SSES_SEAL_UTILITY_HPP

#include <string>
#include <vector>

namespace sses_share
{

namespace fhe_utility
{

void write_to_binary_stream(std::iostream& stream, void* base_ptr_in_stream,
                            const std::string& src_filepath,
                            const bool shift_pos_in_stream = true);

void read_from_binary_stream(std::iostream& stream, void* base_ptr_in_stream,
                             const size_t size,
                             const std::string& dst_filepath,
                             const bool shift_pos_in_stream = true);

} /* namespace fhe_utility */

} /* namespace sses_share */

#endif /* SSES_SEAL_UTILITY_HPP */
