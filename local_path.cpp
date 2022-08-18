#include <math.h>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <filesystem>
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

using namespace std;
mutex mut_m, mut_n;

/// @param[in] nb_elements : size of your for loop
/// @param[in] functor(start, end) :
/// your function processing a sub chunk of the for loop.
/// "start" is the first index to process (included) until the index "end"
/// (excluded)
/// @code
///     for(int i = start; i < end; ++i)
///         computation(i);
/// @endcode
/// @param use_threads : enable / disable threads.
///
///
static void parallel_for(unsigned nb_elements,
                         function<void(int start, int end)> functor,
                         bool use_threads = true) {
    // -------
    unsigned nb_threads_hint = thread::hardware_concurrency();
    unsigned nb_threads = nb_threads_hint == 0 ? 8 : (nb_threads_hint);

    unsigned batch_size = nb_elements / nb_threads;
    unsigned batch_remainder = nb_elements % nb_threads;

    vector<thread> my_threads(nb_threads);

    if (use_threads) {
        // Multithread execution
        for (unsigned i = 0; i < nb_threads; ++i) {
            int start = i * batch_size;
            my_threads[i] = std::thread(functor, start, start + batch_size);
        }
    } else {
        // Single thread execution (for easy debugging)
        for (unsigned i = 0; i < nb_threads; ++i) {
            int start = i * batch_size;
            functor(start, start + batch_size);
        }
    }

    // Deform the elements left
    int start = nb_threads * batch_size;
    functor(start, start + batch_remainder);

    // Wait for the other thread to finish their task
    if (use_threads)
        std::for_each(my_threads.begin(), my_threads.end(), std::mem_fn(&std::thread::join));
}

int main(int argc, char *argv[]) {
    std::filesystem::create_directories("data/metrics_cpp");
    string dataset(argv[1]);
    string dataset_full;
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
        throw invalid_argument( "unknown dataset");
    }
    string edge_input = "data/edge_txt/" + dataset_full + ".edge_txt";
    ifstream fin;
    vector<set<int>> v2Nv(n);
    vector<int> uu, vv;
    uu.reserve(m);
    vv.reserve(m);
    fin.open(edge_input.c_str());
    int u, v;
    for (auto i = 0; i < m; ++i) {
        fin >> u >> v;
        v2Nv[u].insert(v);
        v2Nv[v].insert(u);
        uu.push_back(u);
        vv.push_back(v);
    }
    fin.close();
    // vector<double> cn(m), sa(m), jc(m), hp(m), hd(m), si(m), li(m), aa(m), ra(m), pa(m), fm(m), dl(m);
    vector<double> lp(m);
    int edge_count = 0;
    // epsilon = 0.001
    // for i, (u, v) in enumerate(tqdm(edges)):
    //     LP_uv = 1. + CN_list[i]
    //     Nu, Nv = neighbors_list[u], neighbors_list[v]
    //     for x, y in product(Nu, Nv):
    //         if x == y:
    //             continue
    //         if y in neighbors_list[x]:
    //             LP_uv += epsilon
    //     LP_list.append(LP_uv)
    double epsilon = 0.001;
    parallel_for(m, [&](int start, int end) {
        for (auto i = start; i < end; ++i) {
            mut_n.lock();
            cout << "\r" << edge_count++ << "/" << m << flush;
            mut_n.unlock();
            auto u_i = uu[i];
            auto v_i = vv[i];
            auto &N_u = v2Nv[u_i];
            auto &N_v = v2Nv[v_i];
            int d_u = N_u.size();
            int d_v = N_v.size();
            double dd_u = (double)d_u;
            double dd_v = (double)d_v;
            vector<int> intersect;
            intersect.reserve(min(d_u, d_v));
            set_intersection(N_u.begin(), N_u.end(), N_v.begin(), N_v.end(),
                             back_inserter(intersect));
            double cn_i = (double)intersect.size();
            double lp_i = 1. + cn_i;
            for (auto x: N_u) {
                for (auto y: N_v) {
                    if (x == y) continue;
                    if (v2Nv[x].contains(y)) lp_i += epsilon;
                }
            }
            mut_m.lock();
            lp[i] = lp_i;
            mut_m.unlock();
        }
    });
    ofstream fout;
    string outfile = "data/metrics_cpp/" + dataset_full + "_lp.txt";
    fout.open(outfile.c_str());
    for (auto &x : lp) {
        fout << x << endl;
    }
    fout.close();
    return 0;
}