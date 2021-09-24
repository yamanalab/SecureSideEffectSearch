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


#include <fstream>
#include <sstream>

#include <sses_share/sses_encdata.hpp>
#include <sses_share/sses_fhe_utility.hpp>
#include <sses_share/sses_utility.hpp>

#include <FHE.h>
#include <EncryptedArray.h>

namespace sses_share
{

namespace fhe_utility
{

void write_to_binary_stream(std::iostream& stream, void* base_ptr_in_stream,
                            const std::string& src_filepath, const bool shift_pos_in_stream)
{
    size_t size = utility::file_size(src_filepath);
    std::ifstream ifs(src_filepath, std::ios::binary);
    if (!ifs.is_open())
    {
        std::ostringstream oss;
        oss << "Failed to open. (" << src_filepath << ")";
        STDSC_THROW_FILE(oss.str());
    } else {
        auto* p = static_cast<char*>(base_ptr_in_stream) + stream.tellp();
        ifs.read(p, size);
    }
    
    if (shift_pos_in_stream)
    {
        stream.seekp(size, std::ios_base::cur);
    }
}

void read_from_binary_stream(std::iostream& stream, void* base_ptr_in_stream,
                             const size_t size,
                             const std::string& dst_filepath,
                             const bool shift_pos_in_stream)
{
    std::ofstream ofs(dst_filepath, std::ios::binary);
    if (!ofs.is_open())
    {
        std::ostringstream oss;
        oss << "Failed to open. (" << dst_filepath << ")";
        STDSC_THROW_FILE(oss.str());
    } else {
        auto* p = static_cast<char*>(base_ptr_in_stream) + stream.tellg();
        ofs.write(p, size);
    }

    if (shift_pos_in_stream)
    {
        stream.seekg(size, std::ios_base::cur);
    }
}

} /* namespace fhe_utility */

} /* namespace sses_share */
