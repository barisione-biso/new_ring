
/*! \file test_colored_range_counting_wm_2.cpp
    \brief Creating a CRC WM from a Int vector + range search.
    \author Fabrizio Barisione
*/
#include<iostream>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/wavelet_trees.hpp>
#include <sdsl/wt_algorithm.hpp>
#include <unordered_map>
int main()
{
    //1.
    std::cout << ">> Starting test_wm" << std::endl;
    sdsl::int_vector<> C_spo_O = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,0,0,0,0,0,0,0,43,0,0,0,0,0,0,0,0,0,0,59,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,79};
   
    std::cout << "Buiding CRC WM with size = " << C_spo_O.size() << std::endl;
    sdsl::wm_int<sdsl::bit_vector> wm;
    construct_im(wm, C_spo_O);
    //! This function counts distinct values on a range. It's based on Muthukrishnan's Colored range counting algorithm.
    //! See algorithm 2 here: https://users.dcc.uchile.cl/~gnavarro/ps/tcs11.2.pdf Important: 
    
    std::cout << "SPO -> O" << std::endl;
    auto res = wm.range_search_2d(4,9,0,3, false);
    auto count = get<0>(res);
    /*
    std::cout << " # of Distinct values in range [4, 9] : " << count << std::endl;
    count = wm.count_range_search_2d(4,9,0,3);
    std::cout << " # of Distinct values in range [4, 9] (second function): " << count << std::endl;
    res = wm.range_search_2d(9,10,0,8, false);
    count = get<0>(res);
    std::cout << " # of Distinct values in range [9, 10] : " << count << std::endl;
    count = wm.count_range_search_2d(9,10,0,8);
    std::cout << " # of Distinct values in range [9, 10] (second function): " << count << std::endl;
    res = wm.range_search_2d(1,11,0,0, false);//! Important: you must start from 1, not from 0.
    count = get<0>(res);
    std::cout << " # of Distinct values in range [1, 11] : " << count << std::endl;
    count = wm.count_range_search_2d(1,11,0,0);
    std::cout << " # of Distinct values in range [1, 11] (second function): " << count << std::endl;

    res = wm.range_search_2d(20,20,0,19, false);//! Important: you must start from 1, not from 0.
    count = get<0>(res);
    std::cout << " # of Distinct values in range [20, 20] : " << count << std::endl;
    count = wm.count_range_search_2d(20,20,0,19);
    std::cout << " # of Distinct values in range [20, 20] (second function): " << count << std::endl;
    */
    sdsl::int_vector<> C_spo_P = {0,0,2,3,4,5,0,7,8,9,0,11,12,13,14,0,0,17,18,19,20,0,22,23,24,25,0,27,28,29,30,0,32,33,34,35,0,0,38,39,40,41,0,43,44,45,46,0,0,49,50,51,52,0,54,55,56,57,0,0,60,61,62,0,64,63,66,67,68,69,70,65,71,73,74,0,76,77,78,79,80,81,82,83,84,0,85,87,88,72,0,0,92,93,75,0,96,95,0,0,100};
   
    std::cout << "Buiding CRC WM with size = " << C_spo_P.size() << std::endl;
    //sdsl::wm_int<sdsl::bit_vector> wm;
    construct_im(wm, C_spo_P);
    std::cout << "SPO -> P" << std::endl;
    /*res = wm.range_search_2d(4,9,0,3, false);
    count = get<0>(res);
    std::cout << " # of Distinct values in range [4, 9] : " << count << std::endl;
    count = wm.count_range_search_2d(4,9,0,3);
    std::cout << " # of Distinct values in range [4, 9] (second function): " << count << std::endl;
    res = wm.range_search_2d(9,10,0,8, false);
    count = get<0>(res);
    std::cout << " # of Distinct values in range [9, 10] : " << count << std::endl;
    count = wm.count_range_search_2d(9,10,0,8);
    std::cout << " # of Distinct values in range [9, 10] (second function): " << count << std::endl;
    res = wm.range_search_2d(1,11,0,0, false);//! Important: you must start from 1, not from 0.
    count = get<0>(res);
    std::cout << " # of Distinct values in range [1, 11] : " << count << std::endl;
    count = wm.count_range_search_2d(1,11,0,0);
    std::cout << " # of Distinct values in range [1, 11] (second function): " << count << std::endl;
    */
    res = wm.range_search_2d(20,20,0,19, false);//! Important: you must start from 1, not from 0.
    count = get<0>(res);
    std::cout << " # of Distinct values in range [20, 20] : " << count << std::endl;
    count = wm.count_range_search_2d(20,20,0,19);
    std::cout << " # of Distinct values in range [20, 20] (second function): " << count << std::endl;
    /*
    sdsl::int_vector<> C_spo_S = {0,0,0,0,0,0,2,3,4,5,6,11,7,8,9,10,12,13,17,14,15,16,18,20,21,22,19,23,24,25,26,27,32,28,29,30,31,33,34,35,36,37,38,43,39,40,41,42,44,45,46,47,48,49,54,55,56,50,51,53,57,58,62,52,63,61,65,66,64,59,69,67,71,73,74,75,76,60,78,70,79,77,80,83,82,81,85,86,68,89,72,91,92,84,94,87,95,96,88,97,98};
   
    std::cout << "Buiding CRC WM with size = " << C_spo_S.size() << std::endl;
    //sdsl::wm_int<sdsl::bit_vector> wm;
    construct_im(wm, C_spo_S);
    std::cout << "SPO -> S" << std::endl;
    res = wm.range_search_2d(4,9,0,3, false);
    count = get<0>(res);
    std::cout << " # of Distinct values in range [4, 9] : " << count << std::endl;
    count = wm.count_range_search_2d(4,9,0,3);
    std::cout << " # of Distinct values in range [4, 9] (second function): " << count << std::endl;
    res = wm.range_search_2d(9,10,0,8, false);
    count = get<0>(res);
    std::cout << " # of Distinct values in range [9, 10] : " << count << std::endl;
    count = wm.count_range_search_2d(9,10,0,8);
    std::cout << " # of Distinct values in range [9, 10] (second function): " << count << std::endl;
    res = wm.range_search_2d(1,11,0,0, false);//! Important: you must start from 1, not from 0.
    count = get<0>(res);
    std::cout << " # of Distinct values in range [1, 11] : " << count << std::endl;
    count = wm.count_range_search_2d(1,11,0,0);
    std::cout << " # of Distinct values in range [1, 11] (second function): " << count << std::endl;

    
    res = wm.range_search_2d(20,20,0,19, false);//! Important: you must start from 1, not from 0.
    count = get<0>(res);
    std::cout << " # of Distinct values in range [20, 20] : " << count << std::endl;
    count = wm.count_range_search_2d(20,20,0,19);
    std::cout << " # of Distinct values in range [20, 20] (second function): " << count << std::endl;
    */
}