extern "C" {
	#include "gifdec.h"
}
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <thread>

class PixelGif {
public:
	PixelGif(gd_GIF* gif) : _gif(gif) {
		next_frame();
	}
	PixelGif(const std::string& path) : PixelGif(gd_open_gif(path.c_str())) {
		
	}

	PixelGif(const PixelGif& other) = delete;
	PixelGif(PixelGif&& other) : _gif(other._gif) {
		other._gif = nullptr;
	}

	~PixelGif() {
		if (_gif != nullptr) {
			gd_close_gif(_gif);
			_gif = nullptr;
		}
	}

	PixelGif& operator=(const PixelGif&) = delete;
	PixelGif& operator=(PixelGif&& other) {
		_gif = other._gif;
		other._gif = nullptr;
		return *this;
	}

	const gd_GCE& gce() const {
		return _gif->gce;
	}

	bool done() const {
		return _done;
	}

	bool next_frame() {
		_done = gd_get_frame(_gif) == -1;
		return !done();
	}

	int pixel_value() const {
		return _gif->frame[0];
	}

private:

	gd_GIF* _gif;
	bool _done;
};

class PixelTrack;

namespace std {
	template<>
	struct hash<PixelTrack> {
		std::size_t operator()(const PixelTrack& pixel) const;
	};
};

class PixelTrack {
public:

	static constexpr int pallet_black = 0;
	static constexpr int pallet_white = 1;

	PixelTrack() = default;
	PixelTrack(std::map<int, int> bits, int length) : _bits(std::move(bits)), _length(length) {}

	static PixelTrack from_gif(PixelGif gif) {
		std::map<int, int> bits;
		int total_delay = 0;

		while (gif.next_frame()) {
			int delay = gif.gce().delay;

			bits[total_delay] = gif.pixel_value();
			total_delay += delay;
		}

		if ((--bits.end())->second == pallet_black) {
			bits[total_delay] = pallet_white;
		}

		return PixelTrack(std::move(bits), total_delay);
	}

	int value(int time) const {
		auto it = _bits.upper_bound(time);
		--it;
		return it->second;
	}

	int length() const {
		return _length;
	}

	const std::map<int, int>& bits() const {
		return _bits;
	}

	bool operator==(const PixelTrack& pixel) const {
		return _bits == pixel._bits;
	}

	friend std::ostream& operator<<(std::ostream& os, const PixelTrack& pixel) {
		os << "PixelTrack " << pixel._bits.size() << ' ' << pixel._length << '\n';
		for (auto& p : pixel._bits) {
			os << p.first << ' ' << p.second << '\n';
		}
		return os;
	}

	friend std::istream& operator>>(std::istream& is, PixelTrack& pixel) {
		std::string name;
		int size, length;
		is >> name >> size >> length;

		if (is && name == "PixelTrack") {
			pixel._bits.clear();
			pixel._length = length;

			for (int i = 0; i < size; ++i) {
				int k, v;
				is >> k >> v;
				pixel._bits[k] = v;
			}
			is.ignore();
		}
		return is;
	}

	friend std::size_t std::hash<PixelTrack>::operator()(const PixelTrack& pixel) const;
	
private:
	int _length;
	std::map<int, int> _bits;
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

std::size_t std::hash<PixelTrack>::operator()(const PixelTrack& pixel) const {
	std::size_t seed = 0;
	for (auto& p : pixel._bits) {
		hash_combine(seed, p.first);
		hash_combine(seed, p.second);
	}
	return seed;
}
