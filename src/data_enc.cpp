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

// Read from dummy data
// make encrypted and call update

const int Nmeds=2000;
const int Nsides=100;
const int Nrecs=40000;

int main (void){
  ofstream fdbbasics("../settings/dbbasics.bin", std::ios::binary);
  fdbbasics<<1<<endl;
  fdbbasics<<Nrecs<<endl;
  fdbbasics<<Nmeds<<endl;
  fdbbasics<<Nsides<<endl;
  fdbbasics.close();

  ifstream fdmydata ("../dmydata/data.csv");
  
  return 0;
}
