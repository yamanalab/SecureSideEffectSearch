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

#ifndef SSES_DEFINE_HPP
#define SSES_DEFINE_HPP

#define SSES_TIMEOUT_SEC (60)
#define SSES_RETRY_INTERVAL_USEC (2000000)

#define SSES_DEFAULT_MAX_CONCURRENT_QUERIES 128
#define SSES_DEFAULT_MAX_RESULTS 128
#define SSES_DEFAULT_MAX_RESULT_LIFETIME_SEC 50000

#define SSES_DEFAULT_SERVER_DB_SRC_FILEAPATH "demo.csv"
#define SSES_DEFAULT_SERVER_DB_BASE_DIR "."

#define SSES_DEFAULT_SIZE_OF_STR_FOR_TRANSDATA 2048

#define SSES_DEFAULT_NUM_THREADS 28

#endif /* SSES_DEFINE_HPP */
