#include "gif_parser.hpp"

#include <iterator>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
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

std::vector<PixelTrack> getPixelTracks(char** paths, int npaths) {
    std::unordered_set<PixelTrack> uniqueTracks;

    for (int i = 0; i < npaths; ++i) {
        PixelTrack track;
        std::ifstream ifs{paths[i]};
        ifs >> track;
        uniqueTracks.insert(std::move(track));
    }

    return std::vector<PixelTrack>{uniqueTracks.begin(), uniqueTracks.end()};
}

int main(int argc, char** argv) {
    std::vector<PixelTrack> tracks = getPixelTracks(&argv[1], argc-1);
    std::cout << tracks.size() << " Tracks " << std::endl;

    int max_time = 0;
    for (auto& track : tracks) {
        max_time = std::max(max_time, track.length());
    }

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