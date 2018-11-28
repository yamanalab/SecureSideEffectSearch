#include <iostream>
#include <fstream>
#include <NTL/ZZ.h>
#include <NTL/BasicThreadPool.h>
#include "FHE.h"
#include "timing.h"
#include "EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>
#include <cassert>
#include <cstdio>
#include <thread>
#include <chrono>
#include <random>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
using namespace std;

// Server only have sk
// Receive query from client
//
// query: Enc(age&gender)
//        List<MedID>
//        List<SideID>
//
// Return search result to client
// Usage: server [PORT]

vector<vector<int>> medIndex;
vector<vector<int>> sideIndex;

vector<int> mergeOR(const vector<vector<int>> &index, const vector<int> &id){

  int len = id.size();

  if (len == 0){
    vector<int> ret;
    return ret;
  }

  if (len == 1){
    return index[id[0]];
  }

  id_l = vector<int>(id.begin(), id.begin()+len/2);
  id_r = vector<int>(id.begin()+len/2, id.end());

  const vector<int> res_l = mergeOR(index, id_l);
  const vector<int> res_r = mergeOR(index, id_r);

  int len_l = res_l.size(), p_l = 0;
  int len_r = res_r.size(), p_r = 0;

  vector<int> ret;
  while (p_l != len_l && p_r != len_r){
    if (res_l[p_l] < res_r[p_r]) ret.push_back(res_l[p_l++]);
    else{
      ret.push_back(res_r[p_r++]);
      if (res_l[p_l] == res_r[p_r]) ++p_l;
    }
  }
  while (p_l != len_l) ret.push_back(res_l[p_l++]);
  while (p_r != len_r) ret.push_back(res_r[p_r++]);

  return ret;

}

vector<int> merge(const vector<int> &medID, const vector<int> &sideID){

  // First use OR to merge between medIndex[medID] -> res1

  const vector<int> medres = mergeOR(medIndex, medID);

  // Then use OR to merge between sideIndex[sideID] -> res2

  const vector<int> sideres = mergeOR(sideIndex, sideID);

  // Then use AND to merge res1 and res2 -> ret

  vector<int> ret;

  int lenmed = medres.size(), pmed = 0;
  int lenside = sideres.size(), pside = 0;

  while (pmed != lenmed && pside != lenside){
    if (medres[pmed] < sideres[pside]) ++pmed;
    else if (medres[pmed] > sideres[pside]) ++pside;
    else{
      ret.push_back(medres[pmed++]);
      ++pside;
    }
  }

  return ret;

}

int main (int argc, char *argv[]){

  ifstream fctxt("context.bin", std::ios::binary);
  unsigned long m, p, r;
  std::vector<long> gens, ords;
  readContextBase(fctxt, m, p, r, gens, ords);
  FHEcontext context(m, p, r, gens, ords);
  assert(fctxt>>context);
  fctxt.close();
  //read context from context.bin

  FHEPubKey publicKey(context);
  ifstream fpkey("pk.bin", std::ios::binary);
  assert(fpkey>>publicKey);
  fpkey.close();
  //read publicKey from pk.bin

  ifstream findexmed ("med.inv", std::ios::binary);
  int numMed;
  findexmed>>numMed;
  for (int i=0;i<numMed;++i){
    int numindex;
    findexmed>>numindex;
    vector<int> tempindex;
    while (numindex--){
      int temprec;
      findexmed>>temprec;
      tempindex.push_back(temprec);
    }
    medIndex.push_back(tempindex);
  }
  findexmed.close();

  ifstream findexside ("side.inv", std::ios::binary);
  int numSide;
  findexside>>numSide;
  for (int i=0;i<numSide;++i){
    int numindex;
    findexside>>numindex;
    vector<int> tempindex;
    while (numindex--){
      int temprec;
      findexside>>temprec;
      tempindex.push_back(temprec);
    }
    sideIndex.push_back(tempindex);
  }
  findexside.close();

  assert(argc==2);
  string port_str(argv[1]);
  int port=stoi(port_str);

  using boost::asio::ip::tcp;
  boost::asio::io_service ios;
  tcp::endpoint endpoint(tcp::v4(), port);
  tcp::acceptor acceptor(ios, endpoint);
  while (true){
    tcp::iostream client;
    boost::system::error_code err;
    acceptor.accept(*client.rdbuf(), err);
    if (!err){
      cout<<"connected"<<endl;
      Ctxt query_mask;
      client>>query_mask;
      int nMed,nSide;
      vector<int> MedID;
      vector<int> SideID;
      client>>nMed;
      for (int i=0;i<nMed;++i){
        int medid;
        client>>medid;
        MedID.push_back(medid);
      }
      client>>nSide;
      for (int i=0;i<nSide;++i){
        int sideid;
        client>>sideid;
        SideID.push_back(sideid);
      }
      //complete input

      /*
      Dealing part consist of three parts:
      1. First use merge to do filtering by MedID & SideID
      2. Then split to chunks (500 for each chunk), and do multithreading calculation
      3. Calculate time and memory usage
      */

      /*Part 1 start*/

      vector<int> filteredres=merge(MedID, SideID);

      /*Part 1 end*/


      //complete dealing

      //complete output
      client.close();
    }
    else cerr<<"Error: "<<err<<endl;
  }

  return 0;

}
