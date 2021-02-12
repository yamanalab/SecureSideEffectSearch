#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "filepath_info.h"
using namespace std;

// This file update invertedindex
// Usage: update [RECORD ID] [FILENAME FOR ENCRYPTED MASK] [num(MedID)]
// [List<MedID>] [num(SideID)] [List<SideID>]

int main(int argc, char* argv[])
{

    assert(argc > 6);
    cout << "argc: " << argc << endl;
    // Must contains at least 6 parameters

    size_t recordId = stoi(argv[1]);
    string filename(argv[2]);
    cout << "recordId: " << recordId << endl;

    try
    {
        ifstream ftest(filename.c_str(), std::ios::binary);
        ftest.close();
    }
    catch (const exception& e)
    {
        cerr << "Seems problems within the filename or the file itself."
             << endl;
        cerr << "DB not updated." << endl;
        return 1;
    };

    const string numMed_str(argv[3]);
    const int numMed = stoi(numMed_str);
    cout << "numMed: " << numMed << endl;

    vector<int> meds;
    assert(argc >= 6 + numMed);
    for (int i = 0; i < numMed; ++i)
    {
        const string medID_str(argv[4 + i]);
        const int medID = stoi(medID_str);
        if (find(meds.begin(), meds.end(), medID) == meds.end())
            meds.push_back(medID);
    }

    const string numSide_str(argv[4 + numMed]);
    const int numSide = stoi(numSide_str);
    cout << "numSide: " << numSide << endl;

    vector<int> sides;
    assert(argc == 5 + numMed + numSide);
    for (int i = 0; i < numSide; ++i)
    {
        const string sideID_str(argv[5 + numMed + i]);
        const int sideID = stoi(sideID_str);
        if (find(sides.begin(), sides.end(), sideID) == sides.end())
            sides.push_back(sideID);
    }

    string copy_str =
      "mv " + filename + " " + ENCDATA_DIR_PATH + to_string(recordId) + ".bin";
    cout << "Calling: " << copy_str << endl;
    int retValue = system(copy_str.c_str());

    string auxfilename = AUXDATA_DIR_PATH + to_string(recordId) + ".bin";

    ofstream faux(auxfilename.c_str(), std::ios::binary);
    faux << "Medicine: [" << meds[0];
    for (int i = 1; i < numMed; ++i)
        faux << ", " << meds[i];
    faux << "]" << endl;
    faux << "Side Effect: [" << sides[0];
    for (int i = 1; i < numSide; ++i)
        faux << ", " << sides[i];
    faux << "]" << endl;
    faux.close();

    // ofstream fdbbasicsw(DBBASICS_FILE_PATH, std::ios::binary);
    // fdbbasicsw << dbstatus << endl;
    // fdbbasicsw << numRec + 1 << endl;
    // fdbbasicsw << totMed << endl;
    // fdbbasicsw << totSide << endl;
    // fdbbasicsw.close();

    map<int, vector<int>> medIndex;
    map<int, vector<int>> sideIndex;

    ifstream findexmed(MED_INV_FILE_PATH, std::ios::binary);
    string line;
    vector<string> medInfo;
    int numMedDict;
    findexmed >> numMedDict;
    for (int i = 0; i < numMedDict; ++i)
    {
        findexmed >> line;
        boost::algorithm::split(medInfo, line, boost::is_any_of(":"));
        int medId = stoi(medInfo[0]);
        int numindex = stoi(medInfo[1]);
        vector<int> tempindex;
        while (numindex--)
        {
            int temprec;
            findexmed >> temprec;
            tempindex.push_back(temprec);
        }
        medIndex.insert(make_pair(medId, tempindex));
    }
    findexmed.close();

    ifstream findexside(SIDE_INV_FILE_PATH, std::ios::binary);
    vector<string> sideInfo;
    int numSideDict;
    findexside >> numSideDict;
    for (int i = 0; i < numSideDict; ++i)
    {
        findexside >> line;
        boost::algorithm::split(sideInfo, line, boost::is_any_of(":"));
        int sideId = stoi(sideInfo[0]);
        int numindex = stoi(sideInfo[1]);
        vector<int> tempindex;
        while (numindex--)
        {
            int temprec;
            findexside >> temprec;
            tempindex.push_back(temprec);
        }
        sideIndex.insert(make_pair(sideId, tempindex));
    }
    findexside.close();

    for (int i = 0; i < numMed; ++i)
        medIndex[meds[i]].push_back(recordId);
    for (int i = 0; i < numSide; ++i)
        sideIndex[sides[i]].push_back(recordId);
    cout << "Updated rev index" << endl;

    ofstream findexmedw(MED_INV_FILE_PATH, std::ios::binary);
    findexmedw << numMedDict << endl;
    for (const auto& medIdxPair : medIndex)
    {
        int medId = medIdxPair.first;
        vector<int> recordIds = medIdxPair.second;
        findexmedw << medId << ":" << recordIds.size() << endl;
        for (const int& recordId : recordIds)
        {
            findexmedw << recordId << endl;
        }
    }
    // for (int i = 0; i < numMedDict; ++i)
    // {
    //     findexmedw << medIndex[i].size() << endl;
    //     for (int j = 0; j < medIndex[i].size(); ++j)
    //         findexmedw << medIndex[i][j] << endl;
    // }
    findexmedw.close();

    ofstream findexsidew(SIDE_INV_FILE_PATH, std::ios::binary);
    findexsidew << numSideDict << endl;
    for (const auto& sideIdxPair : sideIndex)
    {
        int sideId = sideIdxPair.first;
        vector<int> recordIds = sideIdxPair.second;
        findexsidew << sideId << ":" << recordIds.size() << endl;
        for (const int& recordId : recordIds)
        {
            findexsidew << recordId << endl;
        }
    }
    // for (int i = 0; i < numSideDict; ++i)
    // {
    //     findexsidew << sideIndex[i].size() << endl;
    //     for (int j = 0; j < sideIndex[i].size(); ++j)
    //         findexsidew << sideIndex[i][j] << endl;
    // }
    findexsidew.close();

    // cout << "New record as: #" << numRec << endl;
    cout << "New record id: " << recordId << endl;
    cout << "DB successfully updated." << endl;

    return 0;
}
