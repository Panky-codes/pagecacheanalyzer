#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <string_view>
#include <unordered_map>

namespace helpers {
static inline bool found_str(std::string& str, const char* substr) {
	return str.find(substr) != std::string::npos;
}
}  // namespace helpers

struct cache_entry {
	ssize_t start;
	ssize_t end;

	cache_entry(ssize_t start, ssize_t end) : start(start), end(end) {}

	bool operator<(const cache_entry& other) const {
		return (start < other.start) && (end <= other.start);
	}
};
using mapping_t = std::unordered_map<std::string, std::set<cache_entry>>;

void print_help() {
	std::cout << "Usage: cacheanalyzer [dev] [filename]" << std::endl;
	exit(1);
}

void dump_state(const mapping_t& cache_map) {
	for (auto& cache : cache_map) {
		for (auto& a : cache.second)
			std::cout << " ino " << cache.first
				  << " ofs = " << a.start << " end = " << a.end
				  << std::endl;
		std::cout << std::endl;
	}
}

int main(int argc, char** argv) {
	std::string line;
	std::smatch match;
	mapping_t cache_map;

	if (argc != 3) print_help();
	std::ifstream file{argv[2]};
	if (!file.good()) {
		std::cerr << "Input file not found" << std::endl;
		exit(1);
	}

	std::string dev_pattern =
	    std::string("dev ") + argv[1] + std::string(":\\d+");
	std::string pattern_str =
	    dev_pattern +
	    std::string(" ino (\\w+) pfn=0x\\w+ ofs=(\\d+) order=(\\d+)");
	std::regex pattern(pattern_str);

	while (std::getline(file, line)) {
		if (std::regex_search(line, match, pattern)) {
			std::string inode = match[1];  // inode is in hex
			ssize_t nr_pages = 1U << std::stoi(match[3]);
			ssize_t offset = std::stoull(match[2]) >> 12;
			auto cache = cache_map.contains(inode)
					 ? cache_map[inode]
					 : std::set<cache_entry>{};

			if (helpers::found_str(line, "add")) {
				if (cache.contains(
					{offset, offset + nr_pages})) {
					std::cout << "Reinserting at " << offset
						  << " to " << offset + nr_pages
						  << std::endl;
					std::cout << line << std::endl;
					dump_state(cache_map);
				}

				cache.insert({offset, offset + nr_pages});
			}

			else if (helpers::found_str(line, "delete")) {
				if (!cache.contains(
					{offset, offset + nr_pages})) {
					std::cout << "Illegal Deletion at "
						  << offset << " to "
						  << offset + nr_pages
						  << std::endl;
					std::cout << line << std::endl;
					dump_state(cache_map);
				}

				cache.erase({offset, offset + nr_pages});
			}

			cache_map.insert_or_assign(inode, cache);
		}
	}

	dump_state(cache_map);
	return 0;
}
