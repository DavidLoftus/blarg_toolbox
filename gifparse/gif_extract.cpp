#include "gif_parser.hpp"

#include <filesystem>
#include <sstream>
#include <fstream>

PixelTrack parse_gif(const std::string& path) {
	PixelGif gif{path};
	auto track = PixelTrack::from_gif(std::move(gif));
	return track;
}

std::string outputPath(const std::string& path) {
	int lastSlash = path.find_last_of('/');

	if (lastSlash == std::string::npos) {
		lastSlash = -1;
	}

	std::ostringstream oss;
	oss << "tracks/" << path.substr(lastSlash+1) << ".dat";

	return std::filesystem::absolute(oss.str());
}

void extract(const std::string& path) {
	std::string out_path = outputPath(path);
	if (!std::filesystem::exists(out_path)) {
		auto track = parse_gif(path);
		std::cout << out_path << std::endl;
		std::ofstream fs{out_path};
		fs << track;
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: %s <path>", argv[0]);
		return 1;
	}

	std::vector<std::thread> threads;
	int nthreads = std::thread::hardware_concurrency();
	std::cout << "Using " << nthreads << " threads" << std::endl;
	for (int i = 1; i <= nthreads; i++) {
		threads.emplace_back(
			[=]{
				for (int j = i; j < argc; j += nthreads) {
					extract(argv[j]);
				}
			}
		);
	}

	for (auto& thread : threads) {
		thread.join();
	}
}
