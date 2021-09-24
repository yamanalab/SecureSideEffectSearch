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

#define _POSIX_SOURCE // for mkdir
#include <sys/stat.h> // for mkdir

#include <unordered_map>
#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <map>

#include "FHE.h"
#include "EncryptedArray.h"

#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>

#include <sses_share/sses_utility.hpp>
#include <sses_share/sses_computation_param.hpp>
#include <sses_share/sses_mask.hpp>
#include <sses_share/sses_types.hpp>

#include <sses_server/sses_server_db.hpp>

#define ENABLE_LOCAL_DEBUG
#ifdef ENABLE_LOCAL_DEBUG
#include <sses_share/sses_fhe_debug.hpp>
#endif

namespace sses_server
{

static constexpr char* DEFAULT_DBBASIC_FILENAME = (char*)"dbbasics.bin";
static constexpr char* DEFAULT_MEDINV_FILENAME = (char*)"med.inv";
static constexpr char* DEFAULT_SIDEINV_FILENAME = (char*)"side.inv";
static constexpr char* DEFAULT_ENCDATA_DIRNAME = (char*)"encdata";
static constexpr char* DEFAULT_AUXDATA_DIRNAME = (char*)"auxdata";
    

namespace csvcolumns
{
    constexpr int ID_IDX = 0;
    constexpr int MEDICINE_ID_IDX = 1;
    constexpr int SYMPTOM_ID_IDX = 2;
    constexpr int AGE_IDX = 8;
    constexpr int GENDER_IDX = 9;
}

struct Record
{
    int maskValue;
    std::set<size_t> medicineIds;
    std::set<size_t> symptomIds;
};

DBBasicFile::DBBasicFile()
    : dbstatus(false),
      totalRecordsNum(0),
      totalMedicinesNum(0),
      totalSymptomsNum(0)
{}

DBBasicFile::DBBasicFile(std::string& filepath)
    : dbstatus(false),
      totalRecordsNum(0),
      totalMedicinesNum(0),
      totalSymptomsNum(0)
{
    try {
        load_from_file(filepath);
    } catch (const stdsc::AbstractException& e) {
        STDSC_LOG_WARN(e.what());
    }
}
    
void DBBasicFile::write_to_file(const std::string& filepath,
                                const size_t totalRecordsNum,
                                const size_t totalMedicinesNum,
                                const size_t totalSymptomsNum)
{
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs.is_open())
    {
        std::ostringstream oss;
        oss << "failed to open. (" << filepath << ")";
        STDSC_THROW_FILE(oss.str());
    }
    ofs << 1 << std::endl;
    ofs << totalRecordsNum << std::endl;
    ofs << totalMedicinesNum << std::endl;
    ofs << totalSymptomsNum << std::endl;
}

void DBBasicFile::load_from_file(const std::string& filepath)
{
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open())
    {
        std::ostringstream oss;
        oss << "failed to open. (" << filepath << ")";
        STDSC_THROW_FILE(oss.str());
    }
    ifs >> dbstatus;
    ifs >> totalRecordsNum;
    ifs >> totalMedicinesNum;
    ifs >> totalSymptomsNum;
}

bool DBBasicFile::status() const
{
    return dbstatus;
}

std::string DBBasicFile::to_string() const
{
    std::ostringstream oss;
    oss << totalRecordsNum << ", ";
    oss << totalMedicinesNum << ", ";
    oss << totalSymptomsNum;
    return oss.str();
}
    
struct DatasetInfo
{
    explicit DatasetInfo(std::string& dir)
        : dir_(dir)
    {}
    virtual ~DatasetInfo() = default;

    bool is_enable() const
    {
        bool ret = false;
        if (sses_share::utility::dir_exist(dir_)) {
            
            auto filepath = dbbasic_filepath();
            DBBasicFile dbbasic(filepath);
            if (dbbasic.status()) {
                ret = true;
            }
            
        }
        return ret;
    }

    const std::string dbbasic_filepath() const
    {
        std::ostringstream oss;
        oss << dir_ << "/";
        oss << DEFAULT_DBBASIC_FILENAME;
        return oss.str();
    }

    const std::string medinv_filepath() const
    {
        auto dir = auxdata_dirpath() + "/";
        return dir + DEFAULT_MEDINV_FILENAME;
    }

    const std::string sideinv_filepath() const
    {
        auto dir = auxdata_dirpath() + "/";
        return dir + DEFAULT_SIDEINV_FILENAME;
    }
    
    const std::string encdata_dirpath() const
    {
        std::ostringstream oss;
        oss << dir_ << "/";
        oss << DEFAULT_ENCDATA_DIRNAME;
        return oss.str();
    }

    const std::string auxdata_dirpath() const
    {
        std::ostringstream oss;
        oss << dir_ << "/";
        oss << DEFAULT_AUXDATA_DIRNAME;
        return oss.str();
    }
    
    std::string dir_;
};
    
struct DB::Impl
{
    static constexpr char* LIST_FILENAME = (char*)"list.txt";
    
    Impl(const std::string& db_basedir)
        : db_basedir_(db_basedir)
    {
        {
            std::ostringstream oss;
            oss << db_basedir << "/" << LIST_FILENAME;
            list_filepath_ = oss.str();
        }
        
        load_listfile(list_filepath_);

        std::ostringstream oss;
        for (auto m : map_) {
            auto key_id = m.first;
            auto dsinfo = m.second;
            oss << "{key_id:" << key_id;
            oss << ", dir:" << dsinfo.dir_.c_str();
            oss << ", dbstatus:" << dsinfo.is_enable();
            oss << "} ";
        }
        STDSC_LOG_INFO("Initialized DB : %s", oss.str().c_str());
    }

    bool is_enable(const int32_t key_id) const
    {
        bool ret = false;
        if (map_.count(key_id) > 0) {
            auto& dsinfo = map_.at(key_id);
            ret = dsinfo.is_enable();
        }
        return ret;
    }
    
    void setup(const int32_t key_id,
               const std::string& db_src_filepath,
               const FHEcontext& context,
               const FHEPubKey& pubkey)
    {
        if (is_enable(key_id)) {
            return;
        }

        std::string top_dir = db_basedir_ + "/db_" + std::to_string(key_id);

        if (sses_share::utility::dir_exist(top_dir)) {
            sses_share::utility::remove_dir(top_dir);
            STDSC_LOG_INFO("Remove directory : %s", top_dir.c_str());
        }

        STDSC_LOG_INFO("Creating new DB. [In:%s, Out: %s]",
                       db_src_filepath.c_str(),
                       top_dir.c_str());
        
        STDSC_THROW_FILE_IF_CHECK(mkdir(top_dir.c_str(), S_IRWXU) == 0,
                                  "Err: failed to create DB directory");
        auto encdata_dir = top_dir + "/encdata";
        STDSC_THROW_FILE_IF_CHECK(mkdir(encdata_dir.c_str(), S_IRWXU) == 0,
                                  "Err: failed to create DB directory");
        auto auxdata_dir = top_dir + "/auxdata";
        STDSC_THROW_FILE_IF_CHECK(mkdir(auxdata_dir.c_str(), S_IRWXU) == 0,
                                  "Err: failed to create DB directory");

        std::ifstream csvIfs(db_src_filepath);
        std::string line;
        std::map<size_t, Record> records;
        std::set<size_t> totalMedicines;
        std::set<size_t> totalSymptoms;

        std::vector<std::string> record;
        getline(csvIfs, line); // header
        while (getline(csvIfs, line))
        {
            boost::algorithm::split(record, line, boost::is_any_of(","));
            size_t id = stoi(record[csvcolumns::ID_IDX]);
            size_t medicineId = stoi(record[csvcolumns::MEDICINE_ID_IDX]);
            size_t symptomId = stoi(record[csvcolumns::SYMPTOM_ID_IDX]);
            totalMedicines.insert(medicineId);
            totalSymptoms.insert(symptomId);
            if (records.count(id) == 0)
            {
                int age = stoi(record[csvcolumns::AGE_IDX]);
                assert(age >= 0 && age <= 122);
                int genderData = stoi(record[csvcolumns::GENDER_IDX]);
                assert(genderData == 1 || genderData == 2 || genderData == 3);
                auto gender = sses_share::GENDER_INT_MAP.at(genderData);
                int mask = age + static_cast<int>(gender) * 128 + 5;
                std::set<size_t> medicineIds;
                medicineIds.insert(medicineId);
                std::set<size_t> symptomIds;
                symptomIds.insert(symptomId);
                records.insert(
                    std::make_pair(id, Record{mask, medicineIds, symptomIds}));
            }
            else
            {
                records.at(id).medicineIds.insert(medicineId);
                records.at(id).symptomIds.insert(symptomId);
            }
        }
        csvIfs.close();

        auto totalRecordsNum = records.size();
        auto totalMedicinesNum = totalMedicines.size();
        auto totalSymptomsNum = totalSymptoms.size();        

        auto dbbasicfilepath = top_dir + "/" + std::string(DEFAULT_DBBASIC_FILENAME);
        auto medinvfilepath = auxdata_dir + "/" + std::string(DEFAULT_MEDINV_FILENAME);
        auto sideinvfilepath = auxdata_dir + "/" + std::string(DEFAULT_SIDEINV_FILENAME);
        
        DBBasicFile dbbasicfile;
        dbbasicfile.write_to_file(dbbasicfilepath,
                                  totalRecordsNum,
                                  totalMedicinesNum,
                                  totalSymptomsNum);
        STDSC_LOG_INFO("Created DBBasic file. [%s]", dbbasicfilepath.c_str());

        std::ofstream fmedinv(medinvfilepath, std::ios::binary);
        fmedinv << totalMedicinesNum << std::endl;
        for (const auto& medicineId : totalMedicines) {
            fmedinv << medicineId << ":" << 0 << std::endl;
        }
        fmedinv.close();

        std::ofstream fsideinv(sideinvfilepath, std::ios::binary);
        fsideinv << totalSymptomsNum << std::endl;
        for (const auto& symptomId : totalSymptoms) {
            fsideinv << symptomId << ":" << 0 << std::endl;
        }
        fsideinv.close();

        NTL::ZZX G = context.alMod.getFactorsOverZZ()[0];
        EncryptedArray ea(context, G);

        STDSC_LOG_INFO("Start generating DB data.");
        
        for (const auto& recordPair : records)
        {
            size_t recordId = recordPair.first;
            Record record = recordPair.second;
            int mask = record.maskValue;

             std::string encfilepath = encdata_dir + "/" + std::to_string(recordId) + ".bin";
            std::string auxfilepath = auxdata_dir + "/" + std::to_string(recordId) + ".bin";

            STDSC_LOG_INFO("  recID:%lu, numMed:%lu, numSide:%lu",
                           recordId,
                           record.medicineIds.size(),
                           record.symptomIds.size());

            Ctxt encmask(pubkey);
            pubkey.Encrypt(encmask, NTL::to_ZZX(mask));
            {
                std::ofstream ofs(encfilepath, std::ios::binary);
                ofs << encmask;
                STDSC_LOG_INFO("    | %s", encfilepath.c_str());
            }

            std::vector<size_t> meds, sides;
            for (const auto v : record.medicineIds) {
                meds.push_back(v);
            }
            for (const auto v : record.symptomIds) {
                sides.push_back(v);
            } 

            {
                std::ofstream ofs(auxfilepath, std::ios::binary);
                ofs << "Medicine: [" << meds[0];
                for (size_t i = 1; i < meds.size(); ++i) {
                    ofs << ", " << meds[i];
                }
                ofs << "]" << std::endl;
                ofs << "Side Effect: [" << sides[0];
                for (size_t i = 1; i < sides.size(); ++i) {
                    ofs << ", " << sides[i];
                }
                ofs << "]" << std::endl;
                STDSC_LOG_INFO("    | %s", auxfilepath.c_str());
            }

            update(recordId,
                   record.medicineIds.size(), meds,
                   record.symptomIds.size(), sides,
                   auxfilepath,
                   medinvfilepath,
                   sideinvfilepath);
            STDSC_LOG_INFO("    | %s", medinvfilepath.c_str());
            STDSC_LOG_INFO("    | %s", sideinvfilepath.c_str());
        }

        STDSC_LOG_INFO("Finish generating DB data.");

        map_.emplace(key_id, DatasetInfo(top_dir));
        save_listfile(list_filepath_);
        STDSC_LOG_INFO("Updated List file. [%s]", list_filepath_.c_str());
    }

    std::string dbbasic_filepath(const int32_t key_id) const
    {
        return map_.at(key_id).dbbasic_filepath();
    }

    std::string medinv_filepath(const int32_t key_id) const
    {
        return map_.at(key_id).medinv_filepath();
    }

    std::string sideinv_filepath(const int32_t key_id) const
    {
        return map_.at(key_id).sideinv_filepath();
    }

    std::string encdata_dirpath(const int32_t key_id) const
    {
        return map_.at(key_id).encdata_dirpath();
    }
    
    std::string auxdata_dirpath(const int32_t key_id) const
    {
        return map_.at(key_id).auxdata_dirpath();
    }
    
private:
    
    void load_listfile(const std::string& filepath)
    {
        std::ifstream ifs(filepath);
        if (ifs.is_open()) {
            size_t n;
            ifs >> n;

            for (size_t i=0; i<n; ++i) {
                std::string line;
                ifs >> line;
                
                std::vector<std::string> elems;
                boost::algorithm::split(elems, line, boost::is_any_of(","));

                map_.emplace(std::stoi(elems[0]), DatasetInfo(elems[1]));
            }
        }
    }

    void save_listfile(const std::string& filepath)
    {
        std::ofstream ofs(filepath);
        if (ofs.is_open()) {
            ofs << map_.size() << std::endl;

            for (const auto& v : map_) {
                ofs << v.first;
                ofs << ",";
                ofs << v.second.dir_;
                ofs << std::endl;
            }
        }
    }

    void update(const size_t recordId,
                const size_t numMed,
                const std::vector<size_t>& meds,
                const size_t numSide,
                const std::vector<size_t>& sides,
                const std::string& auxfilepath,
                const std::string& medinvfilepath,
                const std::string& sideinvfilepath)
    {
        std::map<int, std::vector<int>> medIndex;
        std::map<int, std::vector<int>> sideIndex;

        std::ofstream faux(auxfilepath.c_str(), std::ios::binary);
        faux << "Medicine: [" << meds[0];
        for (size_t i = 1; i < numMed; ++i) {
            faux << ", " << meds[i];
        }
        faux << "]" << std::endl;
        faux << "Side Effect: [" << sides[0];
        for (size_t i = 1; i < numSide; ++i) {
            faux << ", " << sides[i];
        }
        faux << "]" << std::endl;
        faux.close();
        
        
        std::ifstream findexmed(medinvfilepath, std::ios::binary);
        std::string line;
        std::vector<std::string> medInfo;
        int numMedDict;
        findexmed >> numMedDict;
        for (int i = 0; i < numMedDict; ++i)
        {
            findexmed >> line;
            boost::algorithm::split(medInfo, line, boost::is_any_of(":"));
            int medId = std::stoi(medInfo[0]);
            int numindex = std::stoi(medInfo[1]);
            std::vector<int> tempindex;
            while (numindex--)
            {
                int temprec;
                findexmed >> temprec;
                tempindex.push_back(temprec);
            }
            medIndex.insert(std::make_pair(medId, tempindex));
        }
        findexmed.close();

        std::ifstream findexside(sideinvfilepath, std::ios::binary);
        std::vector<std::string> sideInfo;
        int numSideDict;
        findexside >> numSideDict;
        for (int i = 0; i < numSideDict; ++i)
        {
            findexside >> line;
            boost::algorithm::split(sideInfo, line, boost::is_any_of(":"));
            int sideId = std::stoi(sideInfo[0]);
            int numindex = std::stoi(sideInfo[1]);
            std::vector<int> tempindex;
            while (numindex--)
            {
                int temprec;
                findexside >> temprec;
                tempindex.push_back(temprec);
            }
            sideIndex.insert(make_pair(sideId, tempindex));
        }
        findexside.close();

        for (size_t i = 0; i < numMed; ++i) {
            medIndex[meds[i]].push_back(recordId);
        }
        for (size_t i = 0; i < numSide; ++i) {
            sideIndex[sides[i]].push_back(recordId);
        }
        STDSC_LOG_TRACE("Updated rev index");

        std::ofstream findexmedw(medinvfilepath, std::ios::binary);
        findexmedw << numMedDict << std::endl;

        for (const auto& medIdxPair : medIndex)
        {
            int medId = medIdxPair.first;
            std::vector<int> recordIds = medIdxPair.second;
            findexmedw << medId << ":" << recordIds.size() << std::endl;
            for (const int& recordId : recordIds)
            {
                findexmedw << recordId << std::endl;
            }
        }
        findexmedw.close();

        std::ofstream findexsidew(sideinvfilepath, std::ios::binary);
        findexsidew << numSideDict << std::endl;
        for (const auto& sideIdxPair : sideIndex)
        {
            int sideId = sideIdxPair.first;
            std::vector<int> recordIds = sideIdxPair.second;
            findexsidew << sideId << ":" << recordIds.size() << std::endl;
            for (const int& recordId : recordIds)
            {
                findexsidew << recordId << std::endl;
            }
        }
        findexsidew.close();

        STDSC_LOG_TRACE("New record id: %lu", recordId);
        STDSC_LOG_TRACE("DB successfully updated.");
    }

private:
    std::string db_basedir_;
    std::string list_filepath_;
    std::unordered_map<int32_t, DatasetInfo> map_;
};
    
DB::DB(const std::string& db_basedir)
  : pimpl_(new Impl(db_basedir))
{}

bool DB::is_enable(const int32_t key_id) const
{
    return pimpl_->is_enable(key_id);
}
    
void DB::setup(const int32_t key_id,
               const std::string& db_src_filepath,
               const FHEcontext& context,
               const FHEPubKey& pubkey)
{
    pimpl_->setup(key_id, db_src_filepath, context, pubkey);
}

std::string DB::dbbasic_filepath(const int32_t key_id) const
{
    return pimpl_->dbbasic_filepath(key_id);
}

std::string DB::medinv_filepath(const int32_t key_id) const
{
        return pimpl_->medinv_filepath(key_id);
}

std::string DB::sideinv_filepath(const int32_t key_id) const
{
        return pimpl_->sideinv_filepath(key_id);
}

std::string DB::encdata_dirpath(const int32_t key_id) const
{
    return pimpl_->encdata_dirpath(key_id);
}

std::string DB::auxdata_dirpath(const int32_t key_id) const
{
    return pimpl_->auxdata_dirpath(key_id);
}


} /* namespace sses_server */
