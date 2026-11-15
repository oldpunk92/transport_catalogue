#include "transport_catalogue.h"

namespace catalogue {
	void TransportCatalogue::AddBus(std::string_view bus_id, bool is_roundtrip, const std::vector<std::string_view>& route) {
		Route bus_route = CreateRoute(route);

		bus_pool_.push_back({ std::string(bus_id), bus_route, is_roundtrip });
		Bus* added_ptr = &bus_pool_.back();
		buses_[added_ptr->id] = added_ptr;

		for (const auto& stop_name : route) {
			StopPtr stop = GetStop(stop_name);
			stop_to_buses_[stop].insert(added_ptr);
		}
	}

	const Bus* catalogue::TransportCatalogue::GetBus(std::string_view bus_id) const {
		auto it = buses_.find(bus_id);
		if (it == buses_.end()) {
			return nullptr;
		}
		return it->second;
	}

	void TransportCatalogue::AddStop(std::string_view stop_name, parameters::Coordinates pos) {
		auto it = stops_.find(stop_name);
		if (it == stops_.end()) {
			stop_pool_.push_back({ std::string(stop_name), std::move(pos) });
			auto added_ptr = &stop_pool_.back();
			stops_[added_ptr->name] = added_ptr;
		}
		else {
			const_cast<Stop*>(it->second)->coordinates = { pos.lat, pos.lng };
		}
	}

	void TransportCatalogue::ChangeDistance(StopPtr cur_stop, StopPtr add_stop, int distance) {
		distance_between_stops_[{cur_stop, add_stop}] = distance;
	}

	void TransportCatalogue::SetDistance(std::string_view cur_stop_name, std::string_view add_stop_name, int distance) {
		if (stops_.find(add_stop_name) == stops_.end()) {
			stop_pool_.push_back({ std::string(add_stop_name), {0, 0} });
			auto added_ptr_empty_stop = &stop_pool_.back();
			stops_[added_ptr_empty_stop->name] = added_ptr_empty_stop;

			distance_between_stops_.insert({ {stops_[cur_stop_name], added_ptr_empty_stop}, distance });
			distance_between_stops_.insert({ {added_ptr_empty_stop, stops_[cur_stop_name]}, distance });
		}
		else {
			ChangeDistance(stops_[cur_stop_name], stops_[add_stop_name], distance);
			distance_between_stops_.insert({ { stops_[add_stop_name], stops_[cur_stop_name]}, distance });
		}
	}

	const std::optional<int> TransportCatalogue::GetDistance(std::string_view first_stop, std::string_view next_stop) const {
		auto it_first_stop = stops_.find(first_stop);
		auto it_next_stop = stops_.find(next_stop);
		if (it_first_stop == stops_.end() || it_next_stop == stops_.end()) return std::nullopt;

		StopPtr first_stop_ptr = it_first_stop->second;
		StopPtr next_stop_ptr = it_next_stop->second;

		auto it_distance = distance_between_stops_.find({ first_stop_ptr, next_stop_ptr });
		if (it_distance == distance_between_stops_.end()) return std::nullopt;
		return it_distance->second;
	}

	StopPtr TransportCatalogue::GetStop(std::string_view stop_name) const {
		auto it = stops_.find(stop_name);
		if (it == stops_.end()) {
			return nullptr;
		}
		return it->second;
	}

	BusInfo TransportCatalogue::GetRouteInfo(std::string_view bus_id) const {
		catalogue::BusInfo info{};
		int count_stops{};
		BusPtr bus = GetBus(bus_id);

		if (bus != nullptr) {
			std::unordered_set<std::string_view> unique_stops_name;
			for (StopPtr item : bus->route) {
				unique_stops_name.insert(item->name);
			}
			int count_unique_stops = (int)unique_stops_name.size();

			parameters::Coordinates start_coord = bus->route.front()->coordinates;
			StopPtr first_stop{};
			double geo_distance{};
			int real_distance{};
			bool is_first = true;

			for (StopPtr stop : bus->route) {
				if (is_first) {
					is_first = false;
					first_stop = stop;
					++count_stops;
					continue;
				}
				real_distance += distance_between_stops_.at({ first_stop, stop });
				geo_distance += ComputeDistance(start_coord, stop->coordinates);
				++count_stops;

				if (!bus->is_roundtrip) {
					real_distance += distance_between_stops_.at({ stop, first_stop });
					geo_distance += ComputeDistance(stop->coordinates, start_coord);
					++count_stops;
				}

				first_stop = stop;
				start_coord = stop->coordinates;
			}

			info.count_stops = count_stops;
			info.count_unique_stops = count_unique_stops;
			info.real_distance = real_distance;
			info.curvature = real_distance / geo_distance;
		}
		return info;
	}

	std::unordered_set<BusPtr> TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
		std::unordered_set<BusPtr> buses_of_stop{};

		StopPtr stop = GetStop(std::string(stop_name));
		if ((stop != nullptr) && (stop_to_buses_.find(stop) != stop_to_buses_.end())) {
			buses_of_stop = stop_to_buses_.at(stop);
		}
		return buses_of_stop;
	}

	Route TransportCatalogue::CreateRoute(const std::vector<std::string_view>& source_of_stops) const {
		Route new_route;
		for (const auto& stop_name : source_of_stops) {
			StopPtr bus_stop = GetStop(stop_name);
			if (bus_stop != nullptr) {
				new_route.push_back(std::move(bus_stop));
			}
		}
		return new_route;
	}
} //namespace catalogue