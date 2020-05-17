#include "dht_crawler.h"
#include <iostream>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost::property_tree;

template <class T>
void try_set_property_from_ptree(T& to_set, const string key, const ptree& pt)
{

	try {
		to_set = pt.get<T>(key);
	} catch (const ptree_bad_path& e) {
		// don't have this property, ignore
		return;
	} catch (const ptree_bad_data& e) {
		// failed translating data
		std::cout << "ignoring illegal value for " << key << endl;
	}

}

dht_crawler* create_crawler(){
	dht_crawler* crawler = new dht_crawler("result.txt", 20, 32900, 1800);
	return crawler;
}

int main() {

	std::cout << "creating crawler object" << endl;
	dht_crawler* crawler = create_crawler();

	if (crawler == NULL) {
		std::cout << "failed creating the crawler object" << endl;
		exit(0);
	}

	std::cout << "creating crawler sessions" << endl;
	crawler->create_sessions();

	std::cout << "running crawler" << endl;
	crawler->run();

	return 0;
}