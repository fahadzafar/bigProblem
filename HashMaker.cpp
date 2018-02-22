// Author : Fahad Zafar
// Copyright : 2014
// This class adds all the data records given to it line by line into 
// a map that will erase all duplicates. The rank is used as the value
// so that we can know where the record came from if in the future we
// need to write out the unique data. In this case we only need to 
// find the number of unique elements in the map.

#include <map>
#include <string>
#include <locale> 

using namespace std;

// This class creates a map for all the entries read from a file
class HashMaker {
	
	
public :
 std::map<long,int> mymap;
 int rank;

HashMaker(int iRank){
	rank = iRank;
}

// For a particular string value, create a hash and insert it into the 
// class level map.
void insertValue(string record){
  std::locale loc;                 // the "C" locale
  const std::collate<char>& coll = std::use_facet<std::collate<char> >(loc);
  long myHash = coll.hash(record.data(),record.data()+record.length());
  mymap.insert(std::pair<long,int>(myHash,rank) );
  
  #ifdef DEBUG 
    cout <<"inserting:" <<myHash <<", size = ";
    printSize();
  #endif
}

void merge(long *secondMapData, int size){
	for (int i =0 ;i < size ; i++)
	  mymap.insert(std::pair<long,int>(secondMapData[i],rank)) ;
}

void printSize() {
	  std::cout << "Map size for " <<  rank << " is " <<mymap.size() << '\n';
	}
};
