//It only works with Muthu.

#include <iostream>
#include <ring.hpp>
#include <ltj_iterator.hpp>
#include <ltj_algorithm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <triple_pattern.hpp>
#include <utils.hpp>

typedef ring::ring<> ring_type;//Ring SPO
typedef uint8_t var_type;
typedef uint64_t size_type;
typedef ring::ltj_iterator<ring_type,var_type,size_type> ltj_iter_type;
typedef std::unordered_map<var_type, std::vector<ltj_iter_type*>> var_to_iterators_type;
std::vector<ltj_iter_type> m_iterators;
var_to_iterators_type m_var_to_iterators;

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

size_type offline_aggregation (ring_type& graph, const string &query_string){
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
        uint64_t number_of_results = 1000;
        uint64_t timeout_in_millis = 600;
        typedef std::vector<typename ring::ltj_algorithm<>::tuple_type> results_type;
        results_type res;
        ring::ltj_algorithm<ring_type> ltj(&query, &graph);
        ltj.join(res, number_of_results, timeout_in_millis);
}
size_type offline_aggregation_individual_triples_approach(ring_type& graph_spo, const string &query_string){
    //vector<Term*> terms_created;
    //vector<Triple*> query;
    std::unordered_map<std::string, uint8_t> hash_table_vars;
    std::vector<ring::triple_pattern> query;
    vector<string> tokens_query = tokenizer(query_string, '.');
    size_type i = 0;
    bool is_s_var, is_p_var, is_o_var = false;
    var_type var_s, var_p, var_o;
    m_iterators.resize(tokens_query.size());
    for (string& token : tokens_query) {
        auto triple_pattern = get_triple(token, hash_table_vars);
        query.push_back(triple_pattern);
        //Query topology analysis
        if(triple_pattern.s_is_variable()){
            var_s = triple_pattern.term_s.value;
            is_s_var = true;
        }
        if(triple_pattern.p_is_variable()){
            var_p = triple_pattern.term_p.value;
            is_p_var = true;
        }
        if(triple_pattern.o_is_variable()){
            var_o = triple_pattern.term_o.value;
            is_o_var = true;
        }
    }
    for (i = 0 ; i < query.size(); i++){
        //On iterator constructor all constants are processed.
        m_iterators[i] = ltj_iter_type(&query[i], &graph_spo);
    }
    i=0;
    size_type weight = 1;
    for(ring::triple_pattern triple :query){
        const ltj_iter_type &iter = m_iterators[i];
        //3 scenarios: 3 variables, 1 variable and 2 variables. The first and second scenarios are trivial.
        if(is_s_var && is_p_var && is_o_var){
            //In this case it doesn't matter if there are any other triple patterns in the query.
            weight = -1ULL; //TODO: assign n_triples
            break;
        } else if(!is_s_var && !is_p_var && !is_o_var){
            //In this case it doesn't matter if there are any other triple patterns in the query.
            weight = 1;
            break;
        } else{
            size_type triple_weight;
            //2 variables, now we want to restrict using SPO order.
            if(!triple.s_is_variable()){
                auto weight_p = ring::util::get_num_diff_values<ring_type, ltj_iter_type>(triple.term_p.value, &graph_spo, iter);
                auto weight_o = ring::util::get_num_diff_values<ring_type, ltj_iter_type>(triple.term_o.value, &graph_spo, iter);
                triple_weight = weight_p < weight_o ? weight_p : weight_o;
            } else if(!triple.p_is_variable()){
                auto weight_s = ring::util::get_num_diff_values<ring_type, ltj_iter_type>(triple.term_s.value, &graph_spo, iter);
                auto weight_o = ring::util::get_num_diff_values<ring_type, ltj_iter_type>(triple.term_o.value, &graph_spo, iter);
                triple_weight = weight_s < weight_o ? weight_s : weight_o;
            } else if(!triple.o_is_variable()){
                auto weight_s = ring::util::get_num_diff_values<ring_type, ltj_iter_type>(triple.term_s.value, &graph_spo, iter);
                auto weight_p = ring::util::get_num_diff_values<ring_type, ltj_iter_type>(triple.term_p.value, &graph_spo, iter);
                triple_weight = weight_s < weight_p ? weight_s : weight_p;
            }
            std::cout << "  triple # " << i << " weight: " << triple_weight <<std::endl;
            weight *= triple_weight;
        }
        ++i;
    }
    return weight;
}
int main(int argc, char* argv[])
{
    if(argc != 3){
        std::cout << "Usage: " << argv[0] << "<dataset> (with no extension) <queries>" << std::endl;
        return 0;
    }
    ring_type graph_spo;
    std::string file = argv[1];
    cout << " Loading the spo index..." << endl;
    sdsl::load_from_file(graph_spo, std::string(file+".spo"));
    std::cout << " Loading the wavelet matrices that support Muthukrishnan's Colored range counting algorithm." << std::endl;
    graph_spo.load_crc_arrays(file);


    std::string queries = argv[2];
    std::vector<std::string> dummy_queries;
    bool result = get_file_content(queries, dummy_queries);
    size_type weight;
    size_type i=0;

    string mode = "one_ring_muthu_leap";
    ring::util::configuration.configure(mode, false, false);//Conf required to USE muthu.

    for (const string& query_string : dummy_queries) {

            //weight = offline_aggregation_individual_triples_approach(graph_spo, query_string);
            //std::cout << "Query # "<< i << " total weight : " << weight << std::endl;
            offline_aggregation(graph_spo, query_string);
            i++;
    }
}