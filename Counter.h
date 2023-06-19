#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>

using namespace std;

typedef std::unordered_map<std::string, int> Counter;

void print_counter_content(Counter counter, std::string name) {
	std::cout << "Contents of the " << name << ":-" << std::endl;
	for (auto& item: counter)
		std::cout << "'" << item.first << "': " << item.second << std::endl;
	std::cout << std::endl;
}

void add_counter_item(Counter& counter, std::string key) {
	// if the key exists  ...
	if (counter.find(key) != counter.end())
		counter[key] += 1; // ... update count by 1
	else
		counter[key] = 1; // ... set new item count to 1
}

vector<string> getKeys(Counter& counter){
	vector<string> keys;
	for (auto& item: counter){
		keys.push_back(item.first);
	}
	return keys;
}

vector<int> getValues(Counter& counter){
	vector<int> values;
	for (auto& item: counter){
		values.push_back(item.second);
	}
	return values;
}

void decompose_counter(Counter& counter, std::string& keys, std::vector<int>& values) {
	std::stringstream ss;
	for (auto& item: counter) {
		ss << item.first << '\n';
		values.push_back(item.second);
	}
	keys = ss.str();
}

void compose_counter(Counter& counter, std::string keys, std::vector<int> values) {
	std::stringstream ss(keys);
	std::string key;
	
	int i = 0;
	while (ss >> key)
		counter[key] = values[i++];
}

void compose_counter(Counter& counter, vector<string> keys, vector<int> values) {
	for (int i=0 ; i<keys.size(); i++){
		if (counter.find(keys[i]) == counter.end()){ //not found
			counter[keys[i]] = values[i];
		} else {
			counter.at(keys[i]) += values[i];
		}
	}
}