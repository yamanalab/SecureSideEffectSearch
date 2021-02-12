#include <NTL/BasicThreadPool.h>
#include <NTL/ZZ.h>
#include <NTL/lzz_pXFactoring.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <thread>

#include "EncryptedArray.h"
#include "FHE.h"
#include "filepath_info.h"
#include "timing.h"
using namespace std;

NTL_CLIENT

// Read from csv data
// make encrypted and call update

struct Record
{
    size_t maskValue;
    std::set<size_t> medicineIds;
    std::set<size_t> symptomIds;
};

size_t calcMaskValue(const size_t& age, const size_t& gender)
{
    assert(0 <= age && age <= 117);
    assert(gender == 0 || gender == 1);
    return age + gender * 128 + 5;
}

int main(int argc, char* argv[])
{
    assert(argc == 2);
    string csvFilepath(argv[1]);
    ifstream csvIfs(csvFilepath);
    string line;
    map<size_t, Record> records;
    std::set<size_t> totalMedicines;
    std::set<size_t> totalSymptoms;

    vector<string> record;
    getline(csvIfs, line); // 1行目のヘッダ行
    while (getline(csvIfs, line))
    {
        boost::algorithm::split(record, line, boost::is_any_of(","));
        size_t id = stoi(record[0]);
        size_t medicineId = stoi(record[1]);
        size_t symptomId = stoi(record[3]);
        totalMedicines.insert(medicineId);
        totalSymptoms.insert(symptomId);
        if (records.count(id) == 0)
        {
            size_t age = stoi(record[11]);
            // <Gender> Female(f): 0, Male(m): 1
            size_t gender = 1;
            if (record[12] == "f")
            {
                gender = 0;
            }
            size_t mask = calcMaskValue(age, gender);
            std::set<size_t> medicineIds;
            medicineIds.insert(medicineId);
            std::set<size_t> symptomIds;
            symptomIds.insert(symptomId);
            records.insert(
              make_pair(id, Record{mask, medicineIds, symptomIds}));
        }
        else
        {
            records.at(id).medicineIds.insert(medicineId);
            records.at(id).symptomIds.insert(symptomId);
        }
    }
    csvIfs.close();

    size_t totalRecordsNum = records.size();
    size_t totalMedicinesNum = totalMedicines.size();
    size_t totalSymptomsNum = totalSymptoms.size();

    ofstream fdbbasics(DBBASICS_FILE_PATH, std::ios::binary);
    fdbbasics << 1 << endl;                 // DB status
    fdbbasics << totalRecordsNum << endl;   // number of records
    fdbbasics << totalMedicinesNum << endl; // number of medicines
    fdbbasics << totalSymptomsNum << endl;  // number of side effects
    fdbbasics.close();

    ofstream fmedinv(MED_INV_FILE_PATH, std::ios::binary);
    fmedinv << totalMedicinesNum << endl;
    for (const auto& medicineId : totalMedicines)
        fmedinv << medicineId << ":" << 0 << endl;
    fmedinv.close();

    ofstream fsideinv(SIDE_INV_FILE_PATH, std::ios::binary);
    fsideinv << totalSymptomsNum << endl;
    for (const auto& symptomId : totalSymptoms)
        fsideinv << symptomId << ":" << 0 << endl;
    fsideinv.close();

    ifstream fctxt(FHE_CONTEXT_FILE_PATH, std::ios::binary);
    unsigned long m, p, r;
    std::vector<long> gens, ords;
    readContextBase(fctxt, m, p, r, gens, ords);
    FHEcontext context(m, p, r, gens, ords);
    assert(fctxt >> context);
    fctxt.close();
    // read context from context.bin

    ZZX G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    // generate ea from context

    FHEPubKey publicKey(context);
    ifstream fpkey(FHE_PK_FILE_PATH, std::ios::binary);
    assert(fpkey >> publicKey);
    fpkey.close();
    // read publicKey from pk.bin

    for (const auto& recordPair : records)
    {
        size_t recordId = recordPair.first;
        Record record = recordPair.second;
        size_t mask = record.maskValue;
        size_t medCount = record.medicineIds.size();
        size_t sideCount = record.symptomIds.size();

        Ctxt encmask(publicKey);
        publicKey.Encrypt(encmask, to_ZZX(mask));
        ofstream fenc("enc_temp.bin", std::ios::binary);
        fenc << encmask;
        fenc.close();

        string sysorder = "./update " + to_string(recordId) + " enc_temp.bin " +
                          to_string(record.medicineIds.size());
        for (const size_t& medId : record.medicineIds)
            sysorder += " " + to_string(medId);
        sysorder += " " + to_string(record.symptomIds.size());
        for (const size_t& sympId : record.symptomIds)
            sysorder += " " + to_string(sympId);

        cout << "Calling: " << sysorder << endl;

        int retValue = system(sysorder.c_str());

        cout << endl;
        cout << endl;
    }
    // system ("rm enc_temp.bin");

    return 0;
}
