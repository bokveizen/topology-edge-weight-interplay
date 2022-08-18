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
    vector<double> cn(m), sa(m), jc(m), hp(m), hd(m), si(m), li(m), aa(m), ra(m), pa(m), fm(m), dl(m);
    int edge_count = 0;
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
            // name2metric['CN'] = cn_uv
            double cn_i = (double)intersect.size();
            // name2metric['SA'] = cn_uv / math.sqrt(du * dv)
            double sa_i = cn_i / sqrt(dd_u * dd_v);
            // name2metric['JC'] = cn_uv / (du + dv - cn_uv)
            double jc_i = cn_i / (dd_u + dd_v - cn_i);
            // name2metric['HP'] = cn_uv / min(du, dv)
            double hp_i = cn_i / min(dd_u, dd_v);
            // name2metric['HD'] = cn_uv / max(du, dv)
            double hd_i = cn_i / max(dd_u, dd_v);
            // name2metric['SI'] = cn_uv / (du + dv)
            double si_i = cn_i / (dd_u + dd_v);
            // name2metric['LI'] = cn_uv / (du * dv)
            double li_i = cn_i / (dd_u * dd_v);
            // for x in CN_uv:
            //     name2metric['AA'] += 1 / math.log(degrees[x])
            //     name2metric['RA'] += 1 / degrees[x]
            double aa_i = 0., ra_i = 0.;
            for (auto x : intersect) {
                double dd_x = (double)v2Nv[x].size();
                aa_i += 1 / log(dd_x);
                ra_i += 1 / dd_x;
            }
            // name2metric['PA'] = du * dv
            double pa_i = dd_u * dd_v;
            // name2metric['FM'] = cn_uv
            // for x, y in product(Nu - Nv, Nv - Nu):
            //     if y in neighbors_list[x]:
            //         name2metric['FM'] += 1
            double fm_i = 0.;
            for (auto x : N_u) {
                for (auto y : N_v) {
                    if (x == y || v2Nv[x].contains(y)) {
                        fm_i += 1.;
                    }
                }
            }
            // name2metric['DL'] = du + dv - 2
            double dl_i = dd_u + dd_v - 2;
            mut_m.lock();
            // vector<double> cn(m), sa(m), jc(m), hp(m), hd(m), si(m), li(m), aa(m), ra(m), pa(m), fm(m), dl(m);
            cn[i] = cn_i;
            sa[i] = sa_i;
            jc[i] = jc_i;
            hp[i] = hp_i;
            hd[i] = hd_i;
            si[i] = si_i;
            li[i] = li_i;
            aa[i] = aa_i;
            ra[i] = ra_i;
            pa[i] = pa_i;
            fm[i] = fm_i;
            dl[i] = dl_i;
            mut_m.unlock();
        }
    });


    ofstream fout;
    string outfile = "data/metrics_cpp/" + dataset_full + "_cn.txt";
    fout.open(outfile.c_str());
    for (auto &x : cn) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_sa.txt";
    fout.open(outfile.c_str());
    for (auto &x : sa) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_jc.txt";
    fout.open(outfile.c_str());
    for (auto &x : jc) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_hp.txt";
    fout.open(outfile.c_str());
    for (auto &x : hp) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_hd.txt";
    fout.open(outfile.c_str());
    for (auto &x : hd) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_si.txt";
    fout.open(outfile.c_str());
    for (auto &x : si) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_li.txt";
    fout.open(outfile.c_str());
    for (auto &x : li) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_aa.txt";
    fout.open(outfile.c_str());
    for (auto &x : aa) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_ra.txt";
    fout.open(outfile.c_str());
    for (auto &x : ra) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_pa.txt";
    fout.open(outfile.c_str());
    for (auto &x : pa) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_fm.txt";
    fout.open(outfile.c_str());
    for (auto &x : fm) {
        fout << x << endl;
    }
    fout.close();

    outfile = "data/metrics_cpp/" + dataset_full + "_dl.txt";
    fout.open(outfile.c_str());
    for (auto &x : dl) {
        fout << x << endl;
    }
    fout.close();
    return 0;
}