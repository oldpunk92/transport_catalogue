#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context.out);
        
        context.out << std::endl;
    }

    // ---------- Circle ------------------ //

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto indentedContext = context.Indented();
        indentedContext.RenderIndent();
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------ //

    Polyline& Polyline::AddPoint(Point point) {
        polyline_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto indentedContext = context.Indented();
        indentedContext.RenderIndent();
        auto& out = context.out;
        out << "<polyline points=\""sv;

        bool is_first = true;
        for (const auto& point : polyline_) {
            if (is_first) {
                out << point.x << ","sv << point.y;
                is_first = false;
            }
            else {
                out << " "sv << point.x << ","sv << point.y;
            }
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------ //

    Text& Text::SetPosition(Point pos) {
        pos_ = std::move(pos);
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = std::move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = std::move(size);
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        for (const auto& item : data) {
            switch (item) {
                case '\"':
                    data_ += "&quot;";
                    break;
                case '\'':
                    data_ += "&apos;";
                    break;
                case '<':
                    data_ += "&lt;";
                    break;
                case '>':
                    data_ += "&gt;";
                    break;
                case '&':
                    data_ += "&amp;";
                    break;
                default:
                    data_.push_back(item);
            }
        }
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto indentedContext = context.Indented();
        indentedContext.RenderIndent();
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << font_size_ << "\""sv;

        if (font_family_) {
            out << " font-family=\""sv << *font_family_ << "\""sv;
        }
        
        if (font_weight_) {
            out << " font-weight=\""sv << *font_weight_ << "\""sv;
        }
        out << ">"sv;
        out << data_;
        out << "</text>";
    }

    // ---------- Document ------------------ //

    void Document::Render(std::ostream& out) const {
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)"sv << '\n'
        << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)"sv << '\n';
        for (const auto& obj : objects_) {
            obj->Render(RenderContext{out});
        }
        out << R"(</svg>)"sv;
    }
}  // namespace svg
