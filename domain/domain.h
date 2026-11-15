#pragma once

#include <string_view>
#include <deque>

#include "geo.h"

namespace domain {
	struct Stop {
		std::string name;
		parameters::Coordinates coordinates;
	};

	struct BusInfo {
		int count_stops;
		int count_unique_stops;
		int real_distance;
		double curvature;

		bool NotFound() const {
			return count_stops == 0 && count_unique_stops == 0
				     && real_distance == 0.0 && curvature == 0;
		}
	};

	using StopPtr = const Stop*;
	using Route = std::deque<StopPtr>;

	struct Bus {
		std::string id;
		Route route;
		bool is_roundtrip;
	};

	using BusPtr = const Bus*;

} //namespace domain