/* 
Program for testing Counter object; demonstrating functions in "Counter.h". 

Compile with:
	$ g++ CounterTest.cpp -o CounterTest

Run with:
	$ ./CounterTest
*/

#include "Counter.h"

int main() {
	// Defining an empty Counter ...
	Counter fruit_basket; 
	
	// Initializing the Counter by inserting key-value pair(s), in multiple ways ...
	fruit_basket["apple"] = 5; // direct access operator
	fruit_basket.insert( {"pear", 2} ); // single pair
	fruit_basket.insert( {{"orange", 5}, {"mango", 1}} ); // multiple pairs
	
	std::cout << "Contents of fruit_basket:-" << std::endl;
	print_counter(fruit_basket);
	
	
	// Updating the Counter, given a single word ...
	update_counter(fruit_basket, "papaya", 2); // adding a new word with count of 2
	update_counter(fruit_basket, "mango", 2); // update count of an existing word by 2
	update_counter(fruit_basket, "dragonfruit"); // adding new word, default count of 1
	update_counter(fruit_basket, "dragonfruit"); // update count of an existing word by 1
	
	// Updating the Counter using a vector of words ...
	std::vector<std::string> some_fruits = {
		"apple", "orange", "orange", "mango", "apple",
		"banana", "orange", "banana", "guava", "mango"
	};
	update_counter(fruit_basket, some_fruits);
	
	// Updating the Counter using another Counter ...
	Counter small_fruit_basket( {{"banana", 2}, {"avocado", 1}, {"pear", 3}} );
	update_counter(fruit_basket, small_fruit_basket);
	
	std::cout << "Contents of updated fruit_basket:-" << std::endl;
	print_counter(fruit_basket);
	
	// Querying the size of the Counter ...
	std::cout << "Size of fruit_basket (i.e. number of unique fruits): " 
		<< get_counter_size(fruit_basket) << std::endl;
	
	// Calculating the total count in the Counter ...
	std::cout << "Total count in fruit_basket: " << get_counter_total(fruit_basket) << std::endl;
	std::cout << std::endl;


	// Decomposing the Counter contents into contiguous data objects ...
	
	std::string counter_keys; // to contiguously store the counter keys (string) where the keys are separated by a newline char
	std::vector<int> counter_values; // to contiguously store the counter values (int) where each value is an element
	decompose_counter(fruit_basket, counter_keys, counter_values);
	
	std::cout << "Decomposition of fruit_basket:-" << std::endl;
	std::cout << "Fruit names (counter keys):" << std::endl << counter_keys;
	std::cout << "Fruit count (counter values):" << std::endl;
	for (int i : counter_values)
		std::cout << i << std::endl;
	std::cout << std::endl;
	
	
	// Recomposing the Counter from contiguous data objects ...
	
	Counter new_fruit_basket; // new Counter
	
	compose_counter(new_fruit_basket, counter_keys, counter_values);
	std::cout << "(Re)composition of new_fruit_basket:-" << std::endl;
	print_counter(new_fruit_basket);
	
	
	// Printing Counter contents in some sorted order ...
	
	std::cout << "Sorted new_fruit_basket by alphabetical order of words:-" << std::endl;
	print_counter(new_fruit_basket, alphabetical);
	
	std::cout << "Sorted new_fruit_basket by most common counts:-" << std::endl;
	print_counter(new_fruit_basket, most_common);
	return 0;
}


/*
Note: While we can use the [] operator to access the elements, it is preferable to use the at() method.

This is because at() throws an exception whenever the specified key doesn't exist, 
while [] creates a new pair with the key and pairs the key with a garbage value.

We can use the following methods to check if the specified key exists in the unordered map.

    find() - returns the iterator to the element if the key is found, else returns the .end() iterator
    count() - returns 1 if the key exists and 0 if not

*/