#include "transport_catalogue.h"


using namespace std;

void TransportCatalogue::AddStop(std::string stop_name, double latit, double longit) {
	auto it = &stops_.emplace_front(Stop(move(stop_name), latit, longit));
	if (stopname_to_stop_.count(it->name_) == 0) {
		stopname_to_stop_[it->name_] = it;
	}
	bus_on_stop_[it->name_];
}

void TransportCatalogue::AddBus(string bus_name, vector<string> stop_rout, bool routingtype) {
	if (!routingtype) {
		vector<string> buff(stop_rout.rbegin() + 1, stop_rout.rend());
		stop_rout.insert(stop_rout.end(), make_move_iterator(buff.begin()), make_move_iterator(buff.end()));
	}
	if (routingtype) {
		if (stop_rout[0] != stop_rout.back()) {
			stop_rout.push_back(stop_rout[0]);
		}
	}
	vector<Stop*> stops_on_rout;
	double length = 0;
	double real_length = 0;
	bool empty = false;
	for (auto stop = stop_rout.begin(); stop != stop_rout.end(); ++stop) {
		if (stopname_to_stop_.count(*stop) == 0 ||
        (stopname_to_stop_.at(*stop)->coordinate.lat==0.0&& stopname_to_stop_.at(*stop)->coordinate.lng == 0.0)) {
			empty = true;
		}
		stops_on_rout.push_back(stopname_to_stop_.at(*stop));
		if (stop + 1 != stop_rout.end()) {
			stop_distance_[{stopname_to_stop_.at(*stop), stopname_to_stop_.at(*(stop + 1))}] =
				ComputeDistance(stopname_to_stop_.at(*stop)->coordinate,
					stopname_to_stop_.at(*(stop + 1))->coordinate);
			length += GetLength(stopname_to_stop_[*stop], stopname_to_stop_[*(stop + 1)]);
			real_length += GetRealLength(stopname_to_stop_[*stop], stopname_to_stop_[*(stop + 1)]);
		}
	}
	auto it = &buses_.emplace_front(Bus(move(bus_name), stops_on_rout,
                                        real_length, real_length / length,
                                        routingtype, empty));
	busname_to_bus_[it->name_] = it;
	for (auto& stopp : it->stops_) {
		bus_on_stop_[stopp->name_].insert(it->name_);
	}
    total_stops+=stops_on_rout.size();
}

void TransportCatalogue::AddRealDistance(string_view stop1, string_view stop2, double dist) {
	if (stopname_to_stop_.count(stop2) == 0) {
		AddStop(string(stop2),0,0);
	}
	real_stop_distance_[{stopname_to_stop_[stop1], stopname_to_stop_[(stop2)]}] = dist;
}

bool TransportCatalogue::BusCount(string_view bus_name) const {
	return busname_to_bus_.count(bus_name);
}

BusInfo TransportCatalogue::GetBusInfo(string_view bus_name) {

	int size_of_route = busname_to_bus_.at(bus_name)->stops_.size();
	unordered_set<Stop*, StopHash> stops(busname_to_bus_.at(bus_name)->stops_.begin(), busname_to_bus_.at(bus_name)->stops_.end());
	int size_of_unique_stops = stops.size();


	return { BusInfo(size_of_route,size_of_unique_stops,busname_to_bus_.at(bus_name)->length,
		busname_to_bus_.at(bus_name)->curve) };
}

bool TransportCatalogue::GetBusEmptyInfo(string_view bus_name) {

	return busname_to_bus_.at(bus_name)->empty;
}

bool TransportCatalogue::StopCount(string_view stop_name) const {
	return stopname_to_stop_.count(stop_name);
}

std::set<std::string_view>& TransportCatalogue::GetStopInfo(std::string_view stop_name) {

	return bus_on_stop_.at(stop_name);
}


double TransportCatalogue::GetLength(Stop* stop1, Stop* stop2) {
	return stop_distance_[{stop1, stop2}];
}
double TransportCatalogue::GetRealLength(Stop* stop1, Stop* stop2) {
	if (real_stop_distance_.find({ stop1, stop2 }) != real_stop_distance_.end()) {
		return real_stop_distance_[{stop1, stop2}];
	}
	else {
		return real_stop_distance_[{stop2, stop1}];
	}

}

const std::map<std::string_view, Bus*>& TransportCatalogue::GetAllBus() const{
	return busname_to_bus_;
}

const std::unordered_set<Stop*> TransportCatalogue::GetAllStopWithBus() const {
	unordered_set<Stop*> result;
	for (const auto& stop : bus_on_stop_) {
		if (!stop.second.empty()) {
			result.insert(stopname_to_stop_.at(stop.first));
		}
	}
	return result;
}

void TransportCatalogue::SetWaitTime(int time) {
    bus_wait_time = time;
}

void TransportCatalogue::SetBusVelocity(double velocity) {
    bus_velocity = velocity;
}

int TransportCatalogue::GetWaitTime() const {
    return bus_wait_time;
}

double TransportCatalogue::GetBusVelocity() const {
    return bus_velocity;
}

size_t TransportCatalogue::GetStopNumber() const{
    return stopname_to_stop_.size();
}
