/*
 * build-index.cpp
 * Copyright (C) 2020 Author removed for double-blind evaluation
 *
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include "ring.hpp"
#include "reverse_ring.hpp"
#include <fstream>
#include <sdsl/construct.hpp>
#include <ltj_algorithm.hpp>
#include "crc_arrays.hpp"

using namespace std;

using namespace std::chrono;
using timer = std::chrono::high_resolution_clock;

typedef ring::ring<> ring_spo;
typedef ring::reverse_ring<> ring_sop;
typedef ring::c_ring cring_spo;
typedef ring::crc_arrays<> crc_arrays;
template<class ring, class crc_arrays>
void build_index(const std::string &dataset, const std::string &output){
    std::vector<spo_triple> D, E;

    //1. Read the source file.
    std::ifstream ifs(dataset);
    uint64_t s, p , o;
    do {
        ifs >> s >> p >> o;
        D.push_back(spo_triple(s, p, o));
    } while (!ifs.eof());

    D.shrink_to_fit();

    //2. Building the Index SPO - OSP - POS (Cyclic)
    crc_arrays crc_a;
    {
        std::cout << " Building the SPO Index " << std::endl;
        std::cout << "--Indexing " << D.size() << " triples" << std::endl;
        memory_monitor::start();
        auto start = timer::now();

        ring_spo ring_spo(D);
        auto stop = timer::now();
        memory_monitor::stop();
        std::cout << "  Index built  " << sdsl::size_in_bytes(ring_spo) << " bytes" << std::endl;

        std::cout << "Building SPO crc arrays" << std::endl;
        crc_a.build_spo_arrays((ring_spo.get_m_bwt_s()).get_L(), (ring_spo.get_m_bwt_p()).get_L(), (ring_spo.get_m_bwt_o()).get_L() );
        std::cout << " CRC arrays built " << sdsl::size_in_bytes(crc_a) << " bytes" << std::endl;
        sdsl::store_to_file(ring_spo, output);
        std::cout << "Index saved" << endl;
        std::cout << duration_cast<seconds>(stop-start).count() << " seconds." << std::endl;
        std::cout << memory_monitor::peak() << " bytes." << std::endl;
    }

    for (uint64_t i = 0; i < D.size(); i++){
        auto& t = D[i];
        auto s = std::get<0>(t);
        auto p = std::get<1>(t);
        auto o = std::get<2>(t);
        //Due to space efficiency spo is used to build sop ring.
        t = spo_triple(s, o, p);
    }

    //3. Building the reverse Index SOP - PSO - OPS (Cyclic)
    {
        std::cout << " Building the SOP Index " << std::endl;
        std::cout << "--Indexing " << D.size() << " triples" << std::endl;
        memory_monitor::start();
        auto start = timer::now();

        ring_sop ring_sop(D);
        D.clear();
        auto stop = timer::now();
        memory_monitor::stop();
        std::cout << "  Index built  " << sdsl::size_in_bytes(ring_sop) << " bytes" << std::endl;

        std::cout << "Building SOP crc arrays" << std::endl;
        crc_a.build_sop_arrays(ring_sop.get_m_bwt_s().get_L(), ring_sop.get_m_bwt_o().get_L(), ring_sop.get_m_bwt_p().get_L());
        std::cout << " CRC arrays built " << sdsl::size_in_bytes(crc_a) << " bytes" << std::endl;
        //The reverse ring is not persisted.
        //sdsl::store_to_file(ring_sop, output);
        //std::cout << "Index saved" << endl;
        std::cout << duration_cast<seconds>(stop-start).count() << " seconds." << std::endl;
        std::cout << memory_monitor::peak() << " bytes." << std::endl;
    }

    sdsl::store_to_file(crc_a, output + ".crc");
    std::cout << "CRC array saved" << endl;

}

int main(int argc, char **argv)
{

    if(argc != 3){
        std::cout << "Usage: " << argv[0] << "<dataset> [ring|c-ring]" << std::endl;
        return 0;
    }

    std::string dataset = argv[1];
    std::string type    = argv[2];
    if(type == "ring"){
        std::string index_name = dataset + ".ring";
        build_index<ring_spo, crc_arrays>(dataset, index_name);
    }else if (type == "c-ring"){
        std::string index_name = dataset + ".c-ring";
        build_index<cring_spo, crc_arrays>(dataset, index_name);
    }else{
        std::cout << "Usage: " << argv[0] << "<dataset> [ring|c-ring]" << std::endl;
    }

    return 0;
}

