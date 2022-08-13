
#include<iostream>
#include "configuration.hpp"
template <class bwt_bit_vector_t = sdsl::bit_vector>
class wm_runs{
    typedef uint64_t value_type;
    typedef uint64_t size_type;
    typedef sdsl::bit_vector c_type;
    typedef sdsl::rank_support_v<> c_rank_type;
    typedef sdsl::select_support_mcl<1> c_select_1_type;
    typedef sdsl::select_support_mcl<0> c_select_0_type;
    typedef sdsl::wm_int<bwt_bit_vector_t> bwt_type;
    private:
        bwt_type m_L;
    public:
        wm_runs() = default;
        wm_runs(const std::string &file_path) {
            sdsl::load_from_file(m_L, file_path);
            std::cout << " wavelet matrix '"<<file_path << "' with size "<< m_L.size()<<" loaded : " << sdsl::size_in_bytes(m_L)<< " bytes." <<std::endl;
        }
        uint64_t get_number_of_runs(){
            if(m_L.size() == 0)
                return 0ULL;
            uint64_t number_of_runs = 0;
            size_type prev_value = m_L[0];
            uint64_t run_length = 0;
            for(size_type i = 0 ; i < m_L.size(); i++){
                //std::cout << m_L.tree[i];
                size_type cur_value = m_L[i];
                if(prev_value != cur_value){
                    std::cout << run_length << std::endl;
                    number_of_runs++;
                    run_length = 0;
                }
                prev_value = cur_value;
            }
            return number_of_runs;
        }
};

int main(int argc, char* argv[])
{
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << "wm_file" << std::endl;
        return 0;
    }
    wm_runs<> wm = wm_runs<>(string(argv[1]));
    std::cout << "Number of runs : " << wm.get_number_of_runs() << std::endl;
    return 1;
}