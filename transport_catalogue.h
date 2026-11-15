#pragma once

#include <unordered_map>
#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <optional>
#include <memory>
#include <algorithm>

#include "domain/geo.h"
#include "domain/domain.h"

namespace catalogue {
	using namespace domain;

	class DistanceHasher {
	public:
		size_t operator()(const std::pair<StopPtr, StopPtr>& obj) const {
			return std::hash<StopPtr>()(obj.first) ^ std::hash<StopPtr>()(obj.second);
		}
	};

	class TransportCatalogue {
	public:
		void AddBus(std::string_view bus_id, bool is_roundtrip, const std::vector<std::string_view>& route);
		BusPtr GetBus(std::string_view bus_id) const;
		void AddStop(std::string_view stop_name, parameters::Coordinates pos);
		StopPtr GetStop(std::string_view stop_name) const;
		BusInfo GetRouteInfo(std::string_view bus_id) const;
		std::unordered_set<BusPtr> GetStopInfo(std::string_view stop_name) const;
		void SetDistance(std::string_view cur_stop_name, std::string_view add_stop_name, int distance);
		inline const std::deque<Bus>& GetBusList() const { return bus_pool_; }
		inline const std::unordered_map<std::string_view, StopPtr>& GetStopList() const { return stops_; }
		const std::optional<int> GetDistance(std::string_view first_stop, std::string_view next_stop) const;

	private:
		Route CreateRoute(const std::vector<std::string_view>& source_of_stops) const;
		void ChangeDistance(StopPtr cur_stop, StopPtr add_stop, int distance);

	private:
		std::unordered_map<std::string_view, StopPtr> stops_;
		std::unordered_map<std::string_view, BusPtr> buses_;
		std::unordered_map<StopPtr, std::unordered_set<BusPtr>> stop_to_buses_;
		std::unordered_map<std::pair<StopPtr, StopPtr>, int, DistanceHasher> distance_between_stops_;

		std::deque<Stop> stop_pool_;
		std::deque<Bus> bus_pool_;
	};
} //namespace catalogue
