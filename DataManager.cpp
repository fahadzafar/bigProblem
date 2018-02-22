// Author : Fahad Zafar
// Copyright : 2014
// this class makes sure that the files are read, unzipped and then 
// maintains the HashMaker object.

#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include "HashMaker.cpp"
using namespace std;

// This class reads the files and makes sure that all the data per line
// is entered into the map.
class DataManager {
	
	// TODO(fahad): Move to a .h file later.
	int FILE_IS_COMPRESSED;
	string COMPRESS_FILE_EXTENSION;
	
	string filename;
	
	public:
	HashMaker *uniqueLines;
	
	DataManager(string iFilename, int iRank){
		filename = iFilename;
		uniqueLines = new HashMaker(iRank);
		
		COMPRESS_FILE_EXTENSION = ".gz";
		FILE_IS_COMPRESSED = 0;
	}
	
	void unzip (){
		//gzip foo.txt.gz -d 
		string unzipCommand = string("gzip ") + filename + string (" -d ");
		system(unzipCommand.c_str());	
		
		// Now remove the ".zip" from the filename and reset it
		std::size_t found = filename.find(COMPRESS_FILE_EXTENSION);
		
		if (found==std::string::npos) {
			cout <<" The file extension:" << COMPRESS_FILE_EXTENSION <<
			" is not present in " << filename << endl;
		// TODO(fahad): Exit the program or something.
		}
		
		filename = filename.substr(0, filename.length() - COMPRESS_FILE_EXTENSION.length());
		cout <<"New uncompressed filename:" << filename << endl;
	}
	
	void placeTransferableData(long* ptr, int size){
		std::map<long,int>::iterator it_type;
		int i = 0;
		for(it_type = uniqueLines->mymap.begin(); it_type != uniqueLines->mymap.end(); it_type++, i++) {
			  ptr[i] = it_type->first;
		}
	}
	
	// Create an entry into the map for each line that is read.
	void processFiles(){
		
		// If file is compressed, you need to unzip it first.
		if (FILE_IS_COMPRESSED) {
			unzip();
		}
		
		fstream dataFile(filename.c_str(), ios::in);
		string record;
		
		if (dataFile.is_open()){
			getline(dataFile, record);
			while (dataFile) {
				#ifdef DEBUG
					cout << record << endl;
				#endif
				uniqueLines->insertValue(record);
				getline(dataFile, record);
			}
			dataFile.close();
		} else {
			cout << "ERROR: Cannot open file.\n";
		}	
	}
	
	void merge(long *secondObjectData, int size){
		uniqueLines->merge(secondObjectData, size);
	}
	
	// Remove any memory allocated at runtime.
	~DataManager(){
		delete uniqueLines;
	}
		
	
	
	
	
};
