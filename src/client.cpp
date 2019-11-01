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
#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
using namespace std;

NTL_CLIENT

// Client holds secret key and public key
//
// Usage: client [IP/DOMAIN] [PORT]

string return_current_time_and_date(){
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    stringstream ss;
    ss << put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

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

  assert(argc==7);
  string ip_address(argv[1]);
  string port(argv[2]);

  using boost::asio::ip::tcp;
  tcp::iostream connect(ip_address.c_str(), port.c_str());
  if (!connect){
    cerr<<return_current_time_and_date()<<" Can not connect: "<<connect.error().message()<<endl;
    return 1;
  }

  cout<<return_current_time_and_date()<<" Successfully created connection with "<<ip_address<<":"<<port<<endl;
  int age = stoi(argv[3]);
  assert(age>=0&&age<=122);
  char gender = *argv[4];
  assert(gender=='m'||gender=='f');
  int mask=age+(gender=='m'?1:0)*128+5;
  Ctxt enc_mask(publicKey);
  publicKey.Encrypt(enc_mask, to_ZZX(mask));
  vector<string> tmp;
  vector<int> meds;
  boost::algorithm::split(tmp, argv[5], boost::is_any_of(","));
  for (string s : tmp) {
    int medID = stoi(s);
    assert(medID<totMed);
    if (find(meds.begin(),meds.end(),medID)==meds.end()) meds.push_back(medID);
  }
  assert(meds.size()>0);
  tmp.clear();
  vector<int> sides;
  boost::algorithm::split(tmp, argv[6], boost::is_any_of(","));
  for (string s : tmp) {
    int sideID = stoi(s);
    assert(sideID<totSide);
    if (find(sides.begin(),sides.end(),sideID)==sides.end()) sides.push_back(sideID);
  }
  assert(sides.size()>0);
  auto start_timer = std::chrono::system_clock::now();

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
  vector<Ctxt> chunks_res;
  for (int i=0;i<numChunks;++i){
    Ctxt chunk_res(publicKey);
    connect>>chunk_res;
    chunks_res.push_back(chunk_res);
  }
  auto dec_timer = std::chrono::system_clock::now();
  for (int i=0;i<numChunks;++i){
    vector<long> decrypted;
    ea.decrypt(chunks_res[i], secretKey, decrypted);
    /*
    cout<<"Chunk #"<<i<<endl;
    for (int j=0;j<100;++j) cout<<i*100+j<<":"<<decrypted[j]<<" ";
    cout<<endl;
    */
    for (int j=0;j<decrypted.size();++j)
      if (decrypted[j]==0) ret.push_back(make_pair(i,j));
  }
  auto dec_end_timer = std::chrono::system_clock::now();
  std::chrono::duration<double> diff_dec = dec_end_timer - dec_timer;

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
  double exec_time, comm_time, percentage;
  connect>>exec_time>>comm_time>>percentage;
  auto end_timer = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end_timer-start_timer;
  cout<<"Server has completed running in "<<exec_time<<" seconds."<<endl;
  cout<<"Communication spent "<<comm_time<<" seconds from server's POV and decryption spent "<<diff_dec.count()<<" seconds from client's POV."<<endl;
  cout<<"Total record number hit: "<<numRes<<endl;
  cout<<"The whole session has used "<<diff.count()<<" seconds."<<endl;
  cout<<return_current_time_and_date()<<" Session with server is over."<<endl;

  cout<<"Show result list?(y/n)";
  char c;
  cin>>c;
  if (c=='Y'||c=='y'){
    cout<<"Result list:"<<endl;
    for (int i=0;i<numRes;++i) cout<<records[i]<<endl;
  }

  return 0;
}
