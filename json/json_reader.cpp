#include "json_reader.h"


namespace catalogue {
    namespace readers {
        using namespace std::literals;

        //----------HUNDLE INPUT----------//

        void InputReader::ParseAndAddStop(json::Dict stop_info) {
            std::string cur_stop_name = stop_info["name"s].AsString();
            double lat = stop_info["latitude"s].AsDouble();
            double lng = stop_info["longitude"s].AsDouble();

            catalogue_.AddStop(cur_stop_name, { lat, lng });

            if (stop_info.find("road_distances"s) != stop_info.end() && !stop_info["road_distances"s].AsDict().empty()) {
                for (const auto& [stop_name, distance] : stop_info["road_distances"s].AsDict()) {
                    catalogue_.SetDistance(cur_stop_name, stop_name, distance.AsInt());
                }
            }
        }

        void InputReader::ParseAndAddBus(json::Dict bus_info) {
            std::vector<std::string_view> route;

            std::string bus_id = bus_info["name"s].AsString();
            bool is_roundtrip = bus_info["is_roundtrip"].AsBool();

            for (const auto& item : bus_info["stops"s].AsArray()) {
                route.push_back(item.AsString());
            }

            catalogue_.AddBus(bus_id, is_roundtrip, std::move(route));
        }

        void InputReader::HandleInputStops(json::Array stops) {
            for (const json::Node& request : stops) {
                if (request.IsDict()) {
                    json::Dict request_map = request.AsDict();

                    if (request_map.find("type"s) != request_map.end()) {
                        std::string type = request_map["type"s].AsString();

                        if (type == "Stop"s) ParseAndAddStop(request_map);
                        else if (type == "Bus"s) continue;
                        else if (type == "Map"s) continue;
                        else throw json::ParsingError("Unknow data type"s + type);
                    }
                }
            }
        }

        void InputReader::HandleInputBuses(json::Array stops) {
            for (const json::Node& request : stops) {
                if (request.IsDict()) {
                    json::Dict request_map = request.AsDict();

                    if (request_map.find("type"s) != request_map.end()) {
                        std::string type = request_map["type"s].AsString();

                        if (type == "Bus"s) ParseAndAddBus(request_map);
                        else if (type == "Stop"s) continue;
                        else if (type == "Map"s) continue;
                        else throw json::ParsingError("Unknow data type"s + type);
                    }
                }
            }
        }

        void InputReader::HandleInputRouteSetting(json::Dict routing_info) {
            int bus_wait_time = routing_info["bus_wait_time"].AsInt();
            double bus_velocity = routing_info["bus_velocity"].AsDouble();

            route_settings_ = { bus_wait_time, bus_velocity };
        }

        svg::Color ConvertNodeToColor(json::Node node) {
            svg::Color color{};
            if (node.IsArray()) {
                json::Array color_arr = node.AsArray();
                if (color_arr.size() == 3) {
                    svg::Rgb rgb_color{};
                    rgb_color.red = (uint8_t)color_arr[0].AsDouble();
                    rgb_color.green = (uint8_t)color_arr[1].AsDouble();
                    rgb_color.blue = (uint8_t)color_arr[2].AsDouble();
                    color = rgb_color;
                }
                else if (color_arr.size() == 4) {
                    svg::Rgba rgba_color{};
                    rgba_color.red = (uint8_t)color_arr[0].AsDouble();
                    rgba_color.green = (uint8_t)color_arr[1].AsDouble();
                    rgba_color.blue = (uint8_t)color_arr[2].AsDouble();
                    rgba_color.opacity = color_arr[3].AsDouble();
                    color = rgba_color;
                }
            }
            else if (node.IsString()) {
                color = node.AsString();
            }
            else {
                throw json::ParsingError("Unknow data for color"s);
            }
            return color;
        }

        svg::Point ConvertNodeToPoint(json::Node node) {
            svg::Point point{};
            if (node.IsArray()) {
                point.x = node.AsArray()[0].AsDouble();
                point.y = node.AsArray()[1].AsDouble();
            }
            else {
                throw json::ParsingError("Unknow data for point"s);
            }
            return point;
        }

        renderer::RenderSettings InputReader::HandleInputRenderSettings(json::Dict render_info) {
            renderer::RenderSettings render_settings{};
            if (!render_info.empty()) {
                render_settings.width = render_info["width"].AsDouble();
                render_settings.height = render_info["height"].AsDouble();
                render_settings.padding = render_info["padding"].AsDouble();

                render_settings.line_width = render_info["line_width"].AsDouble();
                render_settings.stop_radius = render_info["stop_radius"].AsDouble();

                render_settings.bus_label_font_size = render_info["bus_label_font_size"].AsInt();
                render_settings.bus_label_offset = ConvertNodeToPoint(render_info["bus_label_offset"]);


                render_settings.stop_label_font_size = render_info["stop_label_font_size"].AsInt();
                render_settings.stop_label_offset = ConvertNodeToPoint(render_info["stop_label_offset"]);

                render_settings.underlayer_color = ConvertNodeToColor(render_info["underlayer_color"]);
                render_settings.underlayer_width = render_info["underlayer_width"].AsDouble();

                if (render_info["color_palette"].IsArray()) {
                    for (const auto& color : render_info["color_palette"].AsArray()) {
                        render_settings.color_palette.push_back(ConvertNodeToColor(color));
                    }
                }
            }
            return render_settings;
        }

        void InputReader::ParseInput(const json::Document& doc) {
            json::Node base_requests = doc.GetRoot()["base_requests"s];
            if (base_requests.IsArray()) {
                HandleInputStops(base_requests.AsArray());
                HandleInputBuses(base_requests.AsArray());
            }

            json::Node render_settings = doc.GetRoot()["render_settings"s];
            if (render_settings.IsDict()) {
                render_settings_ = HandleInputRenderSettings(render_settings.AsDict());
            }

            json::Node routing_settings = doc.GetRoot()["routing_settings"];
            if (routing_settings.IsDict()) {
                HandleInputRouteSetting(routing_settings.AsDict());
            }
        }

        //----------HUNDLE REQUEST----------//

        json::Node InputReader::HandleBusRequest(json::Dict request) {
            std::string bus_name = request.at("name"s).AsString();
            int request_id = request.at("id"s).AsInt();
            json::Builder builder{};

            auto bus = catalogue_.GetRouteInfo(bus_name);
            if (!bus.NotFound()) {
                builder
                    .StartDict()
                    .Key("curvature"s).Value(bus.curvature)
                    .Key("request_id"s).Value(request_id)
                    .Key("route_length"s).Value(bus.real_distance)
                    .Key("stop_count"s).Value(bus.count_stops)
                    .Key("unique_stop_count"s).Value(bus.count_unique_stops)
                    .EndDict();
            }
            else {
                builder
                    .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict();
            }
            return builder.Build();
        }

        json::Node InputReader::HandleStopRequest(json::Dict request) {
            std::string stop_name = request.at("name"s).AsString();
            int request_id = request.at("id"s).AsInt();
            json::Builder builder{};

            builder.StartDict()
                .Key("request_id"s).Value(request_id);

            if (catalogue_.GetStop(stop_name) == nullptr) {
                builder.Key("error_message"s).Value("not found"s);
            }
            else {
                auto source_buses = catalogue_.GetStopInfo(stop_name);
                std::vector<const Bus*> buses(source_buses.begin(), source_buses.end());

                std::sort(buses.begin(), buses.end(), [](const Bus* lhs, const Bus* rhs) {
                    return lhs->id < rhs->id;
                    });

                builder.Key("buses"s)
                    .StartArray();
                for (const auto* bus : buses) {
                    builder.Value(bus->id);
                }
                builder.EndArray();
            }

            return builder.EndDict().Build();
        }

        json::Node InputReader::HandleMapRequest(json::Dict request) {
            int request_id = request["id"s].AsInt();
            std::ostringstream string_out;
            json::Builder builder{};

            svg::Document svg_doc;
            renderer::MapRenderer map_renderer(catalogue_.GetBusList(), render_settings_);
            map_renderer.Draw(svg_doc);
            svg_doc.Render(string_out);

            return builder.StartDict()
                .Key("map"s).Value(string_out.str())
                .Key("request_id"s).Value(request_id)
                .EndDict()
                .Build();
        }

        json::Node InputReader::HandleRouteRequest(json::Dict request) {
            int request_id = request["id"s].AsInt();
            std::string from_stop_name= request["from"s].AsString();
            std::string to_stop_name = request["to"s].AsString();

            if (!router_) router_.emplace(catalogue_, route_settings_);

            auto route_info = router_.value().GetRouteInfo(from_stop_name, to_stop_name);

            json::Builder builder{};

            builder.StartDict()
                .Key("request_id"s).Value(request_id);

            if (!route_info) {
                builder.Key("error_message"s).Value("not found"s);
            }
            else {
                double total_travel_time{};
                builder
                    .Key("items"s).StartArray();
                for (const auto& edge : route_info.value()) {
                    if (edge.is_wait) {
                        double wait_time = edge.weight;
                        total_travel_time += wait_time;
                        builder
                            .StartDict()
                            .Key("type"s).Value("Wait"s)
                            .Key("stop_name"s).Value(std::string(edge.object_route_name))
                            .Key("time"s).Value(wait_time)
                            .EndDict();
                    }
                    else {
                        double road_time = edge.weight;
                        total_travel_time += road_time;
                        builder
                            .StartDict()
                            .Key("type"s).Value("Bus"s)
                            .Key("bus"s).Value(std::string(edge.object_route_name))
                            .Key("span_count"s).Value(edge.span_count.value())
                            .Key("time"s).Value(road_time)
                            .EndDict();
                    }
                }
                builder
                    .EndArray()
                    .Key("total_time"s).Value(total_travel_time);
            }
            return builder.EndDict().Build();
        }

        void InputReader::PrintStatRequests(const json::Document& doc, std::ostream& out) {
            const auto& root_dict = doc.GetRoot().AsDict();
            if (root_dict.find("stat_requests"s) == root_dict.end()) return;

            const json::Node& stat_requests = root_dict.at("stat_requests"s);
            json::Builder builder{};

            if (stat_requests.IsArray()) {
                builder.StartArray();

                for (const json::Node& request : stat_requests.AsArray()) {
                    if (request.IsDict()) {
                        const json::Dict& request_map = request.AsDict();
                        const std::string& type = request_map.at("type"s).AsString();

                        if (type == "Bus"s) {
                            builder.Value(HandleBusRequest(request_map).GetValue()).EndDict();
                        }
                        else if (type == "Stop"s) {
                            builder.Value(HandleStopRequest(request_map).GetValue()).EndDict();
                        }
                        else if (type == "Map"s) {
                            builder.Value(HandleMapRequest(request_map).GetValue()).EndDict();
                        }
                        else if (type == "Route") {
                            builder.Value(HandleRouteRequest(request_map).GetValue()).EndDict();
                        }
                        else {
                            throw json::ParsingError("Unknown data type: "s + type);
                        }
                    }
                }
                builder.EndArray();
            }
            json::Print(json::Document(builder.Build()), out);
        }
    } //namespace readers
} //namespace catalogue