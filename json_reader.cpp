#include "json_reader.h"

using namespace std;

JSONReader::JSONReader(TransportCatalogue& city_):city(city_){
}

void JSONReader::ReadCatalogue(std::istream& input) {
	json::Document catalogue_data = json::Load(input);
	ParseToCatalogue(catalogue_data);
}

void JSONReader::ParseToCatalogue(json::Document& input) {
	RequestHandler handler(city);
	handler.TransportBase(input);
    handler.TransportRouteSettings(input);
	handler.TransportStat(input);
}