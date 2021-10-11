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

#ifndef SSES_SERVER_STATE_HPP
#define SSES_SERVER_STATE_HPP

#include <cstdbool>
#include <memory>

#include <stdsc/stdsc_state.hpp>

namespace sses_server
{

/**
 * @brief Enumeration for state.
 */
enum StateId_t : int32_t
{
    kStateNil = 0,
    kStateConnected = 1,
    kStateReady = 2,
    kStateComputing = 3,
    kStateComputed = 4,
    kStateExit = 5,
};

/**
 * @brief Enumeration for events.
 */
enum Event_t : uint64_t
{
    kEventNil = 0,
    kEventEncryptionKey = 1,
    kEventQuery = 2,
    kEventChunkResult = 3,
    kEventResult = 4,
    kEventCancelQuery = 5,
};

/**
 * @brief Provides 'Connected' state.
 */
struct StateConnected : public stdsc::State
{
    static std::shared_ptr<State> create();
    StateConnected(void);
    virtual void set(stdsc::StateContext& sc, uint64_t event) override;
    STDSC_STATE_DEFID(kStateConnected);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

/**
 * @brief Provides 'Ready' state.
 */
struct StateReady : public stdsc::State
{
    static std::shared_ptr<State> create();
    StateReady(void);
    virtual void set(stdsc::StateContext& sc, uint64_t event) override;
    STDSC_STATE_DEFID(kStateReady);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

/**
 * @brief Provides 'Computing' state.
 */
struct StateComputing : public stdsc::State
{
    static std::shared_ptr<State> create();
    StateComputing(void);
    virtual void set(stdsc::StateContext& sc, uint64_t event) override;
    STDSC_STATE_DEFID(kStateComputing);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

/**
 * @brief Provides 'Computed' state.
 */
struct StateComputed : public stdsc::State
{
    static std::shared_ptr<State> create();
    StateComputed(void);
    virtual void set(stdsc::StateContext& sc, uint64_t event) override;
    STDSC_STATE_DEFID(kStateComputed);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace sses_server */

#endif /* SSES_SERVER_STATE_HPP */
