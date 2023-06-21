/*
Group name: Kismet


*/

#include <iostream> 		// std::cout
#include <iomanip> 		    // std::setw
#include <string> 			// std::string
#include <unordered_map> 	// std::unordered_map, std::pair
#include <sstream> 			// std::stringstream
#include <vector> 			// std::vector
#include <algorithm> 		// std::sort

/* Counter type is essentially an unordered_map type with word (string) for key-value and
count (int) for mapped-value.
*/
typedef std::unordered_map<std::string, int> Counter;

void print_counter(Counter counter) {
	/* Print the contents of the Counter. */

	for (auto& item: counter)
		// std::cout << "'" << item.first << "': " << item.second << std::endl;
        std::cout << "| " << std::left << std::setw(20) << item.first << " | " << std::setw(5) << item.second << std::endl;
	std::cout << std::endl;
}

bool alphabetical( std::pair<std::string, int> x, std::pair<std::string, int> y) {
	/* Comparison function to be used in sorting Counter items (word-count pair) by comparing two
	items at a time. Enables sorting by alphabetical order of the words. To use, feed it as an
	argument to `print_counter()`.
	*/

	return x.first < y.first;
}

bool most_common( std::pair<std::string, int> x, std::pair<std::string, int> y) {
	/* Comparison function to be used in sorting Counter items (word-count pair) by comparing two
	items at a time. Enables sorting by most common count of words. To use, feed it as an argument
	to `print_counter()`.
	*/

	return x.second > y.second;
}

void print_counter(Counter counter,
	bool (*comp)(std::pair<std::string, int>, std::pair<std::string, int>)) {
	/* Print the contents of the Counter in sorted fashion, given a comparison function as a
	sorting key. Arguement options:
		alphabetical
		most_common
	*/

	// create a vector containing all the counter items, then sort it using the given comparison
	// function
	std::vector< std::pair<std::string, int> > counter_elements(counter.begin(), counter.end());
	std::sort(counter_elements.begin(), counter_elements.end(), comp);

    // print header
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "| " << std::left << std::setw(20) << "Word" << " | " << std::right << std::setw(6) << "Length" << " | " << std::setw(9) << "Frequency" << " |" << std::endl;
    // std::cout << "---------------------------------------------" << std::endl;
	// print the sorted vector of Counter items
	for (auto& item: counter_elements)
        // std::cout << "'" << item.first << "': " << item.second << std::endl;
        std::cout << "| " << std::left << std::setw(20) << item.first << " | " << std::right << std::setw(6) << item.first.length() << " | " << std::setw(9) << item.second << " |" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
	std::cout << std::endl;
}

void update_counter(Counter& counter, std::string word, int count) {
	/* Given a word string and count value, add the value of count to the current count of the
	word in the counter if the word exists, otherwise insert the new word and set it's count.
	Not meant to be used directly.
	*/

	if (counter.find(word) != counter.end())
		counter[word] += count; // update count
	else
		counter[word] = count; // set new word and count
}

void update_counter(Counter& counter, std::string word) {
	/* Given a word string, increment the count of the word in the counter by 1 if the word
	exists, otherwise insert the new word and set it's count to 1.
	*/

	update_counter(counter, word, 1);
}

void update_counter(Counter& counter, std::vector<std::string>& words) {
	/* For each word in the given vector of word strings, increment the count of the word in the
	counter if the word exists, otherwise insert the new word and set it's count to 1.
	*/

	for (std::string word: words)
		update_counter(counter, word, 1);
}

void update_counter(Counter& counter, Counter& other_counter) {
	/* Update the contents of the first counter with content from the second counter. */

	for (auto& pair: other_counter)
		update_counter(counter, pair.first, pair.second);
}

int get_counter_size(Counter& counter) {
	/* Return the size (number of unique words) in the counter. */

	return counter.size();
}

int get_counter_total(Counter& counter) {
	/* Return the total count of every word in the counter. */

	int total_count = 0;
	for (auto& pair: counter)
		total_count += pair.second;
	return total_count;
}

void decompose_counter(Counter counter, std::string& words, std::vector<int>& counts) {
	/* Decompose (break-down) the Counter into two main components of contiguous data objects:
		- a string of individual words, separated by newlines
		- an integer vector of counts of the respective words
	Store the results in the given string and integer vector objects, which can be later used to
	rebuild the Counter using the `compose_counter()` function.
	*/

	std::stringstream ss; // new stringstream object
	for (auto& pair: counter) {
		// Iterating through each word-count pair ensures that they are added to the objects in
		// the correct order. This is necessary to correctly recompose the Counter later.
		ss << pair.first << '\n'; // push word to end of ss, followed by newline char
		counts.push_back(pair.second); // push count value to end vector as new element
	}
	words = ss.str(); // convert stringstream to store in string object
}

void compose_counter(Counter& counter, std::string words, std::vector<int> counts) {
	/* (Re)compose (build) a Counter from two given components of contiguous data objects:
		- a string of individual words, separated by newlines
		- an integer vector of counts of the respective words
	Store the result in the given Counter object, which is assumed to be empty.
	Note: Assumes that there are an equal number of individual words and counts.
	*/

	std::stringstream ss(words); // convert string of words to stream
	std::string word; // temporary variable to hold a word
	int i = 0; // index for counts vector
	while (ss >> word)
		update_counter(counter, word, counts[i++]);
}