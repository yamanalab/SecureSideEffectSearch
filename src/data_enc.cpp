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

NTL_CLIENT

// Read from dummy data
// make encrypted and call update

const int Nmeds=2000;
const int Nsides=100;
const int Nrecs=40000;

int main (void){
  ofstream fdbbasics("../settings/dbbasics.bin", std::ios::binary);
  fdbbasics<<1<<endl;
  fdbbasics<<0<<endl;
  fdbbasics<<Nmeds<<endl;
  fdbbasics<<Nsides<<endl;
  fdbbasics.close();

  ofstream fmedinv("../auxdata/med.inv", std::ios::binary);
  fmedinv<<Nmeds<<endl;
  for (int i=0;i<Nmeds;++i) fmedinv<<0<<endl;
  fmedinv.close();

  ofstream fsideinv("../auxdata/side.inv", std::ios::binary);
  fsideinv<<Nsides<<endl;
  for (int i=0;i<Nsides;++i) fsideinv<<0<<endl;
  fsideinv.close();

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

  ifstream fdmydata ("../dmydata/data.csv");
  for (int i=0;i<Nrecs;++i){
    int recid, mask;
    fdmydata>>recid>>mask;
    int medCount, sideCount;
    vector<int> meds;
    vector<int> sides;
    fdmydata>>medCount;
    for (int i=0;i<medCount;++i){
      int medID;
      fdmydata>>medID;
      meds.push_back(medID);
    }
    fdmydata>>sideCount;
    for (int i=0;i<sideCount;++i){
      int sideID;
      fdmydata>>sideID;
      sides.push_back(sideID);
    }

    Ctxt encmask(publicKey);
    publicKey.Encrypt(encmask, to_ZZX(mask));
    ofstream fenc("enc_temp.bin", std::ios::binary);
    fenc<<encmask;
    fenc.close();

    string sysorder="./update enc_temp.bin "+to_string(medCount);
    for (int i=0;i<medCount;++i) sysorder+=" "+to_string(meds[i]);
    sysorder+=" "+to_string(sideCount);
    for (int i=0;i<sideCount;++i) sysorder+=" "+to_string(sides[i]);

    cout<<"Calling: "<<sysorder<<endl;

    system(sysorder.c_str());

    cout<<endl;
    cout<<endl;
  }
  fdmydata.close();
  //system ("rm enc_temp.bin");

  return 0;
}
