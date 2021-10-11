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

#include <stdsc/stdsc_log.hpp>

#include <sses_server/sses_server_state.hpp>

namespace sses_server
{

// Connected

struct StateConnected::Impl
{
    Impl(void)
    {
    }

    void set(stdsc::StateContext& sc, uint64_t event)
    {
        STDSC_LOG_TRACE("StateConnected: event#%lu", event);
        switch (static_cast<Event_t>(event))
        {
            case kEventEncryptionKey:
                sc.next_state(StateReady::create());
                break;
            default:
                break;
        }
    }
};

std::shared_ptr<stdsc::State> StateConnected::create(void)
{
    return std::shared_ptr<stdsc::State>(new StateConnected());
}

StateConnected::StateConnected() : pimpl_(new Impl())
{
}

void StateConnected::set(stdsc::StateContext& sc, uint64_t event)
{
    pimpl_->set(sc, event);
}

// Ready

struct StateReady::Impl
{
    Impl(void)
    {
    }

    void set(stdsc::StateContext& sc, uint64_t event)
    {
        STDSC_LOG_TRACE("StateReady: event#%lu", event);
        switch (static_cast<Event_t>(event))
        {
            case kEventQuery:
                sc.next_state(StateComputing::create());
                break;
            default:
                break;
        }
    }
};

std::shared_ptr<stdsc::State> StateReady::create(void)
{
    return std::shared_ptr<stdsc::State>(new StateReady());
}

StateReady::StateReady() : pimpl_(new Impl())
{
}

void StateReady::set(stdsc::StateContext& sc, uint64_t event)
{
    pimpl_->set(sc, event);
}

// Computing

struct StateComputing::Impl
{
    Impl(void)
    {
    }

    void set(stdsc::StateContext& sc, uint64_t event)
    {
        STDSC_LOG_TRACE("StateComputing: event#%lu", event);
        switch (static_cast<Event_t>(event))
        {
            case kEventChunkResult:
                sc.next_state(StateComputed::create());
                break;
            case kEventCancelQuery:
                sc.next_state(StateReady::create());
                break;
            default:
                break;
        }
    }
};

std::shared_ptr<stdsc::State> StateComputing::create(void)
{
    return std::shared_ptr<stdsc::State>(new StateComputing());
}

StateComputing::StateComputing() : pimpl_(new Impl())
{
}

void StateComputing::set(stdsc::StateContext& sc, uint64_t event)
{
    pimpl_->set(sc, event);
}

// Computed

struct StateComputed::Impl
{
    Impl(void)
    {
    }

    void set(stdsc::StateContext& sc, uint64_t event)
    {
        STDSC_LOG_TRACE("StateComputed: event#%lu", event);
        switch (static_cast<Event_t>(event))
        {
            case kEventCancelQuery:
            case kEventResult:
                sc.next_state(StateReady::create());
                break;
            default:
                break;
        }
    }
};

std::shared_ptr<stdsc::State> StateComputed::create(void)
{
    return std::shared_ptr<stdsc::State>(new StateComputed());
}

StateComputed::StateComputed() : pimpl_(new Impl())
{
}

void StateComputed::set(stdsc::StateContext& sc, uint64_t event)
{
    pimpl_->set(sc, event);
}

} /* namespace sses_server */
