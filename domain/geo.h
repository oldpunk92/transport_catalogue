#pragma once

#include "svg/svg.h"

#include <cmath>
#include <algorithm>
#include <optional>

namespace parameters {
    struct Coordinates {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);


    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding)
        {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs->coordinates.lng < rhs->coordinates.lng; });
            min_lon_ = (*left_it)->coordinates.lng;
            const double max_lon = (*right_it)->coordinates.lng;

            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs->coordinates.lat < rhs->coordinates.lat; });
            const double min_lat = (*bottom_it)->coordinates.lat;
            max_lat_ = (*top_it)->coordinates.lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
}
