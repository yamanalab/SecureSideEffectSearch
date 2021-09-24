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

#ifndef SSES_CONCURRENT_MAPQUEUE_HPP
#define SSES_CONCURRENT_MAPQUEUE_HPP

#include <cstdbool>
#include <map>
#include <mutex>

#include <stdsc/stdsc_exception.hpp>

namespace sses_share
{

/**
 * @brief This class is Map with exclusivity
 */
template <class Tk, class Tv>
class ConcurrentMapQueue
{
public:
    
    ConcurrentMapQueue() = default;
    virtual ~ConcurrentMapQueue() = default;

    /**
     * Push data
     * @param[in] key key
     * @param[in] val value
     */
    virtual void push(const Tk& key, const Tv& val)
    {
        STDSC_THROW_INVPARAM_IF_CHECK(!map_.count(key),
                                      "key has already exist.");
        std::lock_guard<std::mutex> lock(mtx_);
        map_.emplace(key, val);
    }

    /**
     * Size
     * @return map size
     */
    virtual size_t size() const
    {
        return map_.size();
    }

    /**
     * Return iterator begin
     * @return iterator begin
     */
    virtual typename std::map<Tk, Tv>::iterator begin()
    {
        return map_.begin();
    }

    /**
     * Return iterator end
     * @return iterator end
     */
    virtual typename std::map<Tk, Tv>::iterator end()
    {
        return map_.end();
    }

    /**
     * Number of elements in the value for the key
     * @param[in] key key
     * @return Number of elements
     */
    virtual size_t count(const Tk& key) const
    {
        return map_.count(key);
    }

    /**
     * Pop data
     * @param[out] key key
     * @param[out] val value
     * @return Susscess or Fail
     * @note The elements will be removed from the queue
     */
    virtual bool pop(Tk& key, Tv& val)
    {
        std::lock_guard<std::mutex> lock(mtx_);

        if (0 == map_.size())
        {
            return false;
        }

        const auto front = map_.begin();
        key = front->first;
        val = front->second;
        map_.erase(front);
        return true;
    }

    /**
     * Pop data
     * @param[out] key key
     * @param[out] val value
     * @return Susscess or Fail
     * @note The elements will be removed from the queue
     */
    virtual bool pop(const Tk& key, Tv& val)
    {
        std::lock_guard<std::mutex> lock(mtx_);

        if (0 == map_.size() || 0 == map_.count(key))
        {
            return false;
        }

        val = map_.at(key);
        map_.erase(key);

        return true;
    }

    /**
     * Get data
     * @param[out] key key
     * @param[out] val value
     * @return Susscess or Fail
     */
    virtual bool get(const Tk& key, Tv& val)
    {
        std::lock_guard<std::mutex> lock(mtx_);

        if (0 == map_.size() || 0 == map_.count(key))
        {
            return false;
        }

        val = map_.at(key);
        return true;
    }

private:
    std::map<Tk, Tv> map_;
    std::mutex mtx_;
};

} /* namespace sses_share */

#endif /* SSES_CONCURRENT_MAPQUEUE_HPP */
