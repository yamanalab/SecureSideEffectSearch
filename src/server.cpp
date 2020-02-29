#include <NTL/BasicThreadPool.h>
#include <NTL/ZZ.h>
#include <NTL/lzz_pXFactoring.h>
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
#include "timing.h"
using namespace std;

NTL_CLIENT

//#define __DEBUG__
#define __MULTITHREADING_IN_USE__

const int NThreads = 28;

// Server only have sk
// Receive query from client
//
// query: Enc(age&gender)
//        List<MedID>
//        List<SideID>
//
// Return search result to client
// Usage: server [PORT]
//
// Don't comment on my coding style
// It looks ugly, I know.

string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    stringstream ss;
    ss << put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

vector<vector<int>> medIndex;
vector<vector<int>> sideIndex;

vector<int> mergeOR(const vector<vector<int>> &index, const vector<int> &id)
{

    int len = id.size();

    if (len == 0)
    {
        vector<int> ret;
        return ret;
    }

    if (len == 1)
    {
        return index[id[0]];
    }

    const vector<int> id_l = vector<int>(id.begin(), id.begin() + len / 2);
    const vector<int> id_r = vector<int>(id.begin() + len / 2, id.end());

    const vector<int> res_l = mergeOR(index, id_l);
    const vector<int> res_r = mergeOR(index, id_r);

    int len_l = res_l.size(), p_l = 0;
    int len_r = res_r.size(), p_r = 0;

    vector<int> ret;
    while (p_l != len_l && p_r != len_r)
    {
        if (res_l[p_l] < res_r[p_r])
            ret.push_back(res_l[p_l++]);
        else
        {
            if (res_l[p_l] == res_r[p_r])
                ++p_l;
            ret.push_back(res_r[p_r++]);
        }
    }
    while (p_l != len_l)
        ret.push_back(res_l[p_l++]);
    while (p_r != len_r)
        ret.push_back(res_r[p_r++]);

    return ret;
}

vector<int> merge(const vector<int> &medID, const vector<int> &sideID)
{

    // First use OR to merge between medIndex[medID] -> res1

    const vector<int> medres = mergeOR(medIndex, medID);

    // Then use OR to merge between sideIndex[sideID] -> res2

    const vector<int> sideres = mergeOR(sideIndex, sideID);

    // Then use AND to merge res1 and res2 -> ret

    vector<int> ret;

    int lenmed = medres.size(), pmed = 0;
    int lenside = sideres.size(), pside = 0;

    while (pmed != lenmed && pside != lenside)
    {
        if (medres[pmed] < sideres[pside])
            ++pmed;
        else if (medres[pmed] > sideres[pside])
            ++pside;
        else
        {
            ret.push_back(medres[pmed++]);
            ++pside;
        }
    }

    return ret;
}

int main(int argc, char *argv[])
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    // obtain a seed from the system clock
    std::mt19937 generator(seed);

    ifstream fdbbasics("../settings/dbbasics.bin", std::ios::binary);
    bool dbstatus;
    assert(fdbbasics >> dbstatus);
    assert(dbstatus);
    int numRec, totMed, totSide;
    assert(fdbbasics >> numRec);
    assert(fdbbasics >> totMed);
    assert(fdbbasics >> totSide);
    fdbbasics.close();

    ifstream fctxt("../settings/context.bin", std::ios::binary);
    unsigned long m, p, r;
    std::vector<long> gens, ords;
    readContextBase(fctxt, m, p, r, gens, ords);
    FHEcontext context(m, p, r, gens, ords);
    assert(fctxt >> context);
    fctxt.close();
    // read context from context.bin

    ZZX G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    long nslots = ea.size();
    // generate ea from context

    FHEPubKey publicKey(context);
    ifstream fpkey("../settings/pk.bin", std::ios::binary);
    assert(fpkey >> publicKey);
    fpkey.close();
    // read publicKey from pk.bin

    ifstream findexmed("../auxdata/med.inv", std::ios::binary);
    int numMed;
    findexmed >> numMed;
    for (int i = 0; i < numMed; ++i)
    {
        int numindex;
        findexmed >> numindex;
        vector<int> tempindex;
        while (numindex--)
        {
            int temprec;
            findexmed >> temprec;
            tempindex.push_back(temprec);
        }
        medIndex.push_back(tempindex);
    }
    findexmed.close();
    // read invertedindex for medicine from med.inv

    ifstream findexside("../auxdata/side.inv", std::ios::binary);
    int numSide;
    findexside >> numSide;
    for (int i = 0; i < numSide; ++i)
    {
        int numindex;
        findexside >> numindex;
        vector<int> tempindex;
        while (numindex--)
        {
            int temprec;
            findexside >> temprec;
            tempindex.push_back(temprec);
        }
        sideIndex.push_back(tempindex);
    }
    findexside.close();
    // read invertedindex for side effects from side.inv

    assert(argc == 2 || argc == 3);
    const vector<long> allzero_long(nslots, 0);
    Ctxt allzero(publicKey);
    ea.encrypt(allzero, publicKey, allzero_long);

    string port_str(argv[1]);
    int port = stoi(port_str);
    if (argc == 3)
    {
        int numthreads = stoi(argv[2]);
        assert(numthreads >= 1 && numthreads <= NThreads);
        SetNumThreads(numthreads);
        if (numthreads == 1)
        {
#undef __MULTITHREADING_IN_USE__
            cout << "Successfuly turned off multithreading" << endl;
        }
    }
    else
    {
        SetNumThreads(NThreads);
    }

    using boost::asio::ip::tcp;
    boost::asio::io_service ios;
    tcp::endpoint endpoint(tcp::v4(), port);
    tcp::acceptor acceptor(ios, endpoint);
    while (true)
    {
        tcp::iostream client;
        boost::system::error_code err;
        acceptor.accept(*client.rdbuf(), err);
        if (!err)
        {
            cout << return_current_time_and_date() << " Connected." << endl;
            Ctxt query_mask(publicKey);
            client >> query_mask;
            int nMed, nSide;
            vector<int> MedID;
            vector<int> SideID;
            client >> nMed;
            for (int i = 0; i < nMed; ++i)
            {
                int medid;
                client >> medid;
                MedID.push_back(medid);
            }
            client >> nSide;
            for (int i = 0; i < nSide; ++i)
            {
                int sideid;
                client >> sideid;
                SideID.push_back(sideid);
            }
            cout << return_current_time_and_date() << " Completed input."
                 << endl;

            // complete input

            auto start_timer = std::chrono::system_clock::now();

            /*
            Dealing part consist of three parts:
            1. First use merge to do filtering by MedID & SideID
            2. Then split to chunks (500 for each chunk), and do multithreading
            calculation send the result back to user and get decrypted request
            3. Calculate time and memory usage
            */

            vector<int> filteredres = merge(MedID, SideID);

            int numRes = filteredres.size(), numchunks = 0;
            double percentage = 100.0 * (double)numRes / (double)numRec;
            cout << return_current_time_and_date()
                 << " Completed filtering, with efficiency of " << percentage
                 << "%." << endl;
            vector<vector<int>> chunks;
            vector<Ctxt> chunk_res;
            for (int i = 0; i < numRes; i += 100, ++numchunks)
            {
                int end = min(i + 100, numRes);
                vector<int> chunk(filteredres.begin() + i,
                                  filteredres.begin() + end);
                chunks.push_back(chunk);
                chunk_res.push_back(allzero);
            }
            // cout<<nslots<<endl;

            cout << return_current_time_and_date()
                 << " Completed chunk splitting." << endl;
            /*
            " numRes: "<<numRes<<" numChunks: "<<numchunks<<endl;

            for (int i=0;i<numchunks;++i){
              cout<<"Chunk #"<<i<<endl;
              for (int j=0;j<chunks[i].size();++j) cout<<chunks[i][j]<<" ";
              cout<<endl;
            }
            */

#ifndef __MULTITHREADING_IN_USE__
            long first = 0, last = numchunks;
#else
            cout << return_current_time_and_date()
                 << " Start with multithreading in use." << endl;
            NTL_EXEC_RANGE(numchunks, first, last)
#endif

            for (long i = first; i < last; ++i)
            {

                // cout<<" Lvl --> Before addition:
                // "<<chunk_res[i].findNaturalHeight()<<endl;

                for (int j = 0; j < chunks[i].size(); ++j)
                {
                    vector<long> posindicator_long = allzero_long;
                    posindicator_long[j] = 1;
                    ZZX posindicator;
                    ea.encode(posindicator, posindicator_long);
                    string filename =
                      "../encdata/" + to_string(chunks[i][j]) + ".bin";
                    ifstream fdb(filename.c_str(), std::ios::binary);
                    Ctxt encmask(publicKey);
                    assert(fdb >> encmask);
                    fdb.close();
                    // cout<<"Done extraction of encrypted mask for file
                    // #"<<j<<endl;
                    encmask.multByConstant(posindicator);
                    chunk_res[i].addCtxt(encmask, false);
                    // false means not minus
                }

// cout<<" Lvl --> After addition: "<<chunk_res[i].findNaturalHeight()<<endl;
#ifdef __DEBUG__
                FHESecKey secretKey(context);
                ifstream fskey("../settings/sk.bin", std::ios::binary);
                assert(fskey >> secretKey);
                fskey.close();
                // read secretKey from sk.bin
                cout << "Chunk #" << i << endl;
                cout << "Before summing up" << endl;
                {
                    vector<long> decrypted;
                    ea.decrypt(chunk_res[i], secretKey, decrypted);
                    for (int j = 0; j < 100; ++j)
                        cout << i * 100 + j << ":" << decrypted[j] << " ";
                    cout << endl;
                }
#endif

                chunk_res[i].addCtxt(query_mask, true);

#ifdef __DEBUG__
                cout << "After summing up" << endl;
                {
                    vector<long> decrypted;
                    ea.decrypt(chunk_res[i], secretKey, decrypted);
                    for (int j = 0; j < 100; ++j)
                        cout << i * 100 + j << ":" << decrypted[j] << " ";
                    cout << endl;
                }
#endif

                vector<Ctxt> rangemul;
                for (int diff = -5; diff <= 5; ++diff)
                {
                    Ctxt diffed(publicKey);
                    diffed = chunk_res[i];
                    diffed.addConstant(to_ZZX(diff));
                    rangemul.push_back(diffed);
                }

#ifdef __DEBUG__
                cout << "After rangemul" << endl;
                for (int k = 0; k < 11; ++k)
                {
                    vector<long> decrypted;
                    ea.decrypt(rangemul[k], secretKey, decrypted);
                    cout << "rangemul #" << k << endl;
                    for (int j = 0; j < 100; ++j)
                        cout << i * 100 + j << ":" << decrypted[j] << " ";
                    cout << endl;
                }
#endif

                // cout<<" Lvl --> After ranging:
                // "<<rangemul[0].findNaturalHeight()<<endl;

                while (rangemul.size() > 1)
                {
                    // cout<<" Lvl --> current rangemul size:
                    // "<<rangemul.size()<<" level:
                    // "<<rangemul[0].findNaturalHeight()<<endl;
                    vector<Ctxt> rangemul_derived;
                    for (int j = 0; j < rangemul.size(); j += 2)
                    {
                        if (j + 1 < rangemul.size())
                            rangemul[j].multiplyBy(rangemul[j + 1]);
                        rangemul_derived.push_back(rangemul[j]);
                    }
                    rangemul = rangemul_derived;
                }
                // 11 -> 6 -> 3 -> 2 -> 1 , We need at least level of >6

                chunk_res[i] = rangemul[0];

#ifdef __DEBUG__
                cout << "After times up" << endl;
                {
                    vector<long> decrypted;
                    ea.decrypt(chunk_res[i], secretKey, decrypted);
                    for (int j = 0; j < 100; ++j)
                        cout << i * 100 + j << ":" << decrypted[j] << " ";
                    cout << endl;
                    cout << "Level: " << chunk_res[i].findNaturalHeight()
                         << endl;
                }
#endif

                vector<long> randlist_long;
                for (int i = 0; i < nslots; ++i)
                    randlist_long.push_back(generator() % 256 + 1);
                ZZX randlist;
                ea.encode(randlist, randlist_long);
                chunk_res[i].multByConstant(randlist);

#ifdef __DEBUG__
                {
                    cout << "After randomize" << endl;
                    vector<long> decrypted;
                    ea.decrypt(chunk_res[i], secretKey, decrypted);
                    for (int j = 0; j < 100; ++j)
                        cout << i * 100 + j << ":" << decrypted[j] << " ";
                    cout << endl;
                    cout << "Level: " << chunk_res[i].findNaturalHeight()
                         << endl;
                }
#endif

                // cout<<" Lvl --> After randomize:
                // "<<chunk_res[i].findNaturalHeight()<<endl;
            }

#ifdef __MULTITHREADING_IN_USE__
            NTL_EXEC_RANGE_END
#endif

            cout << return_current_time_and_date() << " Complete calculation."
                 << endl;

            auto start_comm_timer = std::chrono::system_clock::now();

            client << numchunks << endl;
            for (int i = 0; i < numchunks; ++i)
            {
                client << chunk_res[i];
                client.flush();
            }

            cout << return_current_time_and_date()
                 << " Complete output to client." << endl;

            int numchoice;
            vector<pair<int, int>> choice_list;

            client >> numchoice;
            for (int k = 0; k < numchoice; ++k)
            {
                int i, j;
                client >> i >> j;
                if (j < chunks[i].size())
                    choice_list.push_back(make_pair(i, j));
            }
            auto end_timer = std::chrono::system_clock::now();
            std::chrono::duration<double> time_servercalc =
              start_comm_timer - start_timer;
            std::chrono::duration<double> time_servercomm =
              end_timer - start_comm_timer;

            cout << return_current_time_and_date() << " Complete dealing."
                 << endl;
            cout << return_current_time_and_date()
                 << " Time spent for server to do calculation is "
                 << time_servercalc.count() << " seconds." << endl;
            cout << return_current_time_and_date()
                 << " Time spent on communication from server's POV is "
                 << time_servercomm.count() << " seconds." << endl;
            // complete dealing

            numchoice = choice_list.size();
            client << numchoice << endl;
            for (int i = 0; i < numchoice; ++i)
                client << chunks[choice_list[i].first][choice_list[i].second]
                       << endl;
            client << time_servercalc.count() << endl;
            client << time_servercomm.count() << endl;
            client << percentage << endl;
            // complete output

            client.close();
            chunks.clear();
            chunk_res.clear();
            // break;
        }
        else
            cerr << return_current_time_and_date() << " Error: " << err << endl;
    }

    return 0;
}
