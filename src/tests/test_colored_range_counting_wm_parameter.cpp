
/*! \file test_colored_range_counting_wm_parameter.cpp
    \brief A more advance test of the CRC WM creation + range search.
    \author Fabrizio Barisione
    Tested with ../../wikidata-filtered-enumerated-red.dat.crc taken from ../../wikidata-filtered-enumerated-red.dat.ring.spo,
    which contains the first 100 records of wikidata-filtered-enumerated-red.dat.
*/
#include<iostream>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/wavelet_trees.hpp>
#include <sdsl/wt_algorithm.hpp>
#include <unordered_map>
#include "crc_arrays.hpp"

int main(int argc, char* argv[])
{
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << "<dataset>.[ring|c-ring].crc" << std::endl;
        return 0;
    }
    //1.
    std::cout << ">> Starting test_wm" << std::endl << " loading CRC array of " << argv[1] << std::endl;
    ring::crc_arrays<> crc_arrays;
    //crc_arrays.load(string(argv[1]));
    sdsl::load_from_file(crc_arrays, string(argv[1]));
    //crc_arrays.print_arrays();
    //2. Quering it.
    //! This function counts distinct values on a range. It's based on Muthukrishnan's Colored range counting algorithm.
    //! See algorithm 2 here: https://users.dcc.uchile.cl/~gnavarro/ps/tcs11.2.pdf

    //For instance, compare it with sorted data found in dat/wikidata-filtered-enumerated-reduced.dat.
    std::cout << "SPO tests " << std::endl;
    auto num_dist_values = crc_arrays.spo_BWT_O->get_number_distinct_values(1, 10);
    std::cout << " SPO -> L_o # of Distinct values in range [1, 10] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.spo_BWT_O->get_number_distinct_values(12, 20);
    std::cout << " SPO -> L_o # of Distinct values in range [12, 20] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.spo_BWT_O->get_number_distinct_values(20, 20);
    std::cout << " SPO -> L_o # of Distinct values in range [20, 20] : " << num_dist_values << std::endl;
    
    num_dist_values = crc_arrays.spo_BWT_P->get_number_distinct_values(1, 10);
    std::cout << " OSP -> L_p # of Distinct values in range [1, 10] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.spo_BWT_P->get_number_distinct_values(12, 20);
    std::cout << " OSP -> L_p # of Distinct values in range [12, 20] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.spo_BWT_P->get_number_distinct_values(20, 20);
    std::cout << " OSP -> L_p # of Distinct values in range [20, 20] : " << num_dist_values << std::endl;

    num_dist_values = crc_arrays.spo_BWT_S->get_number_distinct_values(1, 10);
    std::cout << " POS -> L_s # of Distinct values in range [1, 10] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.spo_BWT_S->get_number_distinct_values(12, 20);
    std::cout << " POS -> L_s # of Distinct values in range [12, 20] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.spo_BWT_S->get_number_distinct_values(20, 20);
    std::cout << " POS -> L_s # of Distinct values in range [20, 20] : " << num_dist_values << std::endl;

    std::cout << "SOP tests " << std::endl;
    num_dist_values = crc_arrays.sop_BWT_P->get_number_distinct_values(1, 10);
    std::cout << " SOP -> L_p # of Distinct values in range [1, 10] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.sop_BWT_P->get_number_distinct_values(12, 20);
    std::cout << " SOP -> L_p # of Distinct values in range [12, 20] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.sop_BWT_P->get_number_distinct_values(20, 20);
    std::cout << " SOP -> L_p # of Distinct values in range [20, 20] : " << num_dist_values << std::endl;
    
    num_dist_values = crc_arrays.sop_BWT_O->get_number_distinct_values(1, 10);
    std::cout << " PSO -> L_o # of Distinct values in range [1, 10] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.sop_BWT_O->get_number_distinct_values(12, 20);
    std::cout << " PSO -> L_o # of Distinct values in range [12, 20] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.sop_BWT_O->get_number_distinct_values(20, 20);
    std::cout << " PSO -> L_o # of Distinct values in range [20, 20] : " << num_dist_values << std::endl;
    
    num_dist_values = crc_arrays.sop_BWT_S->get_number_distinct_values(1, 10);
    std::cout << " OPS -> L_s # of Distinct values in range [1, 10] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.sop_BWT_S->get_number_distinct_values(12, 20);
    std::cout << " OPS -> L_s # of Distinct values in range [12, 20] : " << num_dist_values << std::endl;
    num_dist_values = crc_arrays.sop_BWT_S->get_number_distinct_values(20, 20);
    std::cout << " OPS -> L_s # of Distinct values in range [20, 20] : " << num_dist_values << std::endl;
    
    
}