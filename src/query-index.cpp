/*
 * query-index.cpp
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
#include <utility>
#include "ring.hpp"
#include "reverse_ring.hpp"
#include <chrono>
#include <triple_pattern.hpp>
#include <ltj_algorithm.hpp>
#include <ltj_algorithm_spo_sop.hpp>
#include <ltj_algorithm_spo_sop_leap.hpp>
#include "utils.hpp"

using namespace std;

//#include<chrono>
//#include<ctime>

using namespace std::chrono;

bool get_file_content(string filename, vector<string> & vector_of_strings)
{
    // Open the File
    ifstream in(filename.c_str());
    // Check if object is valid
    if(!in)
    {
        cerr << "Cannot open the File : " << filename << endl;
        return false;
    }
    string str;
    // Read the next line from File until it reaches the end.
    while (getline(in, str))
    {
        // Line contains string of length > 0 then save it in vector
        if(str.size() > 0)
            vector_of_strings.push_back(str);
    }
    //Close The File
    in.close();
    return true;
}

std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(' ');
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(' ');
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

std::vector<std::string> tokenizer(const std::string &input, const char &delimiter){
    std::stringstream stream(input);
    std::string token;
    std::vector<std::string> res;
    while(getline(stream, token, delimiter)){
        res.emplace_back(trim(token));
    }
    return res;
}

bool is_variable(string & s)
{
    return (s.at(0) == '?');
}

uint8_t get_variable(string &s, std::unordered_map<std::string, uint8_t> &hash_table_vars){
    auto var = s.substr(1);
    auto it = hash_table_vars.find(var);
    if(it == hash_table_vars.end()){
        uint8_t id = hash_table_vars.size();
        hash_table_vars.insert({var, id });
        return id;
    }else{
        return it->second;
    }
}

uint64_t get_constant(string &s){
    return std::stoull(s);
}

ring::triple_pattern get_triple(string & s, std::unordered_map<std::string, uint8_t> &hash_table_vars) {
    vector<string> terms = tokenizer(s, ' ');

    ring::triple_pattern triple;
    if(is_variable(terms[0])){
        triple.var_s(get_variable(terms[0], hash_table_vars));
    }else{
        triple.const_s(get_constant(terms[0]));
    }
    if(is_variable(terms[1])){
        triple.var_p(get_variable(terms[1], hash_table_vars));
    }else{
        triple.const_p(get_constant(terms[1]));
    }
    if(is_variable(terms[2])){
        triple.var_o(get_variable(terms[2], hash_table_vars));
    }else{
        triple.const_o(get_constant(terms[2]));
    }
    return triple;
}

std::string get_type(const std::string &file){
    auto p = file.find_last_of('.');
    return file.substr(p+1);
}

template<class ring_type, class reverse_ring_type, class wm_type = sdsl::bit_vector>
void query(const std::string &file, const std::string &queries, uint64_t number_of_results = 1000, uint64_t timeout_in_millis = 600){
    vector<string> dummy_queries;
    bool result = get_file_content(queries, dummy_queries);

    ring_type graph;
    reverse_ring_type reverse_graph;
    if(ring::util::configuration.is_verbose()){
        cout << " Loading the index SPO..."; fflush(stdout);
    }
    sdsl::load_from_file(graph, file+".spo");
    if(ring::util::configuration.is_verbose()){
        std::cout << endl << " Index loaded " << sdsl::size_in_bytes(graph) << " bytes" << endl;
    }
    if(ring::util::configuration.uses_reverse_index()){
        if(ring::util::configuration.is_verbose()){
            std::cout << " Loading the index SOP..."; fflush(stdout);
        }
        sdsl::load_from_file(reverse_graph, file+".sop");
        if(ring::util::configuration.is_verbose()){
            std::cout << endl << " Index loaded " << sdsl::size_in_bytes(reverse_graph) << " bytes" << endl;
        }
    }
    if(ring::util::configuration.uses_muthu()){
        if(ring::util::configuration.is_verbose()){
            std::cout << " Loading the wavelet matrices that support Muthukrishnan's Colored range counting algorithm." << std::endl;
        }
        graph.load_crc_arrays(file);
    }

    if(ring::util::configuration.is_verbose()){
        std::cout << "Max number of results: " << number_of_results << " Timeout : " << timeout_in_millis << "." << std::endl;
    }
    //graph.store_Ls();
    std::ifstream ifs;
    uint64_t nQ = 0;

    high_resolution_clock::time_point start, stop;
    double total_time = 0.0;
    duration<double> time_span;

    if(result)
    {

        int count = 1;
        for (string& query_string : dummy_queries) {

            //vector<Term*> terms_created;
            //vector<Triple*> query;
            std::unordered_map<std::string, uint8_t> hash_table_vars;
            std::vector<ring::triple_pattern> query;
            vector<string> tokens_query = tokenizer(query_string, '.');
            for (string& token : tokens_query) {
                auto triple_pattern = get_triple(token, hash_table_vars);
                query.push_back(triple_pattern);
            }

            // vector<string> gao = get_gao(query);
            // vector<string> gao = get_gao_min_opt(query, graph);
            // cout << gao [0] << " - " << gao [1] << " - " << gao[2] << endl;

            typedef std::vector<typename ring::ltj_algorithm<>::tuple_type> results_type;
            results_type res;
            start = high_resolution_clock::now();
            if(ring::util::configuration.uses_reverse_index()){
                if(ring::util::configuration.uses_leap()){
                    ring::ltj_algorithm_spo_sop_leap<ring_type,reverse_ring_type, wm_type> ltj(&query, &graph, &reverse_graph);
                    ltj.join(res, number_of_results, timeout_in_millis);

                    stop = high_resolution_clock::now();
                    time_span = duration_cast<microseconds>(stop - start);
                    total_time = time_span.count();

                    std::unordered_map<uint8_t, std::string> ht;
                    for(const auto &p : hash_table_vars){
                        ht.insert({p.second, p.first});
                    }

                    cout << nQ <<  ";" << res.size() << ";" << (unsigned long long)(total_time*1000000000ULL) << ";"<< ltj.get_gao(ht) << endl;
                }else{
                    ring::ltj_algorithm_spo_sop<ring_type,reverse_ring_type, wm_type> ltj(&query, &graph, &reverse_graph);
                    ltj.join(res, number_of_results, timeout_in_millis);

                    stop = high_resolution_clock::now();
                    time_span = duration_cast<microseconds>(stop - start);
                    total_time = time_span.count();

                    std::unordered_map<uint8_t, std::string> ht;
                    for(const auto &p : hash_table_vars){
                        ht.insert({p.second, p.first});
                    }

                    cout << nQ <<  ";" << res.size() << ";" << (unsigned long long)(total_time*1000000000ULL) << ";"<< ltj.get_gao(ht) << endl;
                }
            }
            else{
                ring::ltj_algorithm<ring_type> ltj(&query, &graph);
                ltj.join(res, number_of_results, timeout_in_millis);

                stop = high_resolution_clock::now();
                time_span = duration_cast<microseconds>(stop - start);
                total_time = time_span.count();

                std::unordered_map<uint8_t, std::string> ht;
                for(const auto &p : hash_table_vars){
                    ht.insert({p.second, p.first});
                }

                cout << nQ <<  ";" << res.size() << ";" << (unsigned long long)(total_time*1000000000ULL) << ";"<< ltj.get_gao(ht) << endl;
            }

            
            nQ++;

            // cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << std::endl;

            //cout << "RESULTS QUERY " << count << ": " << number_of_results << endl;
            count += 1;
        }

    }
}

int main(int argc, char* argv[])
{
    //typedef ring::c_ring ring_type;
    if(argc < 3 || argc > 8){
        std::cout << "Usage: " << argv[0] << "<index> <queries> "+ ring::util::configuration.get_configuration_options() << std::endl;
        return 0;
    }

    std::string index = argv[1];
    std::string queries = argv[2];
    std::string type = get_type(index);
    //configuration: execution mode.
    std::string mode = "";
    if(argc >= 3 && argv[3]){
        mode = argv[3];
    }
    //configuration: print gao (yes / no).
    bool print_gao = false;
    if(argc >= 4 && argv[4]){
        std::istringstream(argv[4]) >> print_gao;
    }
    //configuration: verbose (yes / no).
    bool verbose = false;
    if(argc >= 5 && argv[5]){
        std::istringstream(argv[5]) >> verbose;
    }
    uint64_t number_of_results = 1000;
    if(argc >= 6 && argv[6]){
        number_of_results = std::stoull(argv[6]);
    }
    uint64_t timeout = 600;
    if(argc >= 7 && argv[7]){
        timeout = std::stoull(argv[7]);
    }
    ring::util::configuration.configure(mode, print_gao, verbose);
    //print configuration.
    ring::util::configuration.print_configuration();

    //Starting quering the index.
    if(type == "ring"){
        query<ring::ring<>, ring::reverse_ring<>>(index, queries, number_of_results, timeout);
    }else if (type == "c-ring"){
        query<ring::c_ring, ring::c_reverse_ring, sdsl::rrr_vector<15>>(index, queries, number_of_results, timeout);
    }else{
        std::cout << "Type of index: " << type << " is not supported." << std::endl;
    }



	return 0;
}

