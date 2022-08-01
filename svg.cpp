#include "svg.h"

using namespace std;
using namespace std::string_literals;

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, Color color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& en) {
        switch (en)
        {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& en) {
        switch (en)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        }
        return out;
    }
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle";
        out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ----------- Polyline ----------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline";
        out << " points=\""sv;
        for (auto it = points_.begin(); it < points_.end(); ++it) {
            if (it == points_.end() - 1) {
                out << it->x << "," << it->y;
            }
            else {
                out << it->x << "," << it->y << " ";
            }
        }
        out << "\"";
        RenderAttrs(out);
        if (attr) {
            out << "/>";
        }
        else {
            out << " />";
        }
        attr = false;

    }

    // -------------Text-------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }
    void CharOutput(const RenderContext& context, string_view data) {
        auto& out = context.out;
        for (auto word : data) {
            switch (word)
            {
            case '<':
                out << "&lt;";
                break;
            case '>':
                out << "&gt;";
                break;
            case '\"':
                out << "&quot;";
                break;
            case '\'':
                out << "&apos;";
                break;
            case '&':
                out << "&amp;";
                break;
            default:
                out << word;
                break;
            }
        }
    }
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" "
            << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" "
            << "font-size=\"" << font_size_ << "\" ";
        if (!font_family_.empty()) {
            out << "font-family=\"" << font_family_ << "\"";//
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        out << ">";
        CharOutput(context, data_);
        out << "</text>";

    }

    // ---------------------Document---------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objs_.emplace_back((std::move(obj)));
    }


    void Document::Render(std::ostream& out) const {
        auto context = RenderContext(out, 0, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (auto& obj : objs_) {
            obj.get()->Render(context);
        }
        out << "</svg> "sv<<endl;
    }
}  // namespace svg