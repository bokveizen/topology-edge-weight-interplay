// https://stackoverflow.com/questions/67066766/how-to-calculate-edge-betweenness-with-bgl
#include <boost/bimap.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/betweenness_centrality.hpp>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <mutex>
#include <numeric>
#include <set>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

using Mappings = boost::bimap<std::string, int>;

using Graph_type =
    boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS,
                          boost::no_property,
                          boost::property<boost::edge_weight_t, float>>;

using Vertex = typename boost::graph_traits<Graph_type>::vertex_descriptor;
using Edge = std::pair<Vertex, Vertex>;

Graph_type readInGraph(std::string const& fname, Mappings& mappings) {
    std::ifstream myFile(fname);

    // gets first line and turns it to int
    unsigned numEdges = 0;
    myFile >> numEdges;

    std::vector<Edge> edge_array;

    for (std::string src, hyphen, tgt;
         edge_array.size() < numEdges && myFile >> src >> hyphen >> tgt;) {
        // combined lookup/insert:
        auto s = mappings.insert(Mappings::value_type(src, mappings.size()))
                     .first->get_right();
        auto t = mappings.insert(Mappings::value_type(tgt, mappings.size()))
                     .first->get_right();

        // now that all the names are collected we can read in the edge array
        // and store it as the integer mapped to that name in the map
        edge_array.emplace_back(s, t);
    }

    std::vector<float> transmission_delay(edge_array.size(),
                                          1.f);  // no edge weights

    int const numVertices = mappings.size();
    return Graph_type(edge_array.data(), edge_array.data() + edge_array.size(),
                      transmission_delay.data(), numVertices);
}

int main(int argc, char *argv[]) {
    std::filesystem::create_directories("data/metrics_cpp");
    std::string dataset(argv[1]);
    std::string dataset_full;
    int n, m;
    if (dataset == "OF") {
        dataset_full = "OF";
        n = 987, m = 71380;
    } else if (dataset == "FL") {
        dataset_full = "openflights";
        n = 2905, m = 15645;
    } else if (dataset == "th-UB") {
        dataset_full = "threads-ask-ubuntu-proj-graph";
        n = 82075, m = 182648;
    } else if (dataset == "th-MA") {
        dataset_full = "threads-math-sx-proj-graph";
        n = 152702, m = 1088735;
    } else if (dataset == "th-SO") {
        dataset_full = "threads-stack-overflow-proj-graph";
        n = 2301070, m = 20989078;
    } else if (dataset == "sx-UB") {
        dataset_full = "sx-askubuntu";
        n = 152599, m = 453221;
    } else if (dataset == "sx-MA") {
        dataset_full = "sx-mathoverflow";
        n = 24668, m = 187939;
    } else if (dataset == "sx-SO") {
        dataset_full = "sx-stackoverflow";
        n = 2572345, m = 28177464;
    } else if (dataset == "sx-SU") {
        dataset_full = "sx-superuser";
        n = 189191, m = 712870;
    } else if (dataset == "co-DB") {
        dataset_full = "coauth-DBLP-proj-graph";
        n = 1654109, m = 7713116;
    } else if (dataset == "co-GE") {
        dataset_full = "coauth-MAG-Geology-proj-graph";
        n = 898648, m = 4891112;
    } else {
        throw std::invalid_argument( "unknown dataset");
    }
    Mappings mappings;
    std::string edge_input = "data/edge_txt_BGL/" + dataset_full + ".edge_txt_BGL";
    Graph_type g = readInGraph(edge_input, mappings);
    std::ofstream fout;

    using ECMap = std::map<Graph_type::edge_descriptor, double>;
    using ECEntry = ECMap::value_type;
    ECMap ecm;
    brandes_betweenness_centrality(
        g, boost::edge_centrality_map(boost::make_assoc_property_map(ecm)));

    std::vector<std::reference_wrapper<ECEntry>> ranking(ecm.begin(), ecm.end());

    // {
    //     // top-n
    //     auto n = std::min(10ul, ranking.size());
    //     auto first = ranking.begin(), middle = first + n, last = ranking.end();
    //     std::partial_sort(
    //         first, middle, last,
    //         [](ECEntry const& a, ECEntry const& b) { return a.second > b.second; });

    //     ranking.erase(middle, last);
    // }

    auto& edgenames = mappings.right;

    std::string outfile = "data/metrics_cpp/" + dataset_full + "_eb.txt";
    fout.open(outfile.c_str());
    for (ECEntry const& entry : ranking) {
        auto [edge, centrality] = entry;
        auto s = edgenames.at(source(edge, g));
        auto t = edgenames.at(target(edge, g));
        // if (s < t) std::cout << s << "-" << t << " centrality " << centrality << "\n";
        if (s < t) {
//            std::cout << s << " " << t << " " << centrality << "\n";
            fout << s << " " << t << " " << centrality << "\n";
        }
    }
    fout.close();
    return 0;
}
