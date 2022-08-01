#include "map_renderer.h"
using namespace std;
using namespace svg;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}
MapRender::MapRender(SphereProjector& proj, double line_w, double rad,
    Label& bus_l, Label& stop_l, svg::Color underlayer, double underlayer_w,
    std::vector<svg::Color>& colors) :
    proj_(proj), line_w_(line_w), stop_rad_(rad), bus_(bus_l),
    stop_(stop_l), underl_(underlayer), underl_w_(underlayer_w), colors_(colors) {}

void MapRender::EditSvgTextBus(const Point& start,const string& text) {
    bus_texts_.push_back(Text().SetPosition(start).SetOffset(bus_.offset).SetFontSize(bus_.font_size)
        .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetData(text)
        .SetFillColor(underl_).SetStrokeColor(underl_).SetStrokeWidth(underl_w_)
        .SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND));
    bus_texts_.push_back(Text().SetPosition(start).SetOffset(bus_.offset).SetFontSize(bus_.font_size)
        .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetData(text)
        .SetFillColor(colors_[color_Num % colors_.size()]));
}

void MapRender::EditSvgTextStop(const Point& start, const string_view text) {
    auto under_text = Text().SetPosition(start).SetOffset(stop_.offset).SetFontSize(stop_.font_size)
        .SetFontFamily("Verdana"s).SetData(string(text))
        .SetFillColor(underl_).SetStrokeColor(underl_).SetStrokeWidth(underl_w_)
        .SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);
    auto upper_text = Text().SetPosition(start).SetOffset(stop_.offset).SetFontSize(stop_.font_size)
        .SetFontFamily("Verdana"s).SetData(string(text))
        .SetFillColor("black");
    auto cir = Circle().SetCenter(start).SetRadius(stop_rad_).SetFillColor("white");
    stop_texts_.insert({ text, {under_text,upper_text} });
    circles_.insert({ text,cir });
}

void MapRender::BusRender(const Bus* bus) {
    if (!bus->stops_.empty()) {

        Point point;
        Polyline drawline;      

        for (const auto& stop : bus->stops_) {
            point = proj_(stop->coordinate);
            drawline.AddPoint(point).SetStrokeWidth(line_w_)
                .SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND).
                SetFillColor("none").SetStrokeColor(colors_[color_Num % colors_.size()]);
            EditSvgTextStop(point, stop->name_);
        }
        polylines_.push_back(drawline);
        Point start = proj_(bus->stops_[0]->coordinate);
        EditSvgTextBus(start, bus->name_);

        if (!bus->roundtrip && bus->stops_[0]!=bus->stops_[bus->stops_.size()/2]) {
            Point end = proj_(bus->stops_[bus->stops_.size()/2]->coordinate);
            EditSvgTextBus(end, bus->name_);
        }
        ++color_Num;
    }
}

void MapRender::SvgRender(ostream& out) {
    Document result;
    for (const auto& poly : polylines_) {
        result.Add((poly));
    }
    for (const auto& bus_name : bus_texts_) {
        result.Add(bus_name);
    }
    for (const auto& cir : circles_) {
        result.Add(cir.second);
    }
    for (const auto& stop_name : stop_texts_) {
        result.Add(stop_name.second.first);
        result.Add(stop_name.second.second);
    }
    result.Render(out);
}