#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

//static std::string WAIT = " wait";

class TransportRouter{
    using Time = double;
public:
    TransportRouter(TransportCatalogue& city_);
    std::optional<int> GetStopVertex(const std::string_view) const;
    std::string GetVertexName(int) const;
   const graph::DirectedWeightedGraph<Time>& GetGraph() const;
private:
    TransportCatalogue& city;
    void AddBusToGraph(std::vector<Stop*>&, std::string_view);
    void FinishGraph();
    std::unordered_map<std::string_view,std::vector<int>> station_and_graphposition;
    std::unordered_map<int, std::string_view> vertex_name;
    graph::DirectedWeightedGraph<Time> route_graph;
    size_t vertex_count = 0;
};