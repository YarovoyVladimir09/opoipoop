#pragma once

#include <unordered_set>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>
#include <map>

#include "geo.h"
#include"domain.h"

class TransportCatalogue {
public:

	void AddStop(std::string stop_name, double latit, double longit);
	void AddBus(std::string bus_name, std::vector<std::string> stop_rout, bool routingtype);
	void AddRealDistance(std::string_view stop1, std::string_view stop2, double dist);

	bool BusCount(std::string_view bus_name) const;
	bool StopCount(std::string_view stop_name) const;

	BusInfo GetBusInfo(std::string_view bus_name);
	bool GetBusEmptyInfo(std::string_view stop_name);
	std::set<std::string_view>& GetStopInfo(std::string_view stop_name);

	double GetLength(Stop* stop1, Stop* stop2);
	double GetRealLength(Stop* stop1, Stop* stop2);
	const std::map<std::string_view, Bus*>& GetAllBus() const;
	const std::unordered_set<Stop*> GetAllStopWithBus() const;
    size_t GetStopNumber() const;

    void SetWaitTime(int);
    void SetBusVelocity(double );
    int GetWaitTime() const;
    double GetBusVelocity() const;
    int total_stops = 0;


private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
	std::deque<Bus> buses_;
	//std::unordered_map<std::string_view, Bus*> busname_to_bus_;
	std::map<std::string_view, Bus*> busname_to_bus_;
	std::unordered_map<std::pair<Stop*, Stop*>, double, StopPairHash> stop_distance_;
	std::unordered_map<std::pair<Stop*, Stop*>, double, StopPairHash> real_stop_distance_;
	std::unordered_map<std::string_view, std::set<std::string_view>> bus_on_stop_;
    double bus_wait_time = 0;
    double bus_velocity = 0;


};