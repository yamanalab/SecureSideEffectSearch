#include <NTL/BasicThreadPool.h>
#include <NTL/ZZ.h>
#include <NTL/lzz_pXFactoring.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

#include "EncryptedArray.h"
#include "FHE.h"
#include "constants.h"
#include "picojson_wrapper.h"
#include "timing.h"
using namespace std;

NTL_CLIENT

// Client holds secret key and public key
//
// Usage: client [IP/DOMAIN] [PORT] [AGE (0-122)] [GENDER (m|f|o)]
//               [MED IDs] [SIDE IDs]

string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    stringstream ss;
    ss << put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

class Record
{
    int id_;
    vector<int> medicinIds_;
    vector<int> symptomIds_;

public:
    Record()
    {
    }
    Record(const int& id, const vector<int>& medicine_ids,
           const vector<int>& symptom_ids)
      : id_(id), medicinIds_(medicine_ids), symptomIds_(symptom_ids)
    {
    }

    const int& id() const
    {
        return id_;
    }
    const vector<int>& medicinIds() const
    {
        return medicinIds_;
    }
    const vector<int>& symptomIds() const
    {
        return symptomIds_;
    }
};

int main(int argc, char* argv[])
{

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

    FHESecKey secretKey(context);
    ifstream fskey(FHE_SK_FILE_PATH, std::ios::binary);
    assert(fskey >> secretKey);
    fskey.close();
    // read secretKey from sk.bin

    FHEPubKey publicKey(context);
    ifstream fpkey(FHE_PK_FILE_PATH, std::ios::binary);
    assert(fpkey >> publicKey);
    fpkey.close();
    // read publicKey from pk.bin

    assert(argc == 7);
    string ip_address(argv[1]);
    string port(argv[2]);

    using boost::asio::ip::tcp;
    tcp::iostream connect(ip_address.c_str(), port.c_str());
    if (!connect)
    {
        cerr << return_current_time_and_date()
             << " Can not connect: " << connect.error().message() << endl;
        return 1;
    }

    cerr << return_current_time_and_date()
         << " Successfully created connection with " << ip_address << ":"
         << port << endl;
    int age = stoi(argv[3]);
    assert(age >= 0 && age <= 122);
    char gender = *argv[4];
    assert(gender == 'm' || gender == 'f' || gender == 'o');
    int mask = age + static_cast<int>(GENDER_CHAR_MAP.at(gender)) * 128 + 5;
    Ctxt enc_mask(publicKey);
    publicKey.Encrypt(enc_mask, to_ZZX(mask));
    vector<string> tmp;
    vector<int> meds;
    boost::algorithm::split(tmp, argv[5], boost::is_any_of(","));
    for (string s : tmp)
    {
        int medID = stoi(s);
        if (find(meds.begin(), meds.end(), medID) == meds.end())
            meds.push_back(medID);
    }
    assert(meds.size() > 0);
    tmp.clear();
    vector<int> sides;
    boost::algorithm::split(tmp, argv[6], boost::is_any_of(","));
    for (string s : tmp)
    {
        int sideID = stoi(s);
        if (find(sides.begin(), sides.end(), sideID) == sides.end())
            sides.push_back(sideID);
    }
    assert(sides.size() > 0);
    auto start_timer = std::chrono::system_clock::now();

    connect << enc_mask;
    connect.flush();

    connect << meds.size() << endl;
    for (int i = 0; i < meds.size(); ++i)
        connect << meds[i] << endl;

    connect << sides.size() << endl;
    for (int i = 0; i < sides.size(); ++i)
        connect << sides[i] << endl;

    connect.flush();

    int numChunks;
    connect >> numChunks;
    vector<pair<int, int>> ret;
    vector<Ctxt> chunks_res;
    for (int i = 0; i < numChunks; ++i)
    {
        Ctxt chunk_res(publicKey);
        connect >> chunk_res;
        chunks_res.push_back(chunk_res);
    }
    auto dec_timer = std::chrono::system_clock::now();
    for (int i = 0; i < numChunks; ++i)
    {
        vector<long> decrypted;
        ea.decrypt(chunks_res[i], secretKey, decrypted);
        /*
        cerr<<"Chunk #"<<i<<endl;
        for (int j=0;j<100;++j) cerr<<i*100+j<<":"<<decrypted[j]<<" ";
        cerr<<endl;
        */
        for (int j = 0; j < decrypted.size(); ++j)
            if (decrypted[j] == 0)
                ret.push_back(make_pair(i, j));
    }
    auto dec_end_timer = std::chrono::system_clock::now();
    std::chrono::duration<double> diff_dec = dec_end_timer - dec_timer;

    connect << ret.size() << endl;
    for (pair<int, int> ret_item : ret)
    {
        connect << ret_item.first << endl;
        connect << ret_item.second << endl;
    }

    connect.flush();

    int numRes, numMed, numSide;
    connect >> numRes;
    vector<Record> records(numRes);
    int id;
    vector<int> searched_medicine_ids, searched_symptom_ids;
    Record record;
    for (int i = 0; i < numRes; ++i)
    {
        connect >> id;
        connect >> numMed;
        searched_medicine_ids.resize(numMed);
        for (int j = 0; j < numMed; ++j)
        {
            connect >> searched_medicine_ids[j];
        }
        connect >> numSide;
        searched_symptom_ids.resize(numSide);
        for (int k = 0; k < numSide; ++k)
        {
            connect >> searched_symptom_ids[k];
        }
        record = Record(move(id), move(searched_medicine_ids),
                        move(searched_symptom_ids));
        records[i] = move(record);
    }
    double exec_time, comm_time, percentage;
    connect >> exec_time >> comm_time >> percentage;
    auto end_timer = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end_timer - start_timer;
    cerr << "Server has completed running in " << exec_time << " seconds."
         << endl;
    cerr << "Communication spent " << comm_time
         << " seconds from server's POV and decryption spent "
         << diff_dec.count() << " seconds from client's POV." << endl;
    cerr << "Total record number hit: " << numRes << endl;
    cerr << "The whole session has used " << diff.count() << " seconds."
         << endl;
    cerr << return_current_time_and_date() << " Session with server is over."
         << endl;

    Json json;
    json.add<Record>("records", records,
                     [](Record r)
                     {
                         Json j;
                         j.add("id", r.id());
                         j.add("medicine_ids", r.medicinIds());
                         j.add("symptom_ids", r.symptomIds());
                         return j;
                     });

    cout << json.serialize() << endl;

    return 0;
}
