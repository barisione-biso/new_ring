/*
 * utils.hpp
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


#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <set>
#include "ring.hpp"


namespace ring {

    namespace util {
        typedef uint64_t size_type;//TODO: shouldn't they all get their values from a single source?
        typedef uint8_t var_type;
        /*Classes*/
        class configuration{
            private:
                enum class execution_mode { sigmod21, one_ring_muthu_leap, one_ring_muthu_leap_adaptive, sigmod21_adaptive };
                std::unordered_map<execution_mode, string> mode_enum_to_str;
                std::unordered_map<string, execution_mode> mode_str_to_enum;
                execution_mode m_mode;
                bool m_print_gao;
                bool m_verbose;
                bool m_muthu;
                bool m_adaptive;
                size_type m_threshold;
                execution_mode get_execution_mode(std::string &mode){
                    execution_mode ex_mode = execution_mode::sigmod21;
                    std::unordered_map<std::string,execution_mode>::iterator item;
                    item = mode_str_to_enum.find(mode);
                    if(item != mode_str_to_enum.end()){
                        ex_mode = item->second;
                    }
                    return ex_mode;
                }
                std::string get_mode_label() {
                    std::unordered_map<execution_mode, std::string>::iterator item;
                    item = mode_enum_to_str.find(m_mode);
                    if(item != mode_enum_to_str.end()){
                        return item->second;
                    }
                    return "";
                }
                std::string get_mode_options() const{
                    std::string str="";
                    for(auto& item : mode_enum_to_str){
                        str += item.second + "|";
                    }
                    return str;
                }
                std::string get_default_mode() const{
                    auto item = mode_enum_to_str.find(execution_mode::sigmod21);
                    if(item != mode_enum_to_str.end()){
                        return item->second;
                    }
                    return "";//Handle this better
                }
            public:
                configuration() :
                m_mode (execution_mode::sigmod21),
                m_print_gao(false),
                m_verbose(true),
                m_muthu(false),
                m_adaptive(false),
                m_threshold(1){
                    mode_enum_to_str = {
                                        {execution_mode::sigmod21, "sigmod21"},
                                        {execution_mode::one_ring_muthu_leap, "one_ring_muthu_leap"},
                                        {execution_mode::one_ring_muthu_leap_adaptive, "one_ring_muthu_leap_adaptive"},
                                        {execution_mode::sigmod21_adaptive, "sigmod21_adaptive"}
                                    };
                    mode_str_to_enum = {
                                        {"sigmod21", execution_mode::sigmod21},
                                        {"one_ring_muthu_leap", execution_mode::one_ring_muthu_leap},
                                        {"one_ring_muthu_leap_adaptive", execution_mode::one_ring_muthu_leap_adaptive},
                                        {"sigmod21_adaptive", execution_mode::sigmod21_adaptive},
                                    };
                };
                size_type get_threshold() const{
                    return m_threshold;
                }
                bool is_adaptive() const{
                    return m_adaptive;
                }
                bool uses_muthu() const{
                    return m_muthu;
                }
                bool print_gao() const{
                    return m_print_gao;
                }
                bool is_verbose() const{
                    return m_verbose;
                }
                bool uses_get_size_interval() const{
                    return !m_muthu;
                }
                void print_configuration() {//It cannot be declared as 'const' because 'get_mode_label' function uses 'm_mode' member, which is not const as well.
                    if(m_verbose){
                        std::cout << "Configuration" << std::endl << "=============" << std::endl;
                        std::cout << "Execution Mode: " << get_mode_label() << std::endl;
                        std::cout << "Print gao: " << (m_print_gao ? "true" : "false") << std::endl;
                        std::cout << "Verbose: " << (m_verbose ? "true" : "false") << std::endl;
                        std::cout << "Adaptive gao : " << (m_adaptive ? "true" : "false") << std::endl;
                        std::cout << "Uses Muthu : " << (m_muthu ? "true" : "false") << std::endl;
                    }
                }
                void configure(std::string &mode, bool print_gao, bool verbose){
                    m_mode = get_execution_mode(mode);
                    m_print_gao = print_gao;
                    m_verbose = verbose;
                    if(m_mode == execution_mode::one_ring_muthu_leap || m_mode == execution_mode::one_ring_muthu_leap_adaptive){
                        m_muthu = true;
                    }

                    if(m_mode == execution_mode::one_ring_muthu_leap_adaptive || m_mode == execution_mode::sigmod21_adaptive){
                        m_adaptive = true;
                    }
                }
                std::string get_configuration_options() const{
                    return "[execution_mode="+get_mode_options()+" default="+get_default_mode()+"] [print_gao=0|1 default=0] [verbose=0|1 (default) ] [number_of_results=1000 (default)] [timeout=600 (default)]";
                }
        };
        static configuration configuration;

        /*Functions*/
        template<class Iterator>
        uint64_t get_size_interval(const Iterator &iter) {
            if(iter.cur_s == -1UL && iter.cur_p == -1UL && iter.cur_o == -1UL){
                return iter.i_s.size(); //open
            } else if (iter.cur_s == -1UL && iter.cur_p != -1UL && iter.cur_o == -1UL) {
                return iter.i_s.size(); //i_s = i_o
            } else if (iter.cur_s == -1UL && iter.cur_p == -1UL && iter.cur_o != -1UL) {
                return iter.i_s.size(); //i_s = i_p
            } else if (iter.cur_s != -1UL && iter.cur_p == -1UL && iter.cur_o == -1UL) {
                return iter.i_o.size(); //i_o = i_p
            } else if (iter.cur_s != -1UL && iter.cur_p != -1UL && iter.cur_o == -1UL) {
                return iter.i_o.size();
            } else if (iter.cur_s != -1UL && iter.cur_p == -1UL && iter.cur_o != -1UL) {
                return iter.i_p.size();
            } else if (iter.cur_s == -1UL && iter.cur_p != -1UL && iter.cur_o != -1UL) {
                return iter.i_s.size();
            }
            return 0;
        }
        //TODO: Decouple crc_arrays from the ring, maybe? So ptr_ring won't have the responsibility of doing get_number_distinct_values_spo_BWT_S...
        //TODO: refactor to simplify it. See the other get_num_diff_values.
        template<class ring_type = ring<>,class Iterator>
        std::unordered_map<size_type, size_type> get_num_diff_values(ring_type* ptr_ring, const triple_pattern& triple_pattern, const Iterator &iter) {
            std::unordered_map<size_type, size_type> hash_map;
            size_type num_distinct_values_s = 0;
            size_type num_distinct_values_p = 0;
            size_type num_distinct_values_o = 0;
            //First case: ?S ?P ?O
            if (triple_pattern.s_is_variable() && triple_pattern.p_is_variable() && triple_pattern.o_is_variable())
            {
                bwt_interval open_interval = ptr_ring->open_SPO();
                hash_map.insert({triple_pattern.term_s.value, open_interval.size()});
                hash_map.insert({triple_pattern.term_p.value, open_interval.size()});
                hash_map.insert({triple_pattern.term_o.value, open_interval.size()});
                //std::cout << "num_distinct_values S = " << open_interval.size() << " vs. interval size = " << open_interval.size() << std::endl;
                //std::cout << "num_distinct_values P = " << open_interval.size() << " vs. interval size = " << open_interval.size() << std::endl;
                //std::cout << "num_distinct_values O = " << open_interval.size() << " vs. interval size = " << open_interval.size() << std::endl;
                return hash_map;
            }
            //Second case : ?S P ?O
            else if (triple_pattern.s_is_variable() && !triple_pattern.p_is_variable() && triple_pattern.o_is_variable())
            {
                if(iter.i_s.size() < configuration.get_threshold()){
                    num_distinct_values_s = iter.i_s.size();
                    num_distinct_values_o = iter.i_s.size();
                }else{
                    //Ring => Going from P to S.
                    num_distinct_values_s = ptr_ring->get_number_distinct_values_spo_BWT_S(iter.i_s.left(), iter.i_s.right());

                    // Reverse Ring => Going from P to O.
                    // Important: both ring's C_s and reverse ring's C_o contains range of P's ordered lexicographically, therefore they are equivalents.
                    num_distinct_values_o = ptr_ring->get_number_distinct_values_sop_BWT_O(iter.i_s.left(), iter.i_s.right());
                }

                assert (num_distinct_values_s <= iter.i_s.size());
                assert (num_distinct_values_o <= iter.i_s.size());
                //std::cout << "num_distinct_values S = " << num_distinct_values_s << " vs. interval size = " << iter.i_s.size() << std::endl;
                //std::cout << "num_distinct_values O = " << num_distinct_values_o << " vs. interval size = " << iter.i_s.size() << std::endl;
                hash_map.insert({triple_pattern.term_s.value, num_distinct_values_s});
                hash_map.insert({triple_pattern.term_o.value, num_distinct_values_o});
                return hash_map;
            }
            //Third case ?S ?P O
            else if (triple_pattern.s_is_variable() && triple_pattern.p_is_variable() && !triple_pattern.o_is_variable())
            {
                if(iter.i_p.size() < configuration.get_threshold()){
                    num_distinct_values_p = iter.i_p.size();
                    num_distinct_values_s = iter.i_p.size();
                }else{
                    // Ring => Going from O to P.
                    num_distinct_values_p = ptr_ring->get_number_distinct_values_spo_BWT_P(iter.i_p.left(), iter.i_p.right());
                    // Reverse Ring => Going from O to S.
                    // Important: both ring's C_s and reverse ring's C_o contains range of P's ordered lexicographically, therefore they are equivalents.
                    num_distinct_values_s = ptr_ring->get_number_distinct_values_sop_BWT_S(iter.i_p.left(), iter.i_p.right());
                }
                //std::cout << "num_distinct_values S = " << num_distinct_values_s << " vs. interval size = " << iter.i_p.size() << std::endl;
                //std::cout << "num_distinct_values P = " << num_distinct_values_p << " vs. interval size = " << iter.i_p.size() << std::endl;
                hash_map.insert({triple_pattern.term_p.value, num_distinct_values_p});
                hash_map.insert({triple_pattern.term_s.value, num_distinct_values_s});
                return hash_map;
            }
            //Fourth case S ?P ?O
            else if (!triple_pattern.s_is_variable() && triple_pattern.p_is_variable() && triple_pattern.o_is_variable())
            {
                if(iter.i_o.size() < configuration.get_threshold()){
                    num_distinct_values_o = iter.i_o.size();
                    num_distinct_values_p = iter.i_o.size();
                }else{
                    // Ring => Going from S to O.
                    num_distinct_values_o = ptr_ring->get_number_distinct_values_spo_BWT_O(iter.i_o.left(), iter.i_o.right());
                    // Reverse Ring => Going from S to P.
                    // Important: both ring's C_s and reverse ring's C_o contains range of P's ordered lexicographically, therefore they are equivalents.
                    num_distinct_values_p = ptr_ring->get_number_distinct_values_sop_BWT_P(iter.i_o.left(), iter.i_o.right());
                }
                //std::cout << "num_distinct_values P = " << num_distinct_values_p << " vs. interval size = " << iter.i_o.size() << std::endl;
                //std::cout << "num_distinct_values O = " << num_distinct_values_o << " vs. interval size = " << iter.i_o.size() << std::endl;
                hash_map.insert({triple_pattern.term_o.value, num_distinct_values_o});
                hash_map.insert({triple_pattern.term_p.value, num_distinct_values_p});
                return hash_map;
            }
            //Fifth case S P ?O
            else if (!triple_pattern.s_is_variable() && !triple_pattern.p_is_variable() && triple_pattern.o_is_variable())
            {
                if(iter.i_o.size() < configuration.get_threshold()){
                    num_distinct_values_o = iter.i_o.size();
                }else{
                    // Ring => Going from P to O.
                    num_distinct_values_o = ptr_ring->get_number_distinct_values_spo_BWT_O(iter.i_o.left(), iter.i_o.right());
                }
                //std::cout << "num_distinct_values O = " << num_distinct_values_o << " vs. interval size = " << iter.i_o.size() << std::endl;
                hash_map.insert({triple_pattern.term_o.value, num_distinct_values_o});
                return hash_map;
            }
            //Sixth case S ?P O
            else if (!triple_pattern.s_is_variable() && triple_pattern.p_is_variable() && !triple_pattern.o_is_variable())
            {
                if(iter.i_p.size() < configuration.get_threshold()){
                    num_distinct_values_p = iter.i_p.size();
                }else{
                    // Ring => Going from O to P.
                    num_distinct_values_p = ptr_ring->get_number_distinct_values_spo_BWT_P(iter.i_p.left(), iter.i_p.right());
                }
                //std::cout << "num_distinct_values P = " << num_distinct_values_p << " vs. interval size = " << iter.i_p.size() << std::endl;
                hash_map.insert({triple_pattern.term_p.value, num_distinct_values_p});
                return hash_map;
            }
            //Seventh case ?S P O
            else if (triple_pattern.s_is_variable() && !triple_pattern.p_is_variable() && !triple_pattern.o_is_variable())
            {
                if(iter.i_s.size() < configuration.get_threshold()){
                    num_distinct_values_s = iter.i_s.size();
                }else{
                    // Ring => Going from P to S.
                    num_distinct_values_s = ptr_ring->get_number_distinct_values_spo_BWT_S(iter.i_s.left(), iter.i_s.right());
                }
                //std::cout << "num_distinct_values S = " << num_distinct_values_s << " vs. interval size = " << iter.i_s.size() << std::endl;
                hash_map.insert({triple_pattern.term_s.value, num_distinct_values_s});
                return hash_map;
            }
            return hash_map;
        }
        template<class ring_type = ring<>,class Iterator>
        size_type get_num_diff_values(var_type candidate_var, ring_type* ptr_ring, const Iterator &iter){
            //'candidate_var' is related to 'last_var' in current iter ( check iter.m_ptr_triple_pattern)
            //Important: a range in i_* is equivalent for both SPO and SOP rings.
            //-1ULL = Max ULL value.
            if(iter.is_variable_subject(candidate_var)){
                if(iter.cur_p != -1ULL && iter.cur_o != -1ULL){//p and o are set in the iterator.
                    //P -> S
                    //auto& it = iter.i_s.size() < iter.i_o.size() ? iter.i_s : iter.i_o;
                    //return ptr_ring->get_number_distinct_values_spo_BWT_S(it.left(), it.right());
                    if(iter.i_s.size() < configuration.get_threshold()){
                        return iter.i_s.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_spo_BWT_S(iter.i_s.left(), iter.i_s.right());
                    }
                } else if(iter.cur_p != -1ULL){//only p is currently set.
                    //P -> S: i_s = i_o = m_ptr_ring->down_P(cur_p)
                    if(iter.i_s.size() < configuration.get_threshold()){
                        return iter.i_s.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_spo_BWT_S(iter.i_s.left(), iter.i_s.right());
                    }
                } else if(iter.cur_o != -1ULL){//only o is currently set.
                    //O -> S: i_s = i_p = m_ptr_ring->down_O(cur_o)
                    if(iter.i_s.size() < configuration.get_threshold()){
                        return iter.i_s.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_sop_BWT_S(iter.i_s.left(), iter.i_s.right());
                    }
                } else{
                    //neither p nor o are set.
                    //return iter.i_s.size();
                    return ptr_ring->get_sigma_s();
                }
            }
            if(iter.is_variable_predicate(candidate_var)){
                if(iter.cur_s != -1ULL && iter.cur_o != -1ULL){//s and o are set in the iterator.
                    //O -> P
                    //auto& it = iter.i_p.size() < iter.i_o.size() ? iter.i_p : iter.i_o;
                    //return ptr_ring->get_number_distinct_values_spo_BWT_P(it.left(), it.right());
                    if(iter.i_p.size() < configuration.get_threshold()){
                        return iter.i_p.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_spo_BWT_P(iter.i_p.left(), iter.i_p.right());
                    }
                } else if(iter.cur_s != -1ULL){//only s is currently set.
                    //S -> P: i_p = i_o = m_ptr_ring->down_S(cur_s)
                    if(iter.i_p.size() < configuration.get_threshold()){
                        return iter.i_p.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_sop_BWT_P(iter.i_p.left(), iter.i_p.right());
                    }
                } else if(iter.cur_o != -1ULL){//only o is currently set.
                    //O -> P: i_p = i_o = m_ptr_ring->down_O(cur_s)
                    if(iter.i_p.size() < configuration.get_threshold()){
                        return iter.i_p.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_spo_BWT_P(iter.i_p.left(), iter.i_p.right());
                    }
                } else{
                    //neither s nor o are set.
                    //return iter.i_p.size();
                    return ptr_ring->get_sigma_p();
                }
            }
            if(iter.is_variable_object(candidate_var)){
                if(iter.cur_s != -1ULL && iter.cur_p != -1ULL){//s and p are set in the iterator.
                    //S -> O
                    //auto& it = iter.i_s.size() < iter.i_p.size() ? iter.i_s : iter.i_p;
                    //return ptr_ring->get_number_distinct_values_spo_BWT_O(it.left(), it.right());
                    //Eq to  S P ?O
                    if(iter.i_o.size() < configuration.get_threshold()){
                        return iter.i_o.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_spo_BWT_O(iter.i_o.left(), iter.i_o.right());
                    }
                } else if(iter.cur_s != -1ULL){//only s is currently set.
                    //S -> O: i_p = i_o = m_ptr_ring->down_S(cur_s)
                    if(iter.i_o.size() < configuration.get_threshold()){
                        return iter.i_o.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_spo_BWT_O(iter.i_o.left(), iter.i_o.right());
                    }
                } else if(iter.cur_p != -1ULL){//only p is currently set.
                    //P -> O: i_s = i_o = m_ptr_ring->down_P(cur_s)
                    if(iter.i_o.size() < configuration.get_threshold()){
                        return iter.i_o.size();
                    }else{
                        return ptr_ring->get_number_distinct_values_sop_BWT_O(iter.i_o.left(), iter.i_o.right());
                    }
                } else{
                    //neither s nor p are set.
                    //return iter.i_o.size();
                    return ptr_ring->get_sigma_o();
                }
            }
            return -1ULL;
        }
    }
}



#endif
