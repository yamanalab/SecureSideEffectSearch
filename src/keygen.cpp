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

  ifstream fin ("../settings/contextsetting.txt");
  long m,p,r,L,c,w,d,security;
  fin>>p>>r>>L>>c>>w>>d>>security;
  fin.close();

  //long m = 32768 , p = 257 , r = 1;
  //long L = 9;
  //long c = 3;
  //long w = 64;
  //long d = 0;
  //long security = 128;
  m=FindM(security,L,c,p,d,0,0);

  // p=2, r=8, L=7, m = 8191, #slot =630, sec=144.677
  // p=257, r=1, L=9, m=10363
  /*
  23311,23310,518,459.854
  32377,32376,568,699.911
  43691,43690,1285,951.033
  */

  FHEcontext context(m, p, r);
  //initialize FHEcontext
  buildModChain(context, L, c);
  //modify the context, adding primes to the modulus buildModChain

  long num_slot = context.zMStar.getNSlots();

  cout<<"Security: "<<context.securityLevel()<<endl;
  cout<<"m: "<<m<<endl;
  cout<<"Nslots: "<<num_slot<<endl;

  FHESecKey secretKey(context);
  //construct secret Key
  const FHEPubKey& publicKey = secretKey;
  //construct public key

  ZZX G = context.alMod.getFactorsOverZZ()[0];

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
