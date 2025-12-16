#include <map>
#include <cstdlib>
#include <cstdint>

using namespace std;

int main() {
	map<uint32_t, uint64_t> asks; 
	map<uint32_t, uint64_t, greater<uint32_t>> bids;
}
