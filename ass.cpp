#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <mpi.h>
#include "Counter.h"

#define ROOT 0

using namespace std;

std::fstream& GotoLine(std::fstream& file, unsigned int num){
    file.seekg(std::ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return file;
}

int get_lines(std::string filename) {
	std::fstream inFile;
	std::string line;
	int count = 0;
	
	inFile.open(filename);
	if (!inFile.is_open()) {
		std::cout << "Error: could not open file" << std::endl;
		exit(1);
	}
	
	while ( std::getline(inFile, line) ) {
		if (line.back() == '\r')
			line.erase(line.end() - 1); // remove last 'CR' or \r character
		
		//std::cout << line << " [" << line.size() << " bytes]" << std::endl;
		// std::cout << line << std::endl;
		count++;
	}
	
	std::cout << "# of lines read: " << count << std::endl;
	
	std::cout << std::endl;
	inFile.close();
	return count;
}

void process_lines(std::string filename, Counter& counter, int start, int eachLine) {
	fstream inFile;
	string line;
	stringstream ss;
	string word;
	int lineNumber = 0;
	int wordCount = 0;
	int end = start+eachLine;

	
	inFile.open(filename);
	if (!inFile.is_open()) {
		std::cout << "Error: could not open file" << std::endl;
		exit(1);
	}

	while ( lineNumber != start && std::getline(inFile, line) ) {		
		lineNumber++;
	}

	while (lineNumber >= start && lineNumber <= start+eachLine && getline(inFile,line)){
		if (end == lineNumber){
			break;
		}
		if (line.back() == '\r')
			line.erase(line.end() - 1); // remove last 'CR' or \r character
		
		// clear flags and set the stringstream buffer to the current line
		ss.clear();
		ss.str(line); 
		
		// process each word in this line
		wordCount = 0;
		while (ss >> word) {
			add_counter_item(counter, word);
			wordCount++;
		}
		lineNumber++;
	}
	inFile.close();
}


int main(int argc, char* argv[]) {
	// MPI environment variables
	int nprocs, rank, lineCount, eachLine; // number of processes and rank/id of each process
	
	// Initialize the MPI environment
	MPI_Init(&argc, &argv);


	// Get the number of processes and rank of each process
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	char filename[] = "pg100.txt";

	Counter word_counter;

	if (rank == ROOT) {
		lineCount = get_lines(filename);
		eachLine = lineCount/nprocs;
	}
	MPI_Bcast(&lineCount,1,MPI_INT,ROOT,MPI_COMM_WORLD);
	MPI_Bcast(&eachLine,1,MPI_INT,ROOT,MPI_COMM_WORLD);

	process_lines(filename,word_counter,eachLine*rank, eachLine);
	if (rank == ROOT){
		process_lines(filename,word_counter,(eachLine*nprocs), lineCount%nprocs);
	}
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
		print_counter_content(total_counter,"All");
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