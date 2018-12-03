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
//
// Don't comment on my coding style
// It looks ugly, I know.

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

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  // obtain a seed from the system clock
  std::mt19937 generator(seed);

  ifsteram fdbbasics("../settings/dbbasics.bin", std::ios::binary);
  bool dbstatus;
  assert(fdbbasics>>dbstatus);
  assert(dbstatus);
  int numRec, totMed, totSide;
  assert(fdbbasics>>numRec);
  assert(fdbbasics>>totMed);
  assert(fdbbasics>>totSide);
  fdbbasics.close();

  ifstream fctxt("../settings/context.bin", std::ios::binary);
  unsigned long m, p, r;
  std::vector<long> gens, ords;
  readContextBase(fctxt, m, p, r, gens, ords);
  FHEcontext context(m, p, r, gens, ords);
  assert(fctxt>>context);
  fctxt.close();
  //read context from context.bin

  ZZX G=context.alMod.getFactorsOverZZ()[0];
  EncryptedArray ea(context, G);
  //generate ea from context

  FHEPubKey publicKey(context);
  ifstream fpkey("../settings/pk.bin", std::ios::binary);
  assert(fpkey>>publicKey);
  fpkey.close();
  //read publicKey from pk.bin

  ifstream findexmed ("../auxdata/med.inv", std::ios::binary);
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
  //read invertedindex for medicine from med.inv

  ifstream findexside ("../auxdata/side.inv", std::ios::binary);
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
  //read invertedindex for side effects from side.inv

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
      Ctxt query_mask(publicKey);
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
         send the result back to user and get decrypted request
      3. Calculate time and memory usage
      */

      vector<int> filteredres=merge(MedID, SideID);

      int numRes=filteredres.size(), numchunks;
      vector<vector<int>> chunks;
      vector<Ctxt> chunk_res;
      vector<long> allzero500_long(500, 0);
      Ctxt allzero500(publicKey);
      ea.encrypt(allzero500, publicKey, allzero500_long);
      for (int i=0;i<numRes;i+=500, ++numchunks){
        int end=min(i+500, numRes);
        vector<int> chunk(filteredres.begin()+i, fiteredres.begin()+end);
        chunks.push_back(chunk);
        chunk_res.push_back(allzero500);
      }

      NTL_EXEC_RANGE(numchunks, first, last)

        for (long i=first;i<last;++i){

          for (int j=0;j<chunks[i].size();++j){
            vector<long> posindicator_long=allzero500_long;
            posindicator_long[j]=1;
            ZZX posindicator;
            ea.encode(posindicator, posindicator_long);
            string filename="../encdata/"+to_string(chunks[i][j]);
            ifstream fdb(filename.c_str(), std::ios::binary);
            Ctxt encmask(publicKey);
            assert(fdb>>encmask);
            fdb.close();
            encmask.multByConstant(posindicator);
            chunk_res[i].addCtxt(encmask, false);
            // false means not minus
          }

          vector<Ctxt> rangemul;
          for (int diff=-5;diff<=5;++diff){
            Ctxt diffed(publicKey)=chunk_res[i];
            diffed.addConstant(to_ZZX(diff));
            rangemul.push_back(diffed);
          }

          while (rangemul.size()>1){
            vector<Ctxt> rangemul_derived;
            for (int j=0;j<rangemul.size();j+=2){
              if (j+1<rangemul.size()) rangemul[j].multiplyBy(rangemul[j+1]);
              rangemul_derived.push_back(rangemul[j]);
            }
            rangemul=rangemul_derived;
          }
          //11 -> 6 -> 3 -> 2 -> 1 , We need at least level of >6

          chunk_res[i]=rangemul[0];
          chunk_res[i].multByConstant(to_ZZX(generator()%256+1));

        }

      NTL_EXEC_RANGE_END

      client<<numchunks;
      client.flush();
      for (int i=0;i<numchunks;++i){
        client<<chunk_res[i];
        client.flush();
      }

      int numchoice;
      vector<pair<int,int>> choice_list;

      client>>numchoice;
      for (int i=0;i<numchoice;++i){
        int i,j;
        client>>i>>j;
        choice_list.push_back(make_pair(i,j));
      }
      //complete dealing

      for (int i=0;i<numchoice;++i){
        client<<chunks[choice_list[i].first][choice_list[i].second]<<endl;
      }
      //complete output

      client.close();
    }
    else cerr<<"Error: "<<err<<endl;
  }

  return 0;

}
