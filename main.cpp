#include <cassert>
#include <chrono>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "json/json_reader.h"
#include "svg/map_renderer.h"

int main() {
    catalogue::TransportCatalogue catalogue;
    catalogue::readers::InputReader reader(catalogue);

    json::Document json_doc = json::Load(std::cin);

    reader.ParseInput(json_doc);
    reader.PrintStatRequests(json_doc, std::cout);

    return 0;
}
