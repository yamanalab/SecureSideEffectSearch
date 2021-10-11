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


#include <sses_share/sses_utility.hpp>
#include <sses_share/sses_fhekey_container.hpp>
#include <sses_server/sses_server_query.hpp>

namespace sses_server
{
    
Query::Query(const int32_t key_id,
             const sses_share::ComputationParam& param,
             const sses_share::FHECtxtBuffer& encmask,
             sses_share::FHEKeyContainer* key_container_p,
             DB* db_p)
    : key_id_(key_id),
      param_(param),
      encmask_(encmask),
      key_container_p_(key_container_p),
      db_p_(db_p)
{
    key_container_p->setup(key_id);
    auto context = key_container_p->get_context(key_id);
    FHEPubKey pubkey(context);
    key_container_p->get(key_id, sses_share::KeyKind_t::kKindPubKey, pubkey);
}

int32_t QueryQueue::push(const Query& data)
{
    auto id = sses_share::utility::gen_uuid();
    super::push(id, data);
    return id;
}

} /* namespace sses_server */
