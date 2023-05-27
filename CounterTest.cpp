/*Program for testing Counter object.*/

#include <iostream>
#include <string>
#include <vector>
#include "Counter.h"

int main() {
	Counter fruit_basket; // define an empty dict
	
	// insert key-value pair(s)
	fruit_basket["apple"] = 5;
	fruit_basket.insert( {"pear", 2} );
	fruit_basket.insert( {{"orange", 5}, {"mango", 1}} );
	
	// show dict's content
	print_counter_content(fruit_basket, "fruit basket");
	
	// list of fruits to add
	std::string new_fruits[10] = {
		"apple", "orange", "orange", "mango", "apple",
		"banana", "orange", "banana", "guava", "mango"
	};
	
	// add fruits in the list to the dict
	for(std::string fruit : new_fruits) {
		add_counter_item(fruit_basket, fruit);
		std::cout << "Adding fruit '" << fruit << "'..." << std::endl;
	}
	std::cout << std::endl;
	
	// show dict's content again to check
	print_counter_content(fruit_basket, "fruit basket");
	
	std::string counter_keys; // each word separated by newline character
	std::vector<int> counter_values;
	decompose_counter(fruit_basket, counter_keys, counter_values);
	
	std::cout << counter_keys;
	std::cout << "[" << counter_keys.size() << " chars including newlines]\n";
	
	for (int i : counter_values)
		std::cout << i << std::endl;
	
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Composing counter from strings and int vector..." << std::endl;
	
	Counter new_basket;
	compose_counter(new_basket, counter_keys, counter_values);
	print_counter_content(new_basket, "new basket");
	
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