#include "gif_parser.hpp"

#include <iterator>
#include <fstream>
#include <unordered_map>
#include <cassert>

char hex_digits[] = "0123456789ABCDEF";

std::string frameState(const std::vector<PixelTrack>& tracks, int time) {
    std::string hex_string;

    int bits = 0;
    int nbits = 0;

    int ntracks = 0;
    for (auto& track : tracks) {
        int val = track.value(time);
        if(!(val == 0 || val == 1)) {
            std::cerr << "val=" << val << "\n";
            std::exit(1);
        }
        bits = (bits << 1) | val;
        nbits++;
        if (nbits == 4) {
            hex_string.push_back(hex_digits[bits]);
            nbits = 0;
            bits = 0;
        }
    }
    return hex_string;
}

int main(int argc, char** argv) {
    std::vector<PixelTrack> tracks;

    int max_time = 0;

    for (int i = 1; i < argc; ++i) {
        tracks.emplace_back();
        std::ifstream ifs{argv[i]};
        ifs >> tracks.back();
        max_time = std::max(max_time, tracks.back().length());
    }

    std::cout << tracks.size() << " Tracks " << std::endl;

    std::unordered_map<std::string, int> frameStates;
    std::vector<std::string> frameStatesSorted;
    std::vector<int> cipher;

    int count = 0;

    for (int time = 0; time < max_time; time += 40) {
        auto state = frameState(tracks, time);
        auto it = frameStates.find(state);
        if (it == frameStates.end()) {
            frameStatesSorted.push_back(state);
            auto p = frameStates.insert(std::make_pair(std::move(state), count++));
            it = p.first;
        }
        cipher.push_back(it->second);
    }

    std::ofstream frameStateFile{"states.txt"};
    for (int i = 0; i < frameStatesSorted.size(); ++i) {
        frameStateFile << i << ' ' << frameStatesSorted[i] << '\n';
    }

    std::ofstream cipherFile{"cipher.txt"};
    for (int x : cipher) {
        cipherFile << x << '\n';
    }
}