#include "transport_router.h"

namespace catalogue {

    void TransportRouter::AddStopEdges(const std::unordered_map<std::string_view, domain::StopPtr>& stop_list) {
        double wait_time = static_cast<double>(settings_.bus_wait_time);
        graph::VertexId current_vertex_id = 0;

        for (const auto& [stop_name, stop_ptr] : stop_list) {
            graph::VertexId bus_vertex = current_vertex_id++;
            graph::VertexId wait_vertex = current_vertex_id++;
            stop_vertex_[stop_name] = { bus_vertex, wait_vertex };

            graph_.AddEdge({ wait_vertex,
                             bus_vertex,
                             wait_time
                });

            edges_info_.push_back({ stop_name, std::nullopt, wait_time, true });
        }
    }

    void TransportRouter::BuildGraph() {
        auto stop_list = catalogue_.GetStopList();
        size_t count_stops = stop_list.size();
        if (count_stops == 0) return;

        const std::deque<domain::Bus>& bus_list = catalogue_.GetBusList();

        size_t computed_edges = count_stops * 2;
        graph_ = graph::DirectedWeightedGraph<double>(computed_edges);

        for (const auto& bus : bus_list) {
            size_t count_stops = bus.route.size();
            computed_edges += count_stops * count_stops * (bus.is_roundtrip ? 1 : 2);
        }
        edges_info_.reserve(computed_edges);
        
        AddStopEdges(stop_list);

        for (const auto& bus : bus_list) {
            const auto& stops = bus.route;
            if (stops.empty()) continue;

            AddBusEdges(stops.begin(), stops.end(), bus);
            if (!bus.is_roundtrip) AddBusEdges(stops.rbegin(), stops.rend(), bus);
        }
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    const std::optional<TransportRouter::RouteInfo> TransportRouter::GetRouteInfo(std::string_view from, std::string_view to) const {
        if (stop_vertex_.find(from) == stop_vertex_.end() || stop_vertex_.find(to) == stop_vertex_.end()) return std::nullopt;

        graph::VertexId from_vertex_id = stop_vertex_.at(from).wait_vertex;
        graph::VertexId to_vertex_id = stop_vertex_.at(to).wait_vertex;

        std::optional<graph::Router<double>::RouteInfo> route = router_->BuildRoute(from_vertex_id, to_vertex_id);

        if (route) {
            RouteInfo route_info;
            route_info.reserve(route->edges.size());

            for (auto edge_id : route->edges) {
                route_info.push_back(edges_info_[edge_id]);
            }
            return route_info;
        }
        return std::nullopt;
    }
}