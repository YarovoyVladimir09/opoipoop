#pragma once
#include "transport_catalogue.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

class RequestHandler {
public:
    RequestHandler(TransportCatalogue& city_);
    void AddStopFromHandler(const json::Dict& inf);
    void AddDistanceBetweenStop(const json::Dict& inf, const std::string& name,
        std::vector<std::tuple<std::string, std::string, double>>& stops_info);
    void TransportBase(json::Document& input);

    json::Dict BusOut(const json::Dict& input);
    json::Dict StopOut(const json::Dict& input);
    json::Dict RouteOut(const json::Dict& input, const TransportRouter& router, graph::Router<double>& route);
    json::Dict MapOut(const json::Dict& input, std::ostringstream& stream);
    void TransportStat(json::Document& input);
    void TransportRouteSettings(json::Document& input);
    void RenderSettings(json::Document& input, std::ostream& out);

private:
    TransportCatalogue& city;
};
