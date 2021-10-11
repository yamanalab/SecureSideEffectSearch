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

#include <sses_share/sses_cli2srvparam.hpp>

namespace sses_share
{

std::ostream& operator<<(std::ostream& os, const C2SEnckeyParam& param)
{
    os << param.key_id << std::endl;
    os << param.context_stream_sz << std::endl;
    os << param.pubkey_stream_sz << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, C2SEnckeyParam& param)
{
    is >> param.key_id;
    is >> param.context_stream_sz;
    is >> param.pubkey_stream_sz;
    return is;
}

std::ostream& operator<<(std::ostream& os, const C2SQueryParam& param)
{
    os << param.comp_param << std::endl;
    os << param.encdata_stream_sz << std::endl;
    os << param.key_id << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, C2SQueryParam& param)
{
    is >> param.comp_param;
    is >> param.encdata_stream_sz;
    is >> param.key_id;
    return is;
}

std::ostream& operator<<(std::ostream& os, const C2SChunkResreqParam& param)
{
    os << param.query_id << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, C2SChunkResreqParam& param)
{
    is >> param.query_id;
    return is;
}

std::ostream& operator<<(std::ostream& os, const C2SResreqParam& param)
{
    os << param.query_id << std::endl;
    os << param.key_id << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, C2SResreqParam& param)
{
    is >> param.query_id;
    is >> param.key_id;
    return is;
}

std::ostream& operator<<(std::ostream& os, const C2SSelectedInfo& param)
{
    os << param.chunk_id << std::endl;
    os << param.pos_id << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, C2SSelectedInfo& param)
{
    is >> param.chunk_id;
    is >> param.pos_id;
    return is;
}
    
} /* namespace sses_share */
