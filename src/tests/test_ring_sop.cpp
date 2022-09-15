
#include<iostream>
#include "reverse_ring.hpp"

typedef ring::reverse_ring<> ring_sop;

int main(int argc, char* argv[])
{
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << "<dataset>.[ring|c-ring]" << std::endl;
        return 0;
    }
    ring_sop graph_sop;
    cout << " Loading the sop index..." << endl;
    sdsl::load_from_file(graph_sop, std::string(argv[1]));
    uint64_t subject = 1;
    auto predicates = graph_sop.get_P_given_S(subject);
    for(auto &predicate: predicates){
        std::cout << "subject_id: " << subject << ", predicate_id: " << predicate << "\n";
    }
    
    uint64_t object = 19650523;
    auto subjects = graph_sop.get_S_given_O(object);
    for(auto &subject: subjects){
        std::cout << "object: " << object << ", subject_id: " << subject << "\n";
    }
    object = 2;
    predicates = graph_sop.get_O_given_P(object);
    for(auto &predicate: predicates){
        std::cout << "predicate: " << predicate << ", object_id: " << object << "\n";
    }
}