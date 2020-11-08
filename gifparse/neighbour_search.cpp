#include "gif_parser.hpp"

#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>

class TracksMap {
public:

    TracksMap(char** paths, int npaths) {
        std::unordered_map<PixelTrack, std::vector<std::string>> uniqueTracks;

        for (int i = 0; i < npaths; ++i) {
            PixelTrack track;
            std::ifstream ifs{paths[i]};
            ifs >> track;
            uniqueTracks[std::move(track)].emplace_back(paths[i]);
        }

        for (auto& p : uniqueTracks) {
            size_t i = _tracks.size();
            _tracks.push_back(p.first);
            _trackPaths.emplace_back(std::move(p.second));
        }

        for (size_t i = 0; i < _trackPaths.size(); ++i) {
            for (auto& path : _trackPaths[i]) {
                _pathTracks[path] = i;
            }
        }

        for (auto& track : _tracks) {
            max_time = std::max(max_time, track.length());
        }

        std::cout << _tracks.size() << " tracks.\n";
    }

    float neighbour_discrepancy_score(const std::string& aPath, const std::string& bPath) const {
        return neighbour_discrepancy_score(_pathTracks.at(aPath), _pathTracks.at(bPath));
    }

    float neighbour_discrepancy_score(size_t i, size_t j) const {
        const PixelTrack& a = _tracks[i], b = _tracks[j];

        auto& aBits = a.bits();
        auto& bBits = b.bits();

        auto itA = aBits.begin();
        auto itB = bBits.begin();

        int t = 0;

        int discrepancies = 0;

        // for (; t <= std::max(a.length(), b.length()); t += 40) {
        //     discrepancies += (a.value(t) != b.value(t));
        // }

        // return discrepancies;

        while (itA != aBits.end() && itB != bBits.end()) {
            auto nextItA = std::next(itA), nextItB = std::next(itB);

            if (nextItA != aBits.end() && nextItB != bBits.end()) {
                if (nextItA->first <= nextItB->first) {
                    if (itA->second != itB->second) {
                        // std::cout << "Discrepancy [" << t << ", " << nextItA->first << ") -> " << itA->second << '/' << itB->second << '\n';
                        discrepancies += nextItA->first - t;
                    } else {
                        // std::cout << "Match [" << t << ", " << nextItA->first << ") -> " << itA->second << '\n';
                    }
                    t = nextItA->first;
                    itA = nextItA;
                } else {
                    if (itA->second != itB->second) {
                        // std::cout << "Discrepancy [" << t << ", " << nextItB->first << ") -> " << itB->second << '/' << itA->second << '\n';
                        discrepancies += nextItB->first - t;
                    } else {
                        // std::cout << "Match [" << t << ", " << nextItB->first << ") -> " << itB->second << '\n';
                    }
                    t = nextItB->first;
                    itB = nextItB;
                }
            } else if (nextItA != aBits.end()) {
                if (itA->second != PixelTrack::pallet_white) {
                    // std::cout << "Discrepancy [" << t << ", " << nextItA->first << ") -> " << itA->second << '/' << itB->second << '\n';
                    discrepancies += nextItA->first - t;
                } else {
                    // std::cout << "Match [" << t << ", " << nextItA->first << ") -> " << itA->second << '\n';
                }
                t = nextItA->first;
                itA = nextItA;
            } else if (nextItB != bBits.end()) {
                if (itB->second != PixelTrack::pallet_white) {
                    // std::cout << "Discrepancy [" << t << ", " << nextItB->first << ") -> " << itB->second << '/' << itA->second << '\n';
                    discrepancies += nextItB->first - t;
                } else {
                    // std::cout << "Match [" << t << ", " << nextItB->first << ") -> " << itB->second << '\n';
                }
                t = nextItB->first;
                itB = nextItB;
            } else {
                break;
            }
        }

        float score = discrepancies / (float) std::max(a.length(), b.length());
        // if (score < 0.0001) {
        //     std::cout << discrepancies << " / " << std::max(a.length(), b.length()) << " = " << score << "\n";
        // }

        return score;
    }

    std::vector<std::pair<int, float>> find_neighbours(const std::string& trackPath) const {
        return find_neighbours(_pathTracks.at(trackPath));
    }

    std::vector<std::pair<int, float>> find_neighbours(size_t index) const {
        const size_t maxNeighbours = 24;

        std::vector<std::pair<int, float>> trackScores;
        for (size_t i = 0; i < _tracks.size(); ++i) {
            if (i != index) {
                float score = neighbour_discrepancy_score(index, i);
                // std::cout << index << "-" <<  i << " score: " << score << "\n";
                if (score < 0.01) {
                    trackScores.emplace_back(i, score);
                }
            }
        }

        std::sort(
            trackScores.begin(),
            trackScores.end(),
            [&](auto& p1, auto& p2) {
                return p1.second < p2.second;
            });

        if (trackScores.size() > maxNeighbours) {
            trackScores.resize(maxNeighbours);
        }
        return trackScores;
    }

    static std::string calculate_output_path(const std::string& path) {
        int lastSlash = path.find_last_of('/');

        if (lastSlash == std::string::npos) {
            lastSlash = -1;
        }
        return "neighbours/" + path.substr(lastSlash+1);
    }
    

    void find_all_neighbours() const {
        std::mutex ioMut;
        int nthreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        for (size_t threadId = 0; threadId < nthreads; ++threadId) {
            threads.emplace_back([this, threadId, nthreads, &ioMut]{
                for (size_t i = threadId; i < _tracks.size(); i += nthreads) {
                    auto neighbours = find_neighbours(i);
                    std::ofstream ofs{calculate_output_path(_trackPaths[i].front())};
                    for (auto& p : neighbours) {
                        ofs << p.second << ' ' << _trackPaths[i].front() << ' ' << _trackPaths[p.first].front() << '\n'; 
                    }
                    if (i % (nthreads * 10) == 0)
                    std::lock_guard<std::mutex> lock{ioMut};
                    std::cout << "i = " << i << std::endl;
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

private:

    int max_time = 0;
    std::vector<PixelTrack> _tracks;
    std::vector<std::vector<std::string>> _trackPaths;
    std::unordered_map<std::string, std::size_t> _pathTracks;
};

int main(int argc, char** argv) {
    TracksMap tracksMap(&argv[1], argc-1);

    tracksMap.find_all_neighbours();
}