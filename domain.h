#pragma once
#include<string>
#include<vector>

#include "geo.h"

struct Stop {
	Stop(std::string name, double x, double y);
	std::string name_;
	geo::Coordinates coordinate;
};

struct Bus {
	Bus(std::string name, std::vector<Stop*> stop, double length_, double curve_, bool roundtrip_, bool empty_);
	std::string name_;
	std::vector<Stop*> stops_;
	double length;
	double curve;
	bool roundtrip;
	bool empty;
};

struct BusInfo {
	BusInfo(int stops_, int uniq_stops_, double full_distance_, double curve_);
	int stops;
	int uniq_stops;
	double full_distance;
	double curve;
};
class StopPairHash {
public:
	size_t operator()(const std::pair<Stop*, Stop*>& twoStops) const;
};

class StopHash {
public:
	size_t operator()(const Stop* stop_name) const;
};

class BusCompar {
public:
	bool operator()(const Bus* lhs, const Bus* rhs) const;
};