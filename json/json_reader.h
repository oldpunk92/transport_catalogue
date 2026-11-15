#pragma once

#include <sstream>
#include <optional>

#include "json.h"
#include "json_builder.h"
#include "../transport_catalogue.h"
#include "svg/map_renderer.h"
#include "../transport_router.h"

namespace catalogue {
    namespace readers {

        class InputReader {
        public:
            InputReader(catalogue::TransportCatalogue& catalogue) 
                : catalogue_(catalogue) {}

            void ParseInput(const json::Document& doc);
            void PrintStatRequests(const json::Document& doc, std::ostream& out);
            inline const renderer::RenderSettings GetRenderSettings() const { return render_settings_; }

        private:
            void ParseAndAddStop(json::Dict stop_info);
            void ParseAndAddBus(json::Dict bus_info);
            void HandleInputStops(json::Array stops);
            void HandleInputBuses(json::Array stops);
            void HandleInputRouteSetting(json::Dict routing_info);

            renderer::RenderSettings HandleInputRenderSettings(json::Dict render_info);
            
            json::Node HandleStopRequest(json::Dict request);
            json::Node HandleBusRequest(json::Dict request);
            json::Node HandleMapRequest(json::Dict request);
            json::Node HandleRouteRequest(json::Dict request);

        private:
            renderer::RenderSettings render_settings_;
            catalogue::TransportCatalogue& catalogue_;

            catalogue::RouteSettings route_settings_{};
            std::optional<catalogue::TransportRouter> router_;  
        };
    } //namespace readers
} //namespace catalogue



