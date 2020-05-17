#include "dht_crawler.h"
#include <iostream>
#include <fstream>

using namespace std;

int main() {

	cout << "creating crawler object" << endl;

	// dht_crawler(std::string result_file, int session_num, int start_port, int total_intervals);
	dht_crawler crawler{"result.txt", 20, 32900, 1800};

	cout << "creating crawler sessions" << endl;
	crawler.create_sessions();

	cout << "running crawler" << endl;
	crawler.run();

}
