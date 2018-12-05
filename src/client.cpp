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
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
using namespace std;

// Client holds secret key and public key
//
// Usage: client [IP/DOMAIN] [PORT]

int main (int argc, char *argv[]){

  ifstream fdbbasics("../settings/dbbasics.bin", std::ios::binary);
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

  FHESecKey secretKey(context);
  ifstream fskey("../settings/sk.bin", std::ios::binary);
  assert(fskey>>secretKey);
  fskey.close();
  //read secretKey from sk.bin

  FHEPubKey publicKey(context);
  ifstream fpkey("../settings/pk.bin", std::ios::binary);
  assert(fpkey>>publicKey);
  fpkey.close();
  //read publicKey from pk.bin

  assert(argc==3);
  string ip_address(argv[1]);
  string port(argv[2]);

  using boost::asio::ip::tcp;
  tcp::iostream connect(ip_address.c_str(), port.c_str());
  if (!connect){
    cerr<<"Can not connect: "<<connect.error().message()<<endl;
    return 1;
  }

  cout<<"Successfully created connection with "<<ip_address<<":"<<port<<endl;
  cout<<"Enter age:(0-122)";
  int age;
  cin>>age;
  assert(age>=0&&age<=122);
  cout<<"Enter gender:(m/f)";
  char gender;
  cin>>gender;
  assert(gender=='m'||gender=='f');
  int mask=age+(gender=='m'?1:0)*128+5;
  Ctxt enc_mask(publicKey);
  publicKey.Encrypt(enc_mask, to_ZZX(mask));
  cout<<"Age and gender information successfully encrypted."<<endl;
  cout<<"Enter number of medicine (>0): ";
  int numMed;
  cin>>numMed;
  assert(numMed>0);
  cout<<"Enter medicine ID (total "<<numMed<<", 0-"<<totMed-1<<"): ";
  vector<int> meds;
  for (int i=0;i<numMed;++i){
    int medID;
    cin>>medID;
    assert(medID<totMed);
    if (find(meds.begin(),meds.end(),medID)==meds.end()) meds.push_back(medID);
  }
  cout<<"Enter number of side effects (>0): ";
  int numSide;
  cin>>numSide;
  assert(numSide>0);
  cout<<"Enter side effect ID (total "<<numSide<<", 0-"<<totSide-1<<"): ";
  vector<int> sides;
  for (int i=0;i<numSide;++i){
    int sideID;
    cin>>sideID;
    assert(sideID<totSide);
    if (find(sides.begin(),sides.end(),sideID)==sides.end()) sides.push_back(sideID);
  }

  connect<<enc_mask;
  connect.flush();

  connect<<meds.size()<<endl;
  for (int i=0;i<meds.size();++i) connect<<meds[i]<<endl;

  connect<<sides.size()<<endl;
  for (int i=0;i<sides.size();++i) connect<<sides[i]<<endl;

  connect.flush();

  int numChunks;
  connect>>numChunks;
  vector<pair<int,int>> ret;
  for (int i=0;i<numChunks;++i){
    Ctxt chunk_res(publicKey);
    connect>>chunk_res;
    vector<long> decrypted;
    ea.decrypt(chunk_res, secretKey, decrypted);
    for (int j=0;j<decrypted.size()&&j<500;++j)
      if (decrypted[j]==0) ret.push_back(make_pair(i,j));
  }

  connect<<ret.size()<<endl;
  for (pair<int,int> ret_item: ret){
    connect<<ret_item.first<<endl;
    connect<<ret_item.second<<endl;
  }

  connect.flush();

  int numRes;
  connect>>numRes;
  vector<int> records;
  for (int i=0;i<numRes;++i){
    int recID;
    connect>>recID;
    records.push_back(recID);
  }
  unsigned exec_time;
  connect>>exec_time;
  cout<<"Server has completed running in "<<exec_time<<" seconds."<<endl;
  cout<<"Result list:"<<endl;
  for (int i=0;i<numRes;++i) cout<<records[i]<<endl;
  cout<<"Total record number hit: "<<numRes<<endl;

  return 0;
}
