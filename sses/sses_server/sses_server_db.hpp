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

#ifndef SSES_SERVER_DB_HPP
#define SSES_SERVER_DB_HPP

#include <memory>
#include <string>

class FHEcontext;
class FHEPubKey;

namespace sses_server
{

/**
 * @brief This class is used to hold the basic data, medicine data, and side effect data.
 */
class DB
{
public:
    /**
     * Constructor
     * @param[in] db_basedir DB base directory
     */
    DB(const std::string& db_basedir);
    virtual ~DB() = default;

    /**
     * Whether the DB is valid or not
     * @param[in] key_id key ID
     * @return whether the DB is valid or not
     */
    bool is_enable(const int32_t key_id) const;
    
    /**
     * Setup DB
     * @param[in] key_id key ID
     * @param[in] db_src_filepath DB source filepath
     * @param[in] context FHE Context
     * @param[in] pubkey FHE Publickey
     */
    void setup(const int32_t key_id,
               const std::string& db_src_fiilepath,
               const FHEcontext& context,
               const FHEPubKey& pubkey);

    /**
     * Get DBBasic filepath
     * @param[in] key_id key ID
     * @return DBBasic filepath
     */
    std::string dbbasic_filepath(const int32_t key_id) const;

    /**
     * Get MED INV filepath
     * @param[in] key_id key ID
     * @return MED INV filepath
     */
    std::string medinv_filepath(const int32_t key_id) const;

    /**
     * Get SIDE INV filepath
     * @param[in] key_id key ID
     * @return SIDE INV filepath
     */
    std::string sideinv_filepath(const int32_t key_id) const;

    /**
     * Get EncData dirpath
     * @param[in] key_id key ID
     * @return EncData dirpath
     */
    std::string encdata_dirpath(const int32_t key_id) const;
    
    /**
     * Get AuxData dirpath
     * @param[in] key_id key ID
     * @return AuxData dirpath
     */
    std::string auxdata_dirpath(const int32_t key_id) const;
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

/**
 * @brief This class is used to hold the basic data.
 */
struct DBBasicFile
{
    DBBasicFile();
    /**
     * Constructor
     * @param[in] filepath filepath
     */
    explicit DBBasicFile(std::string& filepath);

    /**
     * Write to file
     * @param[in] filepath filepath
     * @param[in] totalRecordsNum total record number
     * @param[in] totalMedicinesNum total medicines number
     * @param[in] totalSymptomsNum total symptoms number
     */
    void write_to_file(const std::string& filepath,
                       const size_t totalRecordsNum,
                       const size_t totalMedicinesNum,
                       const size_t totalSymptomsNum);

    /**
     * Load from file
     * @param[in] filepath filepath
     */
    void load_from_file(const std::string& filepath);

    /**
     * Status
     * @return status
     */
    bool status() const;

    /**
     * Stringfy
     * @return string of parameter
     */
    std::string to_string() const;
    
    bool dbstatus;
    size_t totalRecordsNum;
    size_t totalMedicinesNum;
    size_t totalSymptomsNum;
};


} /* namespace sses_server */

#endif /* SSES_SERVER_DB_HPP */
