#include "transport_router.h"

using Time = double;

TransportRouter::TransportRouter(TransportCatalogue &city_):city(city_), route_graph(city_.total_stops+city_.GetStopNumber()){
    for(auto& [bus_name, bus_data] : city_.GetAllBus()){
        AddBusToGraph(bus_data->stops_, bus_name);
    }
    FinishGraph();
}

void TransportRouter::AddBusToGraph(std::vector<Stop *>& stops, std::string_view bus_name) {
    int vertexes = vertex_count + stops.size();
    for (int i = 0; i<stops.size();++i) {
        if(!station_and_graphposition.count(stops[i]->name_)){
            station_and_graphposition[stops[i]->name_].push_back(vertexes);
            vertex_name[vertexes] = stops[i]->name_;
            ++vertexes;
        }
        station_and_graphposition[stops[i]->name_].push_back(vertex_count+i);
        vertex_name[vertex_count+i] = stops[i]->name_;
        for(int j = i + 1; j < stops.size(); ++j){
            double buf = 0;
            int start = i;
            int end = i+1;
            while (end <= j){
                buf += city.GetRealLength(stops[start], stops[end]);
                ++start;
                ++end;
            }
            Time between_stops = (buf/1000.0)/city.GetBusVelocity() * 60.0;
            graph::Edge<Time> edge;
            edge.from = vertex_count+i;
            edge.to = vertex_count+j;
            edge.weight = between_stops;
            edge.edge_status = graph::RouteInfo::Ride;
            edge.ride_info = graph::RideInfo{bus_name,(j-i)};
            route_graph.AddEdge(edge);
        }
    }
    vertex_count = vertexes;
}

void TransportRouter::FinishGraph() {
    for(auto& [stop_name, stop_vertexes] : station_and_graphposition){
        for(int i = 0; i<stop_vertexes.size(); ++i){
            for(int j = 0; j < stop_vertexes.size(); ++j){
                if (i == j) continue;
                graph::Edge<Time> edge;
                edge.from = stop_vertexes[i];
                edge.to = stop_vertexes[j];
                edge.weight = j==0? 0 : city.GetWaitTime();
                edge.edge_status = graph::RouteInfo::Wait;
                edge.wait_info.stop = stop_name;
                route_graph.AddEdge(edge);
            }
        }
       // vertex_count+=1;
    }
}

const graph::DirectedWeightedGraph<Time>& TransportRouter::GetGraph() const {
    return route_graph;
}

std::optional<int> TransportRouter::GetStopVertex(const std::string_view stop) const{
    if(station_and_graphposition.count(stop)){
        return station_and_graphposition.at(stop)[0];
    }else{
        return std::nullopt;
    }
}

std::string TransportRouter::GetVertexName(int vertex) const {
    return std::string(vertex_name.at(vertex));
}
