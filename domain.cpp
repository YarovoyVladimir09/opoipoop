#include "domain.h"

using namespace std;

Stop::Stop(std::string name, double x, double y) :name_(name) {
	coordinate.lat = x;
	coordinate.lng = y;
}

Bus::Bus(std::string name, std::vector<Stop*> stop, double length_, double curve_, bool roundtrip_, bool empty_)
	:name_(name), length(length_), curve(curve_), roundtrip(roundtrip_), empty(empty_) {
	stops_.insert(stops_.begin(), make_move_iterator(stop.begin()), make_move_iterator(stop.end()));
}

BusInfo::BusInfo(int stops_, int uniq_stops_, double full_distance_, double curve_) :
	stops(stops_), uniq_stops(uniq_stops_), full_distance(full_distance_), curve(curve_) {

}

size_t StopPairHash::operator()(const std::pair<Stop*, Stop*>& twoStops) const {

	return std::hash<const void*>{}(twoStops.first) + hash<const void*>{}(twoStops.second);
}

size_t StopHash::operator()(const Stop* stop_name) const {

	return std::hash<const void*>{}(stop_name);
}

bool BusCompar::operator()(const Bus* lhs, const Bus* rhs) const {
	return lhs->name_ < rhs->name_;
}