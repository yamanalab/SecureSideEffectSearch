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

#ifndef SSES_SERVER_CALLBACK_FUNCTION_HPP
#define SSES_SERVER_CALLBACK_FUNCTION_HPP

#include <stdsc/stdsc_callback_function.hpp>

namespace sses_server
{

/**
 * @brief Provides callback function in receiving encryption keys.
 */
DECLARE_DATA_CLASS(CallbackFunctionEncryptionKeys);

/**
 * @brief Provides callback function in receiving query.
 */
DECLARE_UPDOWNLOAD_CLASS(CallbackFunctionQuery);

/**
 * @brief Provides callback function in receiving chunk result request.
 */
DECLARE_UPDOWNLOAD_CLASS(CallbackFunctionChunkResultRequest);

/**
 * @brief Provides callback function in receiving result request.
 */
DECLARE_UPDOWNLOAD_CLASS(CallbackFunctionResultRequest);

/**
 * @brief Provides callback function in receiving cancel query.
 */
DECLARE_DATA_CLASS(CallbackFunctionCancelQuery);


} /* namespace sses_server */

#endif /* SSES_SERVER_CALLBACK_FUNCTION_HPP */
