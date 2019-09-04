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

// Forked from yuishi's repository

int main()
{
	long p = 2;
  long r = 8;
	long min_slot = 500; // minimum # slots
  long L = 9;         // # levels for a certain task
	long first_m = 10000;// technical artifact of the analysis
	long last_m = 50000; // m ranges in [first_m, last_m]

	ofstream ofs("good_params_for_L" + std::to_string(L));
	ofs << "m,n,#slot,security" << endl;

	cout << "Finding good params ..." << endl;

	int threads=72;

	NTL_EXEC_RANGE(threads, first, last)
		for (int i=first;i<last;++i){
			int st_m=first_m+(last_m-first_m)/threads*i;
			int ed_m=first_m+(last_m-first_m)/threads*(i+1);
			for (long m = st_m; m < ed_m; ++m)
			{
				if (ProbPrime(m))
				{ // if m is a prime number
		  		FHEcontext context(m, p, r);
					long num_slot = context.zMStar.getNSlots();
					if (context.zMStar.numOfGens() == 1 // noise-(almost)free rotation
					 && context.zMStar.SameOrd(0)       // noise-(almost)free rotation
					 && num_slot >= min_slot)           // requirement for #slots
					{
		  			buildModChain(context, L);
						double sec = context.securityLevel();
						if (sec >= 128.0) {
							ofs << m << ","
									<< m-1 << ","
									<< num_slot << ","
									<< sec
									<< endl;
							cout << " m = " << m << ", #slot =" << num_slot << ", sec=" << sec << endl;
						} else {
							cerr << " the security level is low " << sec << " m=" << m << endl;
						}
					}
				}
			}
		}
	NTL_EXEC_RANGE_END
	ofs.close();

	return 0;
}
