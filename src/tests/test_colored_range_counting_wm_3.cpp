
/*! \file test_colored_range_counting_wm_3.cpp
    \brief A more advance test of the CRC WM creation + range search.
    \author Fabrizio Barisione
*/
#include <iostream>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/wavelet_trees.hpp>
#include <sdsl/wt_algorithm.hpp>
#include <unordered_map>
int main()
{
    //1.
    std::cout << ">> Starting test_wm" << std::endl;
    //sdsl::int_vector<> vec = {0,1,1,1,1,1,420,420,420,420,2,2,2,2,2,3,4,4,4,4,4,5,5,5,5,5,6,6,6,6,6,7,7,7,7,7,8,9,9,9,9,9,10,10,10,10,10,11,12,12,12,12,12,13,13,13,13,13,14,524,524,524,524,406,406,524,524,524,524,524,524,406,524,524,524,726,726,726,726,726,726,726,726,726,726,357,726,726,726,406,408,966,966,966,524,487,487,524,546,1097,1097,1097};
    sdsl::int_vector<> vec = {0,1,3,5,7,9,1,3,5,7,9,9,1,3,5,7,9,1,9,3,5,7,1,3,5,7,9,1,3,5,7,9,9,1,3,5,7,9,1,3,5,7,9,9,1,3,5,7,9,1,3,5,7,9,9,9,9,1,3,7,9,1,1,5,1,9,1,9,5,3,5,1,5,5,5,5,5,7,7,3,7,5,3,3,5,7,5,7,9,9,1,1,1,3,3,5,3,5,7,3,5,7};
    sdsl::wm_int<sdsl::bit_vector> L;
    std::cout << "Buiding original WM based  = " << vec << std::endl;
    construct_im(L, vec);

    //2.
    sdsl::int_vector<> C(vec.size());
    std::cout << "Buiding CRC WM with size = " << C.size() << std::endl;
    uint64_t x_s = 0;
    uint64_t x_e = C.size();
    //O ( (x_e - x_s) * log sigma)
    {
        std::unordered_map<uint64_t, uint64_t> hash_map;
        for (uint64_t i = x_s; i < x_e; i++)
        {
            auto it = hash_map.find(L[i]);
            if (it == hash_map.end())
            {
                hash_map.insert({L[i], i});
                C[i - x_s] = 0;
                std::cout << C[i] << " ";
                fflush(stdout);
            }
            else
            {
                C[i - x_s] = it->second + 1;
                std::cout << C[i] << " ";
                fflush(stdout);
                it->second = i;
            }
        }
    }
    std::cout << "" << std::endl;
    //3.
    std::cout << "C = " << C << std::endl;
    //vector<uint64_t> C_aux; // El arreglo "C" del WT.
    sdsl::wm_int<sdsl::bit_vector> wm;
    //sdsl::int_vector<> src;
    construct_im(wm, C);
    std::cout << "  size = " << wm.size() << std::endl;
    std::cout << "  sigma = " << wm.sigma << std::endl;
    std::cout << "  max_level = " << wm.max_level << std::endl; //efectivamente este WT tiene 4 niveles, porque son 11 symbolos, 2⁴ = 16 permite contener los 11 carácteres.
    std::cout << "  rank de 0s = " << wm.rank(wm.size(), 0) << std::endl;
    std::cout << "  rank de 8s = " << wm.rank(wm.size(), 8) << std::endl;
    //! This function counts distinct values on a range. It's based on Muthukrishnan's Colored range counting algorithm.
    //! See algorithm 2 here: https://users.dcc.uchile.cl/~gnavarro/ps/tcs11.2.pdf
    std::cout << "SPO -> P" << std::endl;
    auto res = wm.range_search_2d(3,8,0,3, false);
    auto count = get<0>(res);
    std::cout << " # of Distinct values in range [4, 9] : " << count << std::endl;
    count = wm.count_range_search_2d(3,8,0,3);
    std::cout << " # of Distinct values in range [4, 9] (second function): " << count << std::endl;
    res = wm.range_search_2d(8,9,0,7, false);
    count = get<0>(res);
    std::cout << " # of Distinct values in range [9, 10] : " << count << std::endl;
    count = wm.count_range_search_2d(8,9,0,7);
    std::cout << " # of Distinct values in range [9, 10] (second function): " << count << std::endl;
    res = wm.range_search_2d(00,10,0,0, false);//! Important: what to do with the first position of the CRC? It's a placeholder (val 0)
    count = get<0>(res);
    std::cout << " # of Distinct values in range [1, 11] : " << count << std::endl;
    count = wm.count_range_search_2d(0,10,0,0);
    std::cout << " # of Distinct values in range [1, 11] (second function): " << count << std::endl;
    
    res = wm.range_search_2d(19,19,0,18, false);
    count = get<0>(res);
    std::cout << " # of Distinct values in range [20, 20] : " << count << std::endl;
    count = wm.count_range_search_2d(19,19,0,18);
    std::cout << " # of Distinct values in range [20, 20] (second function): " << count << std::endl;
}