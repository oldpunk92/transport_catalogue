#include "map_renderer.h"

namespace renderer {
    using namespace std::literals;

    void MapRenderer::CreateRoutePolylines(const std::deque<domain::Bus>& buses, const parameters::SphereProjector& projector, svg::ObjectContainer& container) const {
        size_t route_index{};
        for (const auto& bus : buses) {
            svg::Polyline route{};

            size_t color_index = route_index % render_settings_.color_palette.size();

            for (const auto& stop : bus.route) {
                svg::Point point = projector(stop->coordinates);
                route.AddPoint(point);
            }

            if (!bus.is_roundtrip) {
                bool is_first = true;
                for (auto it = bus.route.rbegin(); it != bus.route.rend(); ++it) {
                    if (is_first) {
                        is_first = false;
                        continue;
                    }
                    route.AddPoint(projector((*it)->coordinates));
                }
            }

            route
                .SetStrokeColor(render_settings_.color_palette[color_index])
                .SetFillColor("none")
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            ++route_index;
            container.Add(route);
        }
    }

    void MapRenderer::CreateStopPoints(std::vector<domain::StopPtr> stops, parameters::SphereProjector& projector, svg::ObjectContainer& container) const {
        for (auto stop : stops) {
            svg::Circle stop_point;
            svg::Point point = projector(stop->coordinates);

            stop_point
                .SetCenter({ point.x, point.y })
                .SetRadius(render_settings_.stop_radius)
                .SetFillColor("white");

            container.Add(stop_point);
        }
    }

    void MapRenderer::CreateBusTitles(const std::deque<domain::Bus>& buses, const parameters::SphereProjector& projector, svg::ObjectContainer& container) const {
        size_t route_index{};
        for (const auto& bus : buses) {
            svg::Text bus_id_underlayer{};
            svg::Text bus_id{};

            size_t color_index = route_index % render_settings_.color_palette.size();
            ++route_index;

            bus_id_underlayer
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetOffset({ render_settings_.bus_label_offset.x, render_settings_.bus_label_offset.y })
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus.id);
            bus_id
                .SetFillColor(render_settings_.color_palette[color_index])
                .SetOffset({ render_settings_.bus_label_offset.x, render_settings_.bus_label_offset.y })
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus.id);

            svg::Point pos{ projector(bus.route.front()->coordinates).x, projector(bus.route.front()->coordinates).y };
            container.Add(bus_id_underlayer.SetPosition(pos));
            container.Add(bus_id.SetPosition(pos));

            if (svg::Point pos{ projector(bus.route.back()->coordinates).x, projector(bus.route.back()->coordinates).y }; 
                !bus.is_roundtrip && bus.route.front()->name != bus.route.back()->name) {
                container.Add(bus_id_underlayer.SetPosition(pos));
                container.Add(bus_id.SetPosition(pos));
            }
        }
    }

    void MapRenderer::CreateStopTitles(std::vector<domain::StopPtr> stops, parameters::SphereProjector& projector, svg::ObjectContainer& container) const {
        svg::Text stop_name_underlayer{};
        svg::Text stop_name{};

        for (auto stop : stops) {
            container.Add(svg::Text()
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetPosition({ projector(stop->coordinates).x, projector(stop->coordinates).y })
                .SetOffset({ render_settings_.stop_label_offset.x, render_settings_.stop_label_offset.y })
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetData(std::string(stop->name)));
            container.Add(svg::Text()
                .SetFillColor("black")
                .SetPosition({ projector(stop->coordinates).x, projector(stop->coordinates).y })
                .SetOffset({ render_settings_.stop_label_offset.x, render_settings_.stop_label_offset.y })
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetData(std::string(stop->name)));
        }
    }

    void MapRenderer::Draw(svg::ObjectContainer& container) const {
        if (buses_.empty()) return;

        std::unordered_set<domain::StopPtr> stops_on_map;
        for (const auto& bus : buses_) {
            if (bus.route.empty()) return;
            for (const auto& stop : bus.route) {
                stops_on_map.insert(stop);
            }
        }
        auto sphere_projector = parameters::SphereProjector(stops_on_map.begin()
            , stops_on_map.end()
            , render_settings_.width
            , render_settings_.height
            , render_settings_.padding);

        std::deque<domain::Bus> sort_buses(buses_);
        std::sort(sort_buses.begin(), sort_buses.end(), [&](domain::Bus lhs, domain::Bus rhs) { return lhs.id < rhs.id; });

        std::vector<domain::StopPtr> sort_stops(stops_on_map.begin(), stops_on_map.end());
        std::sort(sort_stops.begin(), sort_stops.end(), [&](domain::StopPtr lhs, domain::StopPtr rhs) { return lhs->name < rhs->name; });

        CreateRoutePolylines(sort_buses, sphere_projector, container);
        CreateBusTitles(sort_buses, sphere_projector, container);
        CreateStopPoints(sort_stops, sphere_projector, container);
        CreateStopTitles(sort_stops, sphere_projector, container);
    }
} //namespace renderer