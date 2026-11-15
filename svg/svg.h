#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <deque>

namespace svg {
    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
        : x(x)
        , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        uint8_t red{};
        uint8_t green{};
        uint8_t blue{};
    };

    struct Rgba {
        uint8_t red{};
        uint8_t green{};
        uint8_t blue{};
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{ "none" };

    inline std::ostream& operator<<(std::ostream& out, const svg::Color& color) {
        std::visit([&out](const auto& value) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::monostate>) {
                out << NoneColor;
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>) {
                out << value;
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, svg::Rgb>) {
                if (value.red > 255 || value.green > 255 || value.blue > 255) {
                    out << "Ошибка: значения RGB выходят за пределы допустимого диапазона";
                }
                else {
                    out << "rgb(" << static_cast<int>(value.red) << ','
                                  << static_cast<int>(value.green) << ','
                                  << static_cast<int>(value.blue) << ')';
                }
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, svg::Rgba>) {
                if (value.red > 255 || value.green > 255 || value.blue > 255 || value.opacity < 0 || value.opacity > 1) {
                    out << "Ошибка: значения RGBA выходят за пределы допустимого диапазона";
                }
                else {
                    out << "rgba(" << static_cast<int>(value.red) << ','
                                   << static_cast<int>(value.green) << ','
                                   << static_cast<int>(value.blue) << ','
                                   << value.opacity << ')';
                }
            }
        }, color);
        return out;
    }


    inline std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
        switch (line_cap) {
            case StrokeLineCap::BUTT:
                os << "butt";
                break;
            case StrokeLineCap::ROUND:
                os << "round";
                break;
            case StrokeLineCap::SQUARE:
                os << "square";
                break;
        }
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
        switch (line_join) {
            case StrokeLineJoin::ARCS:
                os << "arcs";
                break;
            case StrokeLineJoin::BEVEL:
                os << "bevel";
                break;
            case StrokeLineJoin::MITER:
                os << "miter";
                break;
            case StrokeLineJoin::MITER_CLIP:
                os << "miter-clip";
                break;
            case StrokeLineJoin::ROUND:
                os << "round";
                break;
        }
        return os;
    }

    struct RenderContext {
        RenderContext(std::ostream& out)
        : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 4;
        int indent = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color fill_color) {
            fill_color_ = std::move(fill_color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color stroke_color) {
            stroke_color_ = std::move(stroke_color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_ ;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    protected:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_{ 0.0, 0.0 };
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::deque<Point> polyline_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_{ 0.0, 0.0 };
        Point offset_{ 0.0, 0.0 };
        uint32_t font_size_ = 1;
        std::optional<std::string> font_family_{};
        std::optional<std::string> font_weight_{};
        std::string data_{};
    };

    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;
        template <typename T>
        void Add(T obj) {
            AddPtr(std::make_unique<T>(std::move(obj)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Drawable {
    public:
        Drawable() = default;
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container) const = 0;
    };

    class Document : public ObjectContainer {
    public:
        Document() = default;

        template <typename ObjectType>
        void Add(ObjectType object) {
            AddPtr(std::make_unique<ObjectType>(std::move(object)));
        }

        inline void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }

        void Render(std::ostream& out) const;

    private:
        std::deque<std::unique_ptr<Object>> objects_;
    };
}  // namespace svg
