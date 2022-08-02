
#include<iostream>
#include "ring.hpp"

typedef ring::ring<> ring_spo;

int main(int argc, char* argv[])
{
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << "<dataset>.[ring|c-ring]" << std::endl;
        return 0;
    }
    ring_spo graph_spo;
    cout << " Loading the spo index..." << endl;
    sdsl::load_from_file(graph_spo, std::string(argv[1]));
    uint64_t subject = 1;
    auto predicates = graph_spo.get_P_given_S(subject);
    for(auto &predicate: predicates){
        std::cout << "subject_id: " << subject << ", predicate_id: " << predicate << "\n";
    }

    uint64_t object = 19650523;
    auto subjects = graph_spo.get_S_given_O(object);
    for(auto &subject: subjects){
        std::cout << "object: " << object << ", subject_id: " << subject << "\n";
    }
}