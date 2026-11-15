#pragma once

#include "svg.h"
#include "domain/geo.h"
#include "domain/domain.h"

#include <vector>
#include <unordered_set>

namespace renderer {
    struct RenderSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;

        double line_width = 0.0;
        double stop_radius = 0.0;

        int bus_label_font_size = 0;
        svg::Point bus_label_offset;

        int stop_label_font_size = 0;
        svg::Point stop_label_offset{};

        svg::Color underlayer_color{};
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette{};
    };

	class MapRenderer : svg::Drawable {
    public:
        MapRenderer(const std::deque<domain::Bus>& buses, RenderSettings render_settings)
            : render_settings_(render_settings)
            , buses_(buses) {
        }
        virtual ~MapRenderer() = default;

        void Draw(svg::ObjectContainer& container) const override;

    private:
        void CreateRoutePolylines(const std::deque<domain::Bus>& buses, const parameters::SphereProjector& projector, svg::ObjectContainer& container) const;
        void CreateStopPoints(std::vector<domain::StopPtr> stops, parameters::SphereProjector& projector, svg::ObjectContainer& container) const;
        void CreateBusTitles(const std::deque<domain::Bus>& buses, const parameters::SphereProjector& projector, svg::ObjectContainer& container) const;
        void CreateStopTitles(std::vector<domain::StopPtr> stops, parameters::SphereProjector& projector, svg::ObjectContainer& container) const;

    private:
        RenderSettings render_settings_{};
        std::deque<domain::Bus> buses_;
	};
} //namespace renderer
