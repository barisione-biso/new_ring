/*
 * ltj_algorithm_spo_sop_leap.hpp
 * Copyright (C) 2022 Fabrizio
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



#ifndef RING_LTJ_SPO_SOP_LEAP_ALGORITHM_HPP
#define RING_LTJ_SPO_SOP_LEAP_ALGORITHM_HPP


#include <triple_pattern.hpp>
#include <ring.hpp>
#include <reverse_ring.hpp>
#include <ltj_iterator_manager.hpp>
#include <gao.hpp>

namespace ring {

    template<class ring_t = ring<>,
    class reverse_ring_t = reverse_ring<>,
    class wm_t = sdsl::bit_vector,
    class var_t = uint8_t,
    class cons_t = uint64_t,
    class ltj_iterator_t = ltj_iterator_manager<ring_t,reverse_ring_t, var_t,cons_t, wm_t>>//, class gao = gao_t<>
    class ltj_algorithm_spo_sop_leap {

    public:
        typedef uint64_t value_type;
        typedef uint64_t size_type;
        typedef var_t var_type;
        typedef ring_t ring_type;
        typedef reverse_ring_t reverse_ring_type;
        typedef cons_t const_type;
        //typedef gao_t gao_type;
        typedef ltj_iterator_t ltj_iter_type;
        typedef std::unordered_map<var_type, std::vector<ltj_iter_type*>> var_to_iterators_type;

        typedef std::vector<std::pair<var_type, value_type>> tuple_type;
        typedef std::chrono::high_resolution_clock::time_point time_point_type;

        typedef std::pair<size_type, var_type> pair_type;
        typedef std::priority_queue<pair_type, std::vector<pair_type>, greater<pair_type>> min_heap_type;
        typedef sdsl::wm_int<wm_t> wm_type;
    private:
        const std::vector<triple_pattern>* m_ptr_triple_patterns;
        std::vector<var_type> m_gao; //TODO: should be a class
        std::stack<var_type> m_gao_stack;
        //m_gao_vars is a 1:1 umap representation the m_gao_stack, everything in m_gao_stack is true in this structure.
        std::unordered_map<var_type, bool> m_gao_vars;
        //gao_type m_gao_test;
        ring_type* m_ptr_ring;
        reverse_ring_type* m_ptr_reverse_ring;
        std::vector<ltj_iter_type> m_iterators;

        var_to_iterators_type m_var_to_iterators;
        bool m_is_empty = false;
        gao_size<ring_type, var_type, const_type, ltj_iter_type> m_gao_size;

        //Stack holding WMs intersection - Variable, intersection results, ptr to last selected.
        typedef struct {
            var_type var;
            std::vector<value_type> intersection;
            value_type index_next_val;
        } intersection_type;
        std::stack<intersection_type> m_intersection_cache;

        void copy(const ltj_algorithm_spo_sop_leap &o) {
            m_ptr_triple_patterns = o.m_ptr_triple_patterns;
            m_gao = o.m_gao;
            m_ptr_ring = o.m_ptr_ring;
            m_iterators = o.m_iterators;
            m_var_to_iterators = o.m_var_to_iterators;
            m_is_empty = o.m_is_empty;
        }


        inline void add_var_to_iterator(const var_type var, ltj_iter_type* ptr_iterator){
            auto it =  m_var_to_iterators.find(var);
            if(it != m_var_to_iterators.end()){
                it->second.push_back(ptr_iterator);
            }else{
                std::vector<ltj_iter_type*> vec = {ptr_iterator};
                m_var_to_iterators.insert({var, vec});
            }
        }

        bool is_intersection_calculated(var_type x_j) const{
            if(!m_intersection_cache.empty()){
                const auto& top = m_intersection_cache.top();
                if(top.var == x_j){
                    return true;
                }else{
                    return false;
                }
            }
            return false;
        }

    public:


        ltj_algorithm_spo_sop_leap() = default;

        ltj_algorithm_spo_sop_leap(const std::vector<triple_pattern>* triple_patterns, ring_type* ring, reverse_ring_type* reverse_ring){
            m_ptr_triple_patterns = triple_patterns;
            m_ptr_ring = ring;

            m_ptr_reverse_ring = reverse_ring;
            size_type i = 0;
            m_iterators.resize(m_ptr_triple_patterns->size());
            for(const auto& triple : *m_ptr_triple_patterns){
                //Bulding iterators
                m_iterators[i] = ltj_iter_type(&triple, m_ptr_ring, m_ptr_reverse_ring);
                if(m_iterators[i].is_empty){
                    m_is_empty = true;
                    return;
                }

                //For each variable we add the pointers to its iterators
                if(triple.o_is_variable()){
                    add_var_to_iterator(triple.term_o.value, &(m_iterators[i]));
                }
                if(triple.p_is_variable()){
                    add_var_to_iterator(triple.term_p.value, &(m_iterators[i]));
                }
                if(triple.s_is_variable()){
                    add_var_to_iterator(triple.term_s.value, &(m_iterators[i]));
                }

                ++i;
            }
            m_gao_size = gao_size<ring_type, var_type, const_type, ltj_iter_type>(m_ptr_triple_patterns, &m_iterators, m_ptr_ring, m_gao);
            m_gao_vars.reserve(m_gao_size.m_number_of_variables);
        }

        //! Copy constructor
        ltj_algorithm_spo_sop_leap(const ltj_algorithm_spo_sop_leap &o) {
            copy(o);
        }

        //! Move constructor
        ltj_algorithm_spo_sop_leap(ltj_algorithm_spo_sop_leap &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        ltj_algorithm_spo_sop_leap &operator=(const ltj_algorithm_spo_sop_leap &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        ltj_algorithm_spo_sop_leap &operator=(ltj_algorithm_spo_sop_leap &&o) {
            if (this != &o) {
                m_ptr_triple_patterns = std::move(o.m_ptr_triple_patterns);
                m_gao = std::move(o.m_gao);
                m_ptr_ring = std::move(o.m_ptr_ring);
                m_iterators = std::move(o.m_iterators);
                m_var_to_iterators = std::move(o.m_var_to_iterators);
                m_is_empty = o.m_is_empty;
            }
            return *this;
        }

        void swap(ltj_algorithm_spo_sop_leap &o) {
            std::swap(m_ptr_triple_patterns, o.m_ptr_triple_patterns);
            std::swap(m_gao, o.m_gao);
            std::swap(m_ptr_ring, o.m_ptr_ring);
            std::swap(m_iterators, o.m_iterators);
            std::swap(m_var_to_iterators, o.m_var_to_iterators);
            std::swap(m_is_empty, o.m_is_empty);
        }


        /**
        *
        * @param res               Results
        * @param limit_results     Limit of results
        * @param timeout_seconds   Timeout in seconds
        */
        void join(std::vector<tuple_type> &res,
                  const size_type limit_results = 0, const size_type timeout_seconds = 0){
            if(m_is_empty) return;
            time_point_type start = std::chrono::high_resolution_clock::now();
            tuple_type t(m_gao_size.m_number_of_variables);
            search(0, t, res, start, limit_results, timeout_seconds);
        };

        std::string get_gao(std::unordered_map<uint8_t, std::string>& ht) const{
            std::string str = "";
            for(const auto& var : m_gao){
                str += "?" + ht[var] + " ";
            }
            return str;
        }
        var_type next(const size_type j) {
            if(util::configuration.is_adaptive()){
                var_type var = '\0';
                const var_type& cur_var = m_gao_stack.top();
                const std::unordered_map<var_type, bool> & b_vars = m_gao_vars;
                m_gao_size.update_weights(j, cur_var, b_vars, m_var_to_iterators);
                var = m_gao_size.get_next_var(j, m_gao_vars);
                //std::cout << "next var: " << int(var) << std::endl;
                return var;
            }
            else{
                return m_gao[j];
            }
        }

        void push_var_to_stack(const var_type& x_j){
            //assert (m_gao_stack.top() == x_j);
            m_gao_stack.push(x_j);
            m_gao_vars[x_j]=true;
        }

        void pop_var_of_stack(){
            auto v = m_gao_stack.top();
            m_gao_stack.pop();
            m_gao_vars[v]=false;
        }
        /**
         *
         * @param j                 Index of the variable
         * @param tuple             Tuple of the current search
         * @param res               Results
         * @param start             Initial time to check timeout
         * @param limit_results     Limit of results
         * @param timeout_seconds   Timeout in seconds
         */
        bool search(const size_type j, tuple_type &tuple, std::vector<tuple_type> &res,
                    const time_point_type start,
                    const size_type limit_results = 0, const size_type timeout_seconds = 0){

            //(Optional) Check timeout
            if(timeout_seconds > 0){
                time_point_type stop = std::chrono::high_resolution_clock::now();
                size_type sec = std::chrono::duration_cast<std::chrono::seconds>(stop-start).count();
                if(sec > timeout_seconds) return false;
            }

            //(Optional) Check limit
            if(limit_results > 0 && res.size() == limit_results) return false;

            if(j == m_gao_size.m_number_of_variables){
                //Report results
                res.emplace_back(tuple);
            }else{
                //assert(m_gao_stack.size() == m_gao_vars.size());
                var_type x_j = next(j);
                push_var_to_stack(x_j);
                std::vector<ltj_iter_type*>& itrs = m_var_to_iterators[x_j];
                bool ok;
                if(itrs.size() == 1 && itrs[0]->in_last_level()) {//Lonely variables
                    auto results = itrs[0]->seek_all(x_j);
                    //std::cout << "Results: " << results.size() << std::endl;
                    for (const auto &c : results) {
                        //1. Adding result to tuple
                        tuple[j] = {x_j, c};
                        //std::cout << "current var: " << int(std::get<0>(tuple[j])) << " = " << std::get<1>(tuple[j]) << std::endl;
                        //2. Going down in the trie by setting x_j = c (\mu(t_i) in paper)
                        itrs[0]->down(x_j, c);
                        //2. Search with the next variable x_{j+1}
                        ok = search(j + 1, tuple, res, start, limit_results, timeout_seconds);
                        if(!ok) return false;
                        //4. Going up in the trie by removing x_j = c
                        itrs[0]->up(x_j);
                    }
                }else {
                    //Set the index the algorithm will use before the first seek and only in the first level.
                    for (ltj_iter_type* iter : itrs) {
                        if(iter->get_index_permutation() == ""){
                            iter->set_iter(x_j);
                        }
                    }
                    value_type c = seek(x_j);
                    //std::cout << "Seek (init): (" << (uint64_t) x_j << ": " << c << ")" <<std::endl;
                    while (c != 0) { //If empty c=0
                        //1. Adding result to tuple
                        tuple[j] = {x_j, c};
                        //std::cout << "current var: " << int(std::get<0>(tuple[j])) << " = " << std::get<1>(tuple[j]) << std::endl;
                        //2. Going down in the tries by setting x_j = c (\mu(t_i) in paper)
                        for (ltj_iter_type* iter : itrs) {
                            iter->down(x_j, c);
                        }
                        //3. Search with the next variable x_{j+1}
                        ok = search(j + 1, tuple, res, start, limit_results, timeout_seconds);
                        if(!ok) return false;
                        //4. Going up in the tries by removing x_j = c
                        for (ltj_iter_type *iter : itrs) {
                            iter->up(x_j);
                        }
                        //5. Next constant for x_j
                        c = seek(x_j, c + 1);
                        //std::cout << "Seek (bucle): (" << (uint64_t) x_j << ": " << c << ")" <<std::endl;
                    }
                }
                if(util::configuration.is_adaptive()){
                    m_gao_size.set_previous_weight();
                }
                pop_var_of_stack();
                //std::cout << " pop. " << std::endl;
            }
            return true;
        };

        /**
         *
         * @param x_j   Variable
         * @param c     Constant. If it is unknown the value is -1UL
         * @return      The next constant that matches the intersection between the triples of x_j.
         *              If the intersection is empty, it returns 0.
         */

        value_type seek(const var_type x_j, value_type c=-1UL){
            value_type c_i, c_min = UINT64_MAX, c_max = 0;
            std::vector<ltj_iter_type*>& itrs = m_var_to_iterators[x_j];
            while (true){
                //Compute leap for each triple that contains x_j
                for(ltj_iter_type* iter : itrs){
                    if(c == -1UL){
                        c_i = iter->leap(x_j);
                    }else{
                        c_i = iter->leap(x_j, c);
                    }
                    if(c_i == 0) {
                        return 0; //Empty intersection
                    }
                    if(c_i > c_max) c_max = c_i;
                    if(c_i < c_min) c_min = c_i;
                    c = c_max;
                }
                if(c_min == c_max) return c_min;
                c_min = UINT64_MAX; c_max = 0;
            }
        }        
    };

}

#endif //RING_LTJ_ALGORITHM_HPP
