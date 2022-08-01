#include "request_handler.h"

using namespace std;
using namespace json;

string_view Trim(string_view word_to_clear) {
	if (word_to_clear.empty()) {
		return word_to_clear;
	}
	size_t start = word_to_clear.find_first_not_of(" ");
	size_t end = word_to_clear.find_last_not_of(" ");
	return word_to_clear.substr(start, end - start + 1);
}

RequestHandler::RequestHandler(TransportCatalogue& city_) :city(city_) {
}

void RequestHandler::AddStopFromHandler(const json::Dict& inf) {
	city.AddStop(string(Trim(inf.at("name"s).AsString())),
		inf.at("latitude"s).AsDouble(),
		inf.at("longitude"s).AsDouble());
}

void RequestHandler::AddDistanceBetweenStop(const json::Dict& inf, const string& name,
	vector<tuple<string, string, double>>& stops_info) {
	for (const auto& stop : inf) {
		stops_info.push_back({ name,string(Trim(stop.first)),stop.second.AsDouble() });
	}
}

void RequestHandler::TransportBase(json::Document& input) {
	vector<tuple<string, vector<string>, bool>> buses;
	vector<tuple<string, string, double>> stops_info;
	const auto& req_info = input.GetRoot().AsDict();
	for (const auto& base_ : req_info.at("base_requests"s).AsArray()) {
		const auto& trans_inf = base_.AsDict();
		if (trans_inf.at("type"s).AsString() == "Stop"s) {
			AddStopFromHandler(trans_inf);
			if (trans_inf.count("road_distances"s)) {
				AddDistanceBetweenStop(trans_inf.at("road_distances"s).AsDict(),
					string(Trim(trans_inf.at("name"s).AsString())),
					stops_info);
			}
		}
		else if (trans_inf.at("type"s).AsString() == "Bus"s) {
			vector <string> stops;
			for (const auto& stop_ : trans_inf.at("stops"s).AsArray()) {
				stops.push_back(string(Trim(stop_.AsString())));
			}
			buses.push_back({ string(Trim(trans_inf.at("name"s).AsString())),
				move(stops), trans_inf.at("is_roundtrip"s).AsBool() });
		}
	}
	for (auto& [stop1, stop2, dist] : stops_info) {
		city.AddRealDistance(stop1, stop2, dist);
	}
	for (auto& [name, stops, type] : buses) {
		city.AddBus(move(name), move(stops), type);
	}
}


Dict RequestHandler::BusOut(const json::Dict& input) {
	const string& name = string(Trim(input.at("name"s).AsString()));
	if (!city.BusCount(name)) {
		//return Dict{ {"request_id"s,input.at("id"s).AsInt()},{"error_message"s, "not found"s} };
		return json::Builder{}.StartDict().
			Key("request_id"s).Value(input.at("id"s).AsInt()).
			Key("error_message"s).Value("not found"s).
			EndDict().Build().AsDict();
	}
	else if (city.GetBusEmptyInfo(name)) {
		return json::Builder{}.StartDict().EndDict().Build().AsDict();
	}
	else {
		auto [stops, uniq, length, curve] = city.GetBusInfo(name);
		return json::Builder{}.StartDict().Key("curvature"s).Value(curve).
			Key("request_id"s).Value(input.at("id"s).AsInt()).
			Key("route_length"s).Value(length).
			Key("stop_count"s).Value(stops).
			Key("unique_stop_count"s).Value(uniq).
			EndDict().Build().AsDict();
	}
}

Dict RequestHandler::StopOut(const json::Dict& input) {
	const string& name = string(Trim(input.at("name").AsString()));
	if (!city.StopCount(name)) {
		//return Dict{ {"request_id"s,input.at("id").AsInt()},
		//	{"error_message"s, "not found"s} };
		return json::Builder{}.StartDict().
			Key("request_id"s).Value(input.at("id"s).AsInt()).
			Key("error_message"s).Value("not found"s).
			EndDict().Build().AsDict();

	}
	else {
		vector<string> list_of_bus;
		for (auto bus : city.GetStopInfo(name)) {
			list_of_bus.push_back(string(bus));
		}
		//return Dict{ {"buses"s,Array(list_of_bus.begin(),list_of_bus.end())},
		//		{"request_id"s, input.at("id"s).AsInt()} };
		return json::Builder{}.StartDict().
			Key("buses"s).Value(Array(list_of_bus.begin(), list_of_bus.end())).
			Key("request_id"s).Value(input.at("id"s).AsInt()).
			EndDict().Build().AsDict();
	}
}

json::Dict RequestHandler::RouteOut(const Dict &input, const TransportRouter& router, graph::Router<double>& transport_router) {
//   graph::Router<double> transport_router(router.GetGraph());
    const string& from = string(Trim(input.at("from").AsString()));
    const string& to = string(Trim(input.at("to").AsString()));
    auto from_vertex = router.GetStopVertex(from);
    auto to_vertex = router.GetStopVertex(to);
    if(from_vertex == nullopt || to_vertex == nullopt){
        json::Builder{}.StartDict().
                Key("request_id").Value(input.at("id"s).AsInt()).
                Key("error_message").Value("not found").EndDict().
                Build().AsDict();
    }

    auto result = transport_router.BuildRoute(*from_vertex, *to_vertex);
    if(result == nullopt ){
        return json::Builder{}.StartDict().
                Key("request_id").Value(input.at("id"s).AsInt()).
                Key("error_message").Value("not found").EndDict().
                Build().AsDict();
    }
    vector<json::Node> nodes;
    for(auto& edge : result->edges ){
        if(&edge == &result->edges.back()) continue;
        graph::Edge<double> answer = router.GetGraph().GetEdge(edge);
        if (answer.edge_status == graph::RouteInfo::Wait) {
            nodes.emplace_back(Builder{}.StartDict().Key("stop_name").Value(router.GetVertexName(answer.to)).
                    Key("time").Value(answer.weight).Key("type").Value("Wait").EndDict().Build());
        }else{
            nodes.emplace_back(Builder{}.StartDict().Key("bus").Value(string(answer.ride_info.bus)).
                    Key("span_count").Value(answer.ride_info.span_count). Key("time").Value(answer.weight).
                    Key("type").Value("Bus").EndDict().Build());
        }
    }
    return json::Builder{}.StartDict().
            Key("items"s).Value(Array(nodes.begin(), nodes.end())).Key("request_id"s).Value(input.at("id"s).AsInt()).
            Key("total_time").Value(result->weight).EndDict().Build().AsDict();
}

Dict RequestHandler::MapOut(const json::Dict& input, ostringstream& stream) {
	//return Dict{ {"map"s,stream.str()},
	//	{"request_id"s, input.at("id"s).AsInt()} };
	return json::Builder{}.StartDict().
		Key("map"s).Value(stream.str()).
		Key("request_id"s).Value(input.at("id"s).AsInt()).
		EndDict().Build().AsDict();
}

void RequestHandler::TransportStat(json::Document& input) {
    TransportRouter router = TransportRouter(city);
	const auto& req_info = input.GetRoot().AsDict();
    graph::Router<double> transport_router(router.GetGraph());
	Array result;
	//auto context = RenderContext(cout, 2, 0);
	for (const auto& base_ : req_info.at("stat_requests"s).AsArray()) {
		const auto& trans_inf = base_.AsDict();
		if (trans_inf.at("type"s).AsString() == "Stop"s) {
			result.emplace_back(StopOut(trans_inf));
		}
		else if (trans_inf.at("type"s).AsString() == "Bus"s) {
			result.emplace_back(BusOut(trans_inf));
		}
        else if(trans_inf.at("type"s).AsString() == "Route"s){
            result.emplace_back(RouteOut(trans_inf, router, transport_router));
        }
		else if (trans_inf.at("type"s).AsString() == "Map"s) {
			ostringstream svg_res;
			RenderSettings(input,  svg_res);
			result.emplace_back(MapOut(trans_inf, svg_res));
		}
	}
	Print(Document(result), cout);
}

svg::Color ToColor(const json::Node& node) {
	if (node.IsArray()) {
		if (node.AsArray().size() == 3) {
			return svg::Rgb(
				node.AsArray()[0].AsInt(),
				node.AsArray()[1].AsInt(),
				node.AsArray()[2].AsInt());
		}
		else {
			return svg::Rgba(
				node.AsArray()[0].AsInt(),
				node.AsArray()[1].AsInt(),
				node.AsArray()[2].AsInt(),
				node.AsArray()[3].AsDouble());
		}
	}
	else {
		return node.AsString();
	}
}

void RequestHandler::RenderSettings(json::Document& input, ostream& out) {
	const auto& req_info = input.GetRoot().AsDict();
	const auto& render_inf = req_info.at("render_settings"s).AsDict();
	const auto& stops = city.GetAllStopWithBus();
	vector<geo::Coordinates>coordinates;
	for (const auto& stop : stops) {
		coordinates.push_back(stop->coordinate);
	}
	SphereProjector projector(coordinates.begin(), coordinates.end(),
		render_inf.at("width"s).AsDouble(), render_inf.at("height"s).AsDouble(),
		render_inf.at("padding"s).AsDouble());
	Label bus({ render_inf.at("bus_label_font_size"s).AsInt(),
		{
		render_inf.at("bus_label_offset"s).AsArray()[0].AsDouble(),
		render_inf.at("bus_label_offset"s).AsArray()[1].AsDouble()
		} });
	Label stop({ render_inf.at("stop_label_font_size"s).AsInt(),
		{
render_inf.at("stop_label_offset"s).AsArray()[0].AsDouble(),
render_inf.at("stop_label_offset"s).AsArray()[1].AsDouble()
		} });

	svg::Color underlayer = ToColor(render_inf.at("underlayer_color"s));
	vector<svg::Color> colors;
	for (const auto& col : render_inf.at("color_palette"s).AsArray()) {
		colors.push_back(ToColor(col));
	}

	MapRender map_obj(projector, render_inf.at("line_width"s).AsDouble(),
		render_inf.at("stop_radius"s).AsDouble(), bus, stop, underlayer,
		render_inf.at("underlayer_width"s).AsDouble(), colors);

	for (const auto& bus : city.GetAllBus()) {
		map_obj.BusRender(bus.second);
	}

	map_obj.SvgRender(out);

}

void RequestHandler::TransportRouteSettings(Document &input) {
    vector<tuple<string, vector<string>, bool>> buses;
    vector<tuple<string, string, double>> stops_info;
    const auto& req_info = input.GetRoot().AsDict();
    for (const auto& base_ : req_info.at("routing_settings"s).AsDict()) {
        if(base_.first == "bus_wait_time"s){
            city.SetWaitTime(base_.second.AsInt());
        }else if (base_.first == "bus_velocity"s){
            city.SetBusVelocity(base_.second.AsDouble());
        }
    }

}


