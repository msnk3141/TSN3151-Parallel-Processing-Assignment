/*
???

Compile with:
	$ mpic++ ass.cpp -o ass

Run with:
	$ mpirun ./ass
or with specified number of processors (e.g. 4):
	$ mpirun -n 4 ./ass
or run and redirect output to a text file:
	$ mpirun ./ass > results.txt
*/

#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <sstream>
#include <mpi.h>
#include "Counter.h"

#define ROOT 0
#define FILENAME_SIZE 256

using namespace std;

bool is_file_exists(string filename) {
    fstream file(filename);
    return file.good();
}

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

	inFile.close();
	return lineCount;
}

void process_lines(string filename, int eachLines, int startPos, Counter& counter, int minWordLen, int maxWordLen) {
	/* Count the words in a specified section of a given text file, using the given starting line
	position and number of lines to determine which section of the text file to process. Store the
	results in the provided Counter object.
	*/

	fstream inFile; // input file object
	string line; // temporary variable to store each line as a string of chars
	string word; // temporary variable to store each word
	stringstream lineStream; // temporary stream buffer for each line
	int endPos = startPos + eachLines; // ending line position
	int processedLineCount = 0;
    // regex wordRegex("[a-zA-Z]+");  // use regex to remove puntuations
    regex wordRegex("([^\\W_]+(?:['_-][^\\W_]+)*)");

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

        // Extract words using regex
        sregex_iterator it(line.begin(), line.end(), wordRegex);
        sregex_iterator end;

        // Loop over each matching words
        for (; it != end; ++it) {
            smatch match = *it;
            word = match.str();

            // Skip if word length is smaller than min or larger than max
            if (word.length() < minWordLen || word.length() > maxWordLen)
                continue;

            // Transform to lowercase
            transform(word.begin(), word.end(), word.begin(), ::tolower);

            update_counter(counter, word);
        }

        /*
		// clear buffer flags and load the current line into stream buffer
		lineStream.clear();
		lineStream.str(line);
        // process each 'word' in this line, adding/updating the word counter
		while (lineStream >> word) {
			// word text cleaning goes here!
            update_counter(counter, word);
		}
        */

		processedLineCount++;
	}

	inFile.close();
}

void prompt_user(int rank, vector<string>& allFilenames, int& minWordLen, int& maxWordLen) {
	/* Get input from user. Store input values in arguments.*/
	
	int numberOfFiles = 0;
	char tempFilename[FILENAME_SIZE] = ""; // for broadcasting individual filenames

	if (rank == ROOT) {
		cout << "Enter the number of text files to process: "; fflush(stdout);
		cin >> numberOfFiles;
		
		for (int i = 0; i < numberOfFiles; i++) {
			cout << "Enter the name of text file #" << (i + 1) << ": "; fflush(stdout);
			cin >> tempFilename;
			allFilenames.push_back(tempFilename);
		}
		
		/*
        // get the file name, if the file does not exist, prompt the user to enter it again
        while (strlen(filename) == 0) {
            cout << "Enter the file name: ";
            cin.getline(filename, FILENAME_SIZE);
            if (!is_file_exists(filename)) {
                cout << filename << " does not exist. Please enter again." << endl;
                filename[0] = '\0';
            }
        }
		*/

        // get min word length for inclusion
        while (minWordLen <= 0) {
            cout << "Enter the minimum word length to be included in the count (min 1): ";
            cin >> minWordLen;
            if (minWordLen <= 0) {
                cout << "Invalid input. Please enter a number greater than or equal to 1." << endl;
            }
        }

        // get max word length for inclusion
        while (maxWordLen <= 0) {
            cout << "Enter the maximum word length to be included in the count (min 1): ";
            cin >> maxWordLen;
            if (maxWordLen <= 0) {
                cout << "Invalid input. Please enter a number greater than or equal to 1." << endl;
            }
        }
	}
	
	// Broadcast number of files
	MPI_Bcast(&numberOfFiles, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
	
	// Broadcast all filenames
	for (int i = 0; i < numberOfFiles; i++) {
		if (rank == ROOT) 
			strncpy(tempFilename, allFilenames[i].c_str(), FILENAME_SIZE);
		
		MPI_Bcast(tempFilename, FILENAME_SIZE, MPI_CHAR, ROOT, MPI_COMM_WORLD);
		
		if (rank != ROOT)
			allFilenames.push_back(tempFilename);
	}

    // Broadcast min word length
	MPI_Bcast(&minWordLen, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    // Broadcast max word length
	MPI_Bcast(&maxWordLen, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
}

int main(int argc, char* argv[]) {
	// MPI environment variables
	int nprocs, rank; // number of processes and rank/id of each process

	// Initialize the MPI environment
	MPI_Init(&argc, &argv);

	// Get the number of processes and rank of each process
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// User input variables, shared and constant across each proc
    int minWordLen = 0; // minimum word length for inclusion
    int maxWordLen = 0; // maximum word length for inclusion
	vector<string> allFilenames; // list of all the text files
	
	// Define word counter to store the frequency of words of all the text files [ROOT use only]
	Counter allWordCounter; 
	
	// Prompt user for input
	prompt_user(rank, allFilenames, minWordLen, maxWordLen);
	
	// loop over each text file, parallely computing the frequency of words in each file one at a  
	// time, and updating the contents of the `allWordCounter` at the end of each iteration
	for (string filename : allFilenames) {
	
		int totalLines; // total number of lines in the file [ROOT use only]
		int eachLines; // number of lines for each process to work on
		int remLines; // number of remaining lines for root proccess to work on [ROOT use only]
		Counter eachWordCounter; // word counter of this process

		// ROOT proc determines the total number of lines and each process's line count
		if (rank == ROOT) {
			totalLines = get_line_count(filename);
			eachLines = totalLines / nprocs; // divided equally between the processes
			remLines = totalLines % nprocs;

			// handle work for remaining lines
			process_lines(filename, remLines, (eachLines*nprocs), eachWordCounter, minWordLen, maxWordLen);
		}

		// Broadcast each process's line count
		MPI_Bcast(&eachLines, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

		// Each process works on their section of the text file
		process_lines(filename, eachLines, (eachLines*rank), eachWordCounter, minWordLen, maxWordLen);

		// Decomposing the counter of each proc into words and counts objects
		string eachWords;
		vector<int> eachCounts;
		decompose_counter(eachWordCounter, eachWords, eachCounts);

		// Variables/arguments for count data gathering
		int sendIntAmount; 			// number of ints that each proc sends to ROOT proc
		int *gatheredIntBuf = NULL; // large buffer array to store the gathered ints from each proc
									// [ROOT use only]
		int recvIntAmounts[nprocs]; // array of number of ints the ROOT proc receives from each proc
									// [ROOT use only]
		int intBufOffsets[nprocs]; 	// array of index offsets for `gatheredIntBuf` to correctly gather
									// and load contiguous ints [ROOT use only]
		int intBufSize; 			// total size of `gatheredIntBuf` to be computed [ROOT use only]

		// Variables/arguments for word data gathering
		int sendCharAmount;				// number of chars that each proc sends to ROOT proc
		char *gatheredCharBuf = NULL; 	// large buffer array to store the gathered chars from each proc
										// [ROOT use only]
		int recvCharAmounts[nprocs]; 	// array of number of chars the ROOT proc receives from each
										// proc [ROOT use only]
		int charBufOffsets[nprocs]; 	// array of index offsets for `gatheredCharBuf` to correctly
										// gather and load contiguous chars [ROOT use only]
		int charBufSize;				// total size of `gatheredIntBuf` to be computed [ROOT use only]


		// get the amount of ints for each proc to send, then gather the amounts in ROOT proc
		sendIntAmount = eachCounts.size();
		MPI_Gather(&sendIntAmount, 1, MPI_INT, recvIntAmounts, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

		// get the amount of chars for each proc to send, then gather the amounts in ROOT proc
		sendCharAmount = eachWords.size();
		MPI_Gather(&sendCharAmount, 1, MPI_INT, recvCharAmounts, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

		// ROOT proc computes and sets the index offsets and total sizes of the two large buffer arrays,
		// also allocates memory for the two large buffer arrays.
		if (rank == ROOT) {
			intBufSize = charBufSize = 0; // count fromm 0 to help fill offset values
			for (int r = 0; r < nprocs; r++) {
				// index offsets are based on the rank values of each proc along with the total number
				// of procs

				// Compute index offsets and total size for large buffer array of ints (count data)
				intBufOffsets[r] = intBufSize; // displacement relative to `gatheredIntBuf`
				intBufSize += recvIntAmounts[r]; // accumulate the total size

				// Compute index offsets and total size for large buffer array of chars (words data)
				charBufOffsets[r] = charBufSize; // displacement relative to `gatheredCharBuf`
				charBufSize += recvCharAmounts[r]; // accumulate the total of all counter sizes
			}

			gatheredIntBuf = new int[intBufSize]; // allocate buffer memory based on total size
			charBufSize += 1; // additional space for null terminating character
			gatheredCharBuf = new char[charBufSize]; // allocate buffer memory based on total size
		}

		// Gather counts (int data) into large contiguous buffer
		MPI_Gatherv(&eachCounts[0], sendIntAmount, MPI_INT,
			gatheredIntBuf, recvIntAmounts, intBufOffsets, MPI_INT,
			ROOT, MPI_COMM_WORLD);

		// Gather words (char data) into large contiguous buffer
		MPI_Gatherv(eachWords.c_str(), sendCharAmount, MPI_CHAR,
			gatheredCharBuf, recvCharAmounts, charBufOffsets, MPI_CHAR,
			ROOT, MPI_COMM_WORLD);

		/* Create a new word counter and merge all counter data from each proc */

		if (rank == ROOT) {
			gatheredCharBuf[charBufSize-1] = '\0'; // just in case

			// checking the pairing of data points (word-count) between the int and char buffers
			int newlineCount = 0;
			for (int i = 0; i < charBufSize; i++) {
				if (gatheredCharBuf[i] == '\n')
					newlineCount++;
			}
			if (newlineCount != intBufSize) {
				cout << "Error! Data not aligned. Aborting program." << endl;
				exit(1);
			}
			// Create equivalent objects from respective buffers via copy constructors
			vector<int> gatheredCounts(gatheredIntBuf, gatheredIntBuf + intBufSize);
			string gatheredWords(gatheredCharBuf);

			// Free memory of large buffers
			delete[] gatheredIntBuf;
			delete[] gatheredCharBuf;

			// Recompose a counter from gathered data
			Counter mergedCounter;
			compose_counter(mergedCounter, gatheredWords, gatheredCounts);

			// Update the contents of `allWordCounter`
			update_counter(allWordCounter, mergedCounter);
		}
	} // end of for-loop
	
	if (rank == ROOT) {
		// Output the final report
        cout << "---------------------------------------------" << endl;
		cout << "|             Word Count Report             |" << endl;
        print_counter(allWordCounter, most_common);
		cout << "Unique words: " << get_counter_size(allWordCounter) << endl;
		cout << "Total words: "<< get_counter_total(allWordCounter) << endl;
	}

	// Finalize the MPI environment.
    MPI_Finalize();

	return 0;
}