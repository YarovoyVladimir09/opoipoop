#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <variant>
#include <cstdint>
#include <iomanip>

static bool attr = false;

namespace svg {
    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& en);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& en);
    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы

    struct Rgb {
        Rgb(uint8_t r, uint8_t g, uint8_t b) :
            red(r),
            green(g),
            blue(b) {}
        Rgb() = default;
        uint8_t red = 0u;
        uint8_t green = 0u;
        uint8_t blue = 0u;
    };

    struct Rgba {
        Rgba(uint8_t r, uint8_t g, uint8_t b, double o) :
            red(r),
            green(g),
            blue(b),
            opacity(o) {}
        Rgba() = default;
        uint8_t red = 0u;
        uint8_t green = 0u;
        uint8_t blue = 0u;
        double opacity = 1;
    };

    using Color = std::variant<std::monostate, Rgb, Rgba, std::string>;

    std::ostream& operator<<(std::ostream& out, Color color);

    inline const Color NoneColor{ "none" };

    struct ColorPrinter {
        std::ostream& out;
        void operator()(std::monostate) {
            out << NoneColor;
        }
        void operator()(std::string col) {
            out << col;
        }
        void operator()(Rgb col) {
            out << "rgb(" << static_cast<int>(col.red)
                << "," << static_cast<int>(col.green)
                << "," << static_cast<int>(col.blue) << ")";
        }
        void operator()(Rgba col) {
            out << "rgba(" << static_cast<int>(col.red)
                << "," << static_cast<int>(col.green)
                << "," << static_cast<int>(col.blue)
                << ","
                << col.opacity << ")";
        }
    };


    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            linecap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (!std::holds_alternative<std::monostate>(fill_color_)) {
                out << " fill=\""sv << fill_color_ << "\""sv;
                attr = true;
            }
            if (!std::holds_alternative<std::monostate>(stroke_color_)) {
                out << " stroke=\""sv << stroke_color_ << "\""sv;
                attr = true;
            }
            if (width_) {
                out << " stroke-width=\"" << *width_ << "\"";
                attr = true;
            }
            if (linecap_) {
                out << " stroke-linecap=\"" << *linecap_ << "\"";
                attr = true;
            }
            if (linejoin_) {
                out << " stroke-linejoin=\"" << *linejoin_ << "\"";
                attr = true;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        Color fill_color_;
        Color stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> linecap_;
        std::optional<StrokeLineJoin> linejoin_;
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
    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
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
        int indent_step = 0;
        int indent = 0;
    };
    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;
    protected:
        Object() = default;
    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };
    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);
        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);
        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);
        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);
        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);
        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class ObjectContainer {
    public:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::move(std::make_unique<Obj>(std::move(obj))));
        }
        // Добавляет в svg-документ объект-наследник svg::Object
        virtual ~ObjectContainer() = default;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container)const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        Document() = default;
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj)override;
        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objs_;
    };
}