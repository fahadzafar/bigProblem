// Author : Fahad Zafar
// Copyright : 2014
// This is the main file that runs the utility. The concept is to create
// a map for all the lines you read from the file. Then the cores 
// exchange the data until all the data is collected at the root which
// is the answer. The collection is optimized so that the minimum number
// of transfers are required. ceil(ln(1000)) merges would be required
// for 1000 cores running in parallel.

// mpich2 or openmpi must be installed.
// Make command:  mpic++ -L/usr/local/lib findUniqueNumber.cpp -o findUniqueNumber
// Command used to run this code :  mpirun -n 1000 --hostfile ~/nodes_to_use findUniqueNumber
// More information in the README.

// #define DEBUG 1

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <math.h>

#include "DataManager.cpp"

// Id of the core
int myRank_;

// Total size of cores.
int size;

// Total files you will read. Currently its fixed to 1 per core
int TOTAL_FILES = 1000;

// Name of the file to read.
string FILENAME = "foo.txt";

// Prototypes for a helper function.
void printMerges(int noOfMerges);

// ------------------------- MAIN --------------------------------
int main(
        int argc,
        char *argv[]) { 

    // Initialize all the MPI routines and get node specific information.
    MPI_Init(&argc, &argv); // starts MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank_); // get current process id
    MPI_Comm_size(MPI_COMM_WORLD, &size); // get number of processes

    // Start Timer.
    double starttime, endtime;
    if (myRank_== 0) {
        starttime = MPI_Wtime();
    }
    
    // Read the data and enter it into the map
    int filesPerCore = 1; 
		DataManager dm(FILENAME, myRank_);
		dm.processFiles();
		
		// size = 10;
		// Now that the data has been hashed, merge the maps for each of the
		// cores to get the unique set of values in the last core. Merging
		// should be done in pairs, hence for instance for 10 cores there 
		// should be 3 merges since ln(10) = 2.3
		int noOfMerges = (int)ceil(log2(size));
		
		// check to see if even or odd
		int i = 0;
		
		
		int number = 500;
		int sizeOfElements = 0;
		int incomingSizeOfElements = 0;
		long *buffer = 0;
		
		if (myRank_ == 0) {
			printMerges(noOfMerges);
		}
		
		for (int j = 0 ; j < noOfMerges ; j++) {
			int distance = int(pow(2,j));
			for (i = 0; i < size; i = i + distance + distance) {
				
				sizeOfElements = dm.uniqueLines->mymap.size();
				if ((i + distance) < size) {
				  // Send or receive data here.
				  // i merges with  i + distance. i will receive the data and 
				  // i + distance will send the data in a game of life kind of
				  // fashion. At the end of the compute, myRank = 0 will have
				  // the final answer.
					// ---------------- First send the size of buffer you will 
					// transfer/receive
						if (myRank_ == i) {
						// receive the size of elements first.
						MPI_Recv(&incomingSizeOfElements, 1, MPI_INT, i + distance, 0, MPI_COMM_WORLD,	MPI_STATUS_IGNORE);
						//cout << i << " will be receiving:" <<   incomingSizeOfElements   << " no. elements."<<endl;	
					} else {
						// send the size of elements because myRank_ == i + distance.
						// cout << i + distance <<" will be sending sending:"<< sizeOfElements  << " no. of elements." << endl;
						
						// Send the number of elements first.
						MPI_Send(&sizeOfElements, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
					}
					
					// Make sure the number of elements to be tranferred are known
					// to all parties, i-e, the sender and receiver.
					MPI_Barrier(MPI_COMM_WORLD);

					
					//------------ Transfer the actual data.
					if (myRank_ == i) {
						// receive	
						// receive the size of elements first.
						// cout << i << " waiting on data." <<endl;
						buffer = new long[incomingSizeOfElements];
						MPI_Recv(buffer, incomingSizeOfElements, MPI_LONG, i + distance, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						
						// Once the data is received, just insert into the map and 
						// that will remove all the duplicates.
						dm.uniqueLines->merge(buffer, incomingSizeOfElements);
					
					} else {
						// send because myRank_ == i + distance.		
						buffer = new long[sizeOfElements];
						
						// Now place all the hashes into a buffer for each record 
						// read by this core. Sending hash is awesome since its only
						// a long and the probability of hash collision is
						// 1/numeric_limits<unsigned long>::max().
						// Reference : http://www.cplusplus.com/reference/locale/collate/hash/
						// Can also use a better hash function if you want.
						dm.placeTransferableData(buffer, sizeOfElements);
						
						// Send the number of elements first.
						MPI_Send(buffer, sizeOfElements, MPI_LONG, i, 0, MPI_COMM_WORLD);
						// cout << i + distance <<" sent the data. "<<endl;
					}
					//--------------------------
				}	else {
					// Dont do anything. stand alone for next stage of merge.
				}
			}		
			
		
		// Make sure everyone has traded and completed the jth merge before
		// continuing.
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Empty the dynamically allocated memory.
    delete buffer;  
		}
		
		// Final unique records count.
		if (myRank_ == 0) {
			cout << " >>>>>>>>> Final unique records:" << dm.uniqueLines->mymap.size() << endl;
		}
    // End the timer.
    MPI_Barrier(MPI_COMM_WORLD);
    if (myRank_== 0) {
        endtime = MPI_Wtime();
        cout << "\nTime :" << endtime - starttime << endl;
    }

    // Finsih MPI and return.
    MPI_Finalize();
    return 0;
}

// Helper function to print the number of merges required
void printMerges(int noOfMerges){
	int i = 0;
	cout << "-------Merge information------------------------" << endl;
	cout << "No. of merges = " << noOfMerges << endl;
		  
	for (int j = 0 ; j < noOfMerges ; j++) {
		int distance = int(pow(2,j));
		cout << "******Merge Stage : " << j + 1<< endl;	
		for (i = 0; i < size; i = i + distance + distance) {
			if ((i + distance) < size) {
					cout << i << " merges with " << i + distance <<"." <<endl;
				}	else {
				// Dont do anything.
				cout << i << " stands alone. " << endl;
			}
		} 
	}
	cout << "-------------------------------" << endl;
}

