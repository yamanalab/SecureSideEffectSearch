#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>
#include<string>
#include<cassert>
using namespace std;

// This file update invertedindex
// Usage: update [FILENAME FOR ENCRYPTED MASK] [num(MedID)] [List<MedID>] [num(SideID)] [List<SideID>]

int main (int argc, char *argv[]){

  assert(argc>5);
  cout<<"argc: "<<argc<<endl;
  // Must contains at least 5 parameters

  ifstream fdbbasics("../settings/dbbasics.bin", std::ios::binary);
  bool dbstatus;
  assert(fdbbasics>>dbstatus);
  assert(dbstatus);
  int numRec, totMed, totSide;
  assert(fdbbasics>>numRec);
  assert(fdbbasics>>totMed);
  assert(fdbbasics>>totSide);
  fdbbasics.close();

  string filename(argv[1]);
  try{
    ifstream ftest(filename.c_str(), std::ios::binary);
    ftest.close();
  }
  catch(const exception& e){
    cerr<<"Seems problems within the filename or the file itself."<<endl;
    cerr<<"DB not updated."<<endl;
    return 1;
  };

  const string numMed_str(argv[2]);
  const int numMed=stoi(numMed_str);
  cout<<"numMed: "<<numMed<<endl;

  vector<int> meds;
  assert(argc>=5+numMed);
  for (int i=0;i<numMed;++i){
    const string medID_str(argv[3+i]);
    const int medID=stoi(medID_str);
    assert(medID<totMed);
    if (find(meds.begin(),meds.end(),medID)==meds.end()) meds.push_back(medID);
  }

  const string numSide_str(argv[3+numMed]);
  const int numSide=stoi(numSide_str);
  cout<<"numSide: "<<numSide<<endl;

  vector<int> sides;
  assert(argc==4+numMed+numSide);
  for(int i=0;i<numSide;++i){
    const string sideID_str(argv[4+numMed+i]);
    const int sideID=stoi(sideID_str);
    assert(sideID<totSide);
    if (find(sides.begin(),sides.end(),sideID)==sides.end()) sides.push_back(sideID);
  }

  string copy_str="mv "+filename+" ../encdata/"+to_string(numRec)+".bin";
  cout<<"Calling: "<<copy_str<<endl;
  system(copy_str.c_str());

  string auxfilename="../auxdata/"+to_string(numRec)+".bin";

  ofstream faux(auxfilename.c_str(), std::ios::binary);
  faux<<"Medicine: ["<<meds[0];
  for (int i=1;i<numMed;++i) faux<<", "<<meds[i];
  faux<<"]"<<endl;
  faux<<"Side Effect: ["<<sides[0];
  for (int i=1;i<numSide;++i) faux<<", "<<sides[i];
  faux<<"]"<<endl;
  faux.close();

  ofstream fdbbasicsw("../settings/dbbasics.bin", std::ios::binary);
  fdbbasicsw<<dbstatus<<endl;
  fdbbasicsw<<numRec+1<<endl;
  fdbbasicsw<<totMed<<endl;
  fdbbasicsw<<totSide<<endl;
  fdbbasicsw.close();

  vector<vector<int>> medIndex;
  vector<vector<int>> sideIndex;
  ifstream findexmed ("../auxdata/med.inv", std::ios::binary);
  int numMedDict;
  findexmed>>numMedDict;
  for (int i=0;i<numMedDict;++i){
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

  ifstream findexside ("../auxdata/side.inv", std::ios::binary);
  int numSideDict;
  findexside>>numSideDict;
  for (int i=0;i<numSideDict;++i){
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

  for (int i=0;i<numMed;++i) medIndex[meds[i]].push_back(numRec);
  for (int i=0;i<numSide;++i) sideIndex[sides[i]].push_back(numRec);
  cout<<"Updated rev index"<<endl;

  ofstream findexmedw ("../auxdata/med.inv", std::ios::binary);
  findexmedw<<numMedDict<<endl;
  for (int i=0;i<numMedDict;++i){
    findexmedw<<medIndex[i].size()<<endl;
    for (int j=0;j<medIndex[i].size();++j) findexmedw<<medIndex[i][j]<<endl;
  }
  findexmedw.close();

  ofstream findexsidew ("../auxdata/side.inv", std::ios::binary);
  findexsidew<<numSideDict<<endl;
  for (int i=0;i<numSideDict;++i){
    findexsidew<<sideIndex[i].size()<<endl;
    for (int j=0;j<sideIndex[i].size();++j) findexsidew<<sideIndex[i][j]<<endl;
  }
  findexsidew.close();

  cout<<"New record as: #"<<numRec<<endl;
  cout<<"DB successfully updated."<<endl;

  return 0;

}
