#include <iostream>
#include <NTL/ZZ.h>
#include <NTL/BasicThreadPool.h>
#include "FHE.h"
#include "timing.h"
#include "EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>
#include <cassert>
#include <cstdio>
#include <fstream>
using namespace std;

// generate keyset and context

int main (void){

  long m = 2048 , p = 257 , r = 1;
  long L = 4;
  long c = 3;
  long w = 64;
  long d = 0;
  long security = 128;
  ZZX G;
  //m=FindM(security,L,c,p,d,0,0);

  FHEcontext context(m, p, r);
  //initialize FHEcontext
  buildModChain(context, L, c);
  //modify the context, adding primes to the modulus buildModChain

  assert(context.securityLevel()>=128);

  FHESecKey secretKey(context);
  //construct secret Key
  const FHEPubKey& publicKey = secretKey;
  //construct public key

  G = context.alMod.getFactorsOverZZ()[0];

  secretKey.GenSecKey(w);
  //actually generate a secret key with Hamming weight w
  addSome1DMatrices(secretKey);

  cout<<"Key Generated Successfully"<<endl;

  ofstream fpkey("../settings/pk.bin", std::ios::binary);
  fpkey<<publicKey<<endl;
  fpkey.close();
  //store publicKey in pk.bin

  ofstream fskey("../settings/sk.bin", std::ios::binary);
  fskey<<secretKey<<endl;
  fskey.close();
  //store secretKey in sk.bin

  ofstream fctxt("../settings/context.bin", std::ios::binary);
  writeContextBase(fctxt, context);
  fctxt<<context;
  fctxt.close();
  //store context in context.bin


  cout<<"Public Key stored in pk.bin, distribute it freely!"<<endl;
  cout<<"Secret Key stored in sk.bin, take it care!"<<endl;
  cout<<"Context stored in context.bin!"<<endl;
  cout<<endl;

  return 0;
}
