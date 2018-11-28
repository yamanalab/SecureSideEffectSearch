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
#include <chrono>
#include <random>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
using namespace std;
using boost::asio::ip::tcp;

// Server only have sk
// Receive query from client
//
// query: Enc(age&gender)
//        List<MedID>
//        List<SideID>
//
// Return search result to client

int main (void){

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

  while (true){
    
  }

  return 0;

}
