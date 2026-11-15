#pragma once

#include "transport_catalogue.h"
#include "domain/router.h"

namespace catalogue {
	struct EdgeInfo {
		std::string_view object_route_name{};
		std::optional<int> span_count = std::nullopt;
		double weight = 0.0;
		bool is_wait = false;
	};

	struct RouteSettings {
		int bus_wait_time = 0;
		double bus_velocity = 0.0;
	};

	class TransportRouter {
		using RouteInfo = std::vector<EdgeInfo>;
	private:
		struct StopVertex {
			graph::VertexId bus_vertex{};
			graph::VertexId wait_vertex{};
		};
		
		void AddStopEdges(const std::unordered_map<std::string_view, domain::StopPtr>& stop_list);
		
		template <typename It>
		void AddBusEdges(It range_begin, It range_end, const domain::Bus& bus) {
			if (range_begin == range_end) return;

			std::vector<double> distances_between_neighbors;
			distances_between_neighbors.reserve(std::distance(range_begin, range_end));

			for (auto it = range_begin; std::next(it) != range_end; ++it) {
				distances_between_neighbors.push_back(static_cast<double>(catalogue_.GetDistance((*it)->name, (*std::next(it))->name).value()));
			}

			int i_idx = 0;
			for (auto i_it = range_begin; i_it != range_end; ++i_it, ++i_idx) {
				double total_distance = 0.0;
				int span_count = 0;

				for (auto j_it = std::next(i_it); j_it != range_end; ++j_it) {
					total_distance += distances_between_neighbors[i_idx + span_count];
					++span_count;

					double road_time = total_distance / (settings_.bus_velocity * 1000.0 / 60.0);

					graph_.AddEdge({
						stop_vertex_[(*i_it)->name].bus_vertex,
						stop_vertex_[(*j_it)->name].wait_vertex,
						road_time
						});

					edges_info_.push_back({ bus.id, span_count, road_time, false });
				}
			}
		}

		void BuildGraph();

	public:
		TransportRouter() = default;

		TransportRouter(TransportCatalogue& catalogue, RouteSettings settings)
			: catalogue_(catalogue)
			, settings_(settings) {
			BuildGraph();
		}

		const std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;

	private:
		TransportCatalogue& catalogue_;
		graph::DirectedWeightedGraph<double> graph_;
		std::unordered_map<std::string_view, StopVertex> stop_vertex_;
		RouteInfo edges_info_;
		std::unique_ptr<graph::Router<double>> router_ = nullptr;
		RouteSettings settings_;
	};
}
