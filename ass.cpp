/*
???

Compile with:
	$ mpic++ ass.cpp -o ass
	
Run with:
	$ mpirun ./ass
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <mpi.h>
#include "Counter.h"

#define ROOT 0

using namespace std;

int get_line_count(string filename) {
	/* Return the number of lines in a given text file. */
	
	fstream inFile; // input file object
	string line; // temporary variable to store each line as a string of chars
	int lineCount = 0;
	
	inFile.open(filename);
	if (!inFile.is_open()) {
		cout << "Error: could not open file '" << filename << "'" << endl;
		exit(1);
	}
	
	// count the number of lines in the input file
	while ( getline(inFile, line) ) {
		if (line.back() == '\r')
			line.erase(line.end() - 1); // remove last 'CR' or \r character if exists
		lineCount++;
	}
	
	// cout << "'" << filename << "' line count: " << lineCount << endl << endl;
	
	inFile.close();
	return lineCount;
}

void process_lines(string filename, int eachLines, int startPos, Counter& counter) {
	/* Count the words in a specified section of a given text file, using the given starting line 
	position and number of lines to determine which section of the text file to process. Store the 
	results in the provided Counter object.
	*/
	
	// int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	fstream inFile; // input file object
	string line; // temporary variable to store each line as a string of chars
	string word; // temporary variable to store each word
	stringstream lineStream; // temporary stream buffer for each line
	int endPos = startPos + eachLines; // ending line position
	int processedLineCount = 0;

	// cout << " rank:" << rank 
		// << " startPos:" << startPos
		// << " endPos:" << endPos 
		// << endl;
		
	inFile.open(filename);
	if (!inFile.is_open()) {
		cout << "Error: could not open file '" << filename << "'" << endl;
		exit(1);
	}
	
	// Loop over each line in the text file. Start to process the text lines from the given 
	// starting line position up to but not including the ending line position.
	for (int pos = 0; pos < endPos; pos++) {
		getline(inFile, line); // read the line at this position
		if (pos < startPos)
			continue; // advance to next iteration/line if starting line position is not yet reached
		
		if (line.back() == '\r')
			line.erase(line.end() - 1); // remove last 'CR' or \r character if exists
		
		// clear buffer flags and load the current line into stream buffer
		lineStream.clear();
		lineStream.str(line); 
		
		// process each 'word' in this line, adding/updating the word counter
		while (lineStream >> word) {
			// word text cleaning goes here!
			
			update_counter(counter, word);
		}
		processedLineCount++;
	}
	
	// cout << " rank:" << rank 
		// << " processedLineCount:" << processedLineCount
		// << endl;
	
	inFile.close();
}


int main(int argc, char* argv[]) {
	// MPI environment variables
	int nprocs, rank; // number of processes and rank/id of each process
	
	// Initialize the MPI environment
	MPI_Init(&argc, &argv);

	// Get the number of processes and rank of each process
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	// Process variables
	string filename = "pg100.txt";
	int totalLines; // total number of lines in the file
	int eachLines; // number of lines for each process to work on
	int remLines; // number of remaining lies for root proccess to work on
	Counter word_counter; // word counter of this process

	// Root proc determines the total number of lines and each process's line count
	if (rank == ROOT) {
		totalLines = get_line_count(filename);
		eachLines = totalLines / nprocs; // divided equally between the processes
		remLines = totalLines % nprocs;
		
		// cout << "totalLines:" << totalLines 
			// << " eachLines:" << eachLines 
			// << " remLines:" << remLines << endl;
		
		// handle work for remaining lines
		process_lines(filename, remLines, (eachLines*nprocs), word_counter);
	}
	
	// Broadcast each process's line count
	MPI_Bcast(&eachLines, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	// Each process works on their section of the text file
	process_lines(filename, eachLines, (eachLines*rank), word_counter);
	
	MPI_Barrier(MPI_COMM_WORLD);

	/*----------------
		gather values
	------------------*/

	auto values = getValues(word_counter);
	auto keys = getKeys(word_counter);
	int dataSize = values.size();

	int recvIntCount[nprocs];

	MPI_Gather(&dataSize, 1, MPI_INT,recvIntCount, 1, MPI_INT,ROOT, MPI_COMM_WORLD);

	int totlen = 0;
    int displsValues[nprocs];
    int *totalInt = NULL;

    if (rank == ROOT) {

        displsValues[0] = 0;
        totlen += recvIntCount[0];

        for (int i=1; i<nprocs; i++) {
           totlen += recvIntCount[i];
           displsValues[i] = displsValues[i-1] + recvIntCount[i-1];
        }
        totalInt = (int*) malloc(totlen * sizeof(int));
    }

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Gatherv(&values[0], dataSize, MPI_INT, totalInt, recvIntCount, displsValues, MPI_INT,ROOT, MPI_COMM_WORLD);
	
	/*----------------
		gather keys
	------------------*/
	int maxKeyLength = 150;

	int numKeys = keys.size();
    int totalLength = numKeys * maxKeyLength;
    int finalNumKeys = 0;
    MPI_Reduce(&numKeys, &finalNumKeys, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Computing number of elements that are received from each process
    int *recvcounts = NULL;
    if (rank == 0)
        recvcounts = new int[nprocs];

    MPI_Gather(&totalLength, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Computing displacement relative to recvbuf at which to place the incoming data from each process
    int *displs = NULL;
    if (rank == 0)
    {
        displs = new int[nprocs];

        displs[0] = 0;
        for (int i = 1; i < nprocs; i++)
            displs[i] = displs[i - 1] + recvcounts[i - 1];
    }

    char(*dictKeys)[maxKeyLength];
    char(*finalDictKeys)[maxKeyLength];
    dictKeys = (char(*)[maxKeyLength])malloc(numKeys * sizeof(*dictKeys));
    if (rank == 0)
        finalDictKeys = (char(*)[maxKeyLength])malloc(finalNumKeys * sizeof(*finalDictKeys));

    // Collect keys for each process
    int i = 0;
    for (auto str : keys)
    {
        strncpy(dictKeys[i], str.c_str(), maxKeyLength);
        i++;
    }

    MPI_Gatherv(dictKeys, totalLength, MPI_CHAR, finalDictKeys, recvcounts, displs, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Create new counter and merge all previous counter

	vector<string> keysTotal;
	vector<int> valuesTotal;

	if (!rank){
		for (int i = 0; i<totlen;i++){
			keysTotal.push_back((string)finalDictKeys[i]);
			valuesTotal.push_back(totalInt[i]);
			// cout << finalDictKeys[i]  << " : " << totalInt[i] << endl;
		}
	}


	Counter total_counter;

	if (!rank){
		compose_counter(total_counter,keysTotal,valuesTotal);
		cout << "Contents of the total_counter:-" << endl;
		print_counter(total_counter);
	}

    delete[] dictKeys;
    if (rank == 0)
    {
        delete[] finalDictKeys;
        delete[] recvcounts;
        delete[] displs;
    }

	// Finalize the MPI environment.
    MPI_Finalize();
	
	return 0;
}