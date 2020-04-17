#pragma once

#include <functional>
#include "picojson.h"

/**
 * Wrapper class for generating json by picojson
 */
class Json
{
    picojson::object obj_;

public:
    operator picojson::object()
    {
        return this->obj_;
    }
    operator picojson::value()
    {
        return picojson::value(this->obj_);
    }

    picojson::object getObject()
    {
        return *this;
    }
    picojson::value toValue()
    {
        return *this;
    }

    inline void add(const std::string& key, const int& value)
    {
        Json::add(obj_, key, (double)value);
    }

    inline void add(const std::string& key, const unsigned int& value)
    {
        Json::add(obj_, key, (double)value);
    }

    inline void add(const std::string& key, const float& value)
    {
        Json::add(obj_, key, (double)value);
    }

    inline void add(const std::string& key, const bool& value)
    {
        obj_.insert(std::make_pair(key.c_str(), picojson::value(value)));
    }

    inline void add(const std::string& key, const std::string& value)
    {
        obj_.insert(std::make_pair(key.c_str(), picojson::value(value)));
    }

    inline void add(const std::string& key, Json& child)
    {
        obj_.insert(std::make_pair(key.c_str(), child.toValue()));
    }

    template <class T>
    void add(const std::string& key, T& value,
             const std::function<Json(T)>& convert)
    {
        obj_.insert(std::make_pair(key.c_str(), convert(value).toValue()));
    }

    template <class T>
    void add(const std::string& key, const std::vector<T>& list,
             const std::function<Json(T)>& convert)
    {
        picojson::array jsonList;
        for (const auto& item : list)
        {
            jsonList.push_back(convert(item).toValue());
        }
        obj_.insert(std::make_pair(key.c_str(), picojson::value(jsonList)));
    }

    void add(const std::string& key, const std::vector<int>& list)
    {
        picojson::array jsonList;
        for (const auto& item : list)
        {
            jsonList.push_back(picojson::value(static_cast<double>(item)));
        }
        obj_.insert(std::make_pair(key.c_str(), picojson::value(jsonList)));
    }

    std::string serialize()
    {
        return this->toValue().serialize();
    }

    inline static void add(picojson::object& obj, const std::string& key,
                           const double& value)
    {
        obj.insert(std::make_pair(key.c_str(), value));
    }
};
