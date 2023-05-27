#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mpi.h>
#include "Counter.h"

#define ROOT 0

void print_lines(std::string filename) {
	std::ifstream inFile;
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
		std::cout << line << std::endl;
		count++;
	}
	
	std::cout << "# of lines read: " << count << std::endl;
	
	std::cout << std::endl;
	inFile.close();
}

void process_lines(std::string filename, Counter& counter) {
	std::ifstream inFile;
	std::string line;
	std::stringstream ss;
	std::string word;
	int lineCount = 0;
	int wordCount = 0;
	
	inFile.open(filename);
	if (!inFile.is_open()) {
		std::cout << "Error: could not open file" << std::endl;
		exit(1);
	}
	
	// process by line
	while ( std::getline(inFile, line) ) {
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
		
		lineCount++;
		//std::cout << "Line #" << lineCount << ": " << wordCount << " words processed" << std::endl;
	}
	
	std::cout << std::endl;
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
	
	char filename[] = "pg100.txt";
	Counter word_counter;
	
	if (rank == ROOT) {
		//print_lines(filename);
		process_lines(filename, word_counter);
		print_counter_content(word_counter, "word counter");
		std::cout << "Unique words: " << word_counter.size() << std::endl;
	}
	
	// Finalize the MPI environment.
    MPI_Finalize();
	
	return 0;
}