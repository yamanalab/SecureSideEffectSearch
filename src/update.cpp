#include<iostream>
using namespace std;

// This file update invertedindex
// Usage: update [FILENAME FOR ENCRYPTED MASK] [num(MedID)] [List<MedID>] [num(SideID)] [List<SideID>]

int main (int argc, char *argv[]){

  assert(argc>5);
  // Must contains at least 5 parameters

  ifsteram fdbbasics("../settings/dbbasics.bin", std::ios::binary);
  bool dbstatus;
  assert(fdbbasics>>dbstatus);
  assert(dbstatus);
  int numRec, totMed, totSide;
  assert(fdbbasics>>numRec);
  assert(fdbbasics>>totMed);
  assert(fdbbasics>>totSide);
  fdbbasics.close();

  string filename(argv[1]);
  file.insert(0, "../data/");
  try{
    ifstream ftest(filename, std::ios::binary);
  }
  catch(const std::exception& e){
    cout<<" Seems problems within the filename or the file itself. "<<endl;
    cout<<" DB not updated. "<<endl;
    return 1;
  };

  return 0;

}
