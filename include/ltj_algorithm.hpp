/*
 * ltj_algorithm.hpp
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



#ifndef RING_LTJ_ALGORITHM_HPP
#define RING_LTJ_ALGORITHM_HPP


#include <triple_pattern.hpp>
#include <ring.hpp>
#include <ltj_iterator.hpp>
#include <gao.hpp>

namespace ring {

    template<class ring_t = ring<>, class var_t = uint8_t, class cons_t = uint64_t>//, class gao = gao_t<>
    class ltj_algorithm {

    public:
        typedef uint64_t value_type;
        typedef uint64_t size_type;
        typedef var_t var_type;
        typedef ring_t ring_type;
        typedef cons_t const_type;
        //typedef gao_t gao_type;
        typedef ltj_iterator<ring_type, var_type, const_type> ltj_iter_type;
        typedef std::unordered_map<var_type, std::vector<ltj_iter_type*>> var_to_iterators_type;
        typedef std::vector<std::pair<var_type, value_type>> tuple_type;
        typedef std::chrono::high_resolution_clock::time_point time_point_type;

        typedef std::pair<size_type, var_type> pair_type;
        typedef std::priority_queue<pair_type, std::vector<pair_type>, greater<pair_type>> min_heap_type;
    private:
        const std::vector<triple_pattern>* m_ptr_triple_patterns;
        std::vector<var_type> m_gao; //TODO: should be a class
        std::stack<var_type> m_gao_stack;
        //m_gao_vars is a 1:1 umap representation the m_gao_stack, everything in m_gao_stack is true in this structure.
        std::unordered_map<var_type, bool> m_gao_vars;
        //gao_type m_gao_test;
        ring_type* m_ptr_ring;
        std::vector<ltj_iter_type> m_iterators;
        var_to_iterators_type m_var_to_iterators;
        bool m_is_empty = false;
        gao_size<ring_type> m_gao_size;
        void copy(const ltj_algorithm &o) {
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

    public:


        ltj_algorithm() = default;

        ltj_algorithm(const std::vector<triple_pattern>* triple_patterns, ring_type* ring){

            m_ptr_triple_patterns = triple_patterns;
            m_ptr_ring = ring;

            size_type i = 0;
            m_iterators.resize(m_ptr_triple_patterns->size());
            for(const auto& triple : *m_ptr_triple_patterns){
                //Bulding iterators
                m_iterators[i] = ltj_iter_type(&triple, m_ptr_ring);
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
            m_gao_size = gao_size<ring_type>(m_ptr_triple_patterns, &m_iterators, m_ptr_ring, m_gao);

            m_gao_vars.reserve(m_gao_size.m_number_of_variables);
        }

        //! Copy constructor
        ltj_algorithm(const ltj_algorithm &o) {
            copy(o);
        }

        //! Move constructor
        ltj_algorithm(ltj_algorithm &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        ltj_algorithm &operator=(const ltj_algorithm &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        ltj_algorithm &operator=(ltj_algorithm &&o) {
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

        void swap(ltj_algorithm &o) {
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
        var_type next(const size_type j) const{
            if(util::configuration.is_adaptive()){
                if(j == 0){
                    push_var_to_stack(m_gao_size.m_var_info[0].name);
                    return m_gao_size.m_var_info[0].name;
                } else {
                    const var_type& cur_var = m_gao_stack.top();
                    const std::unordered_map<var_type, bool> & b_vars = m_gao_vars;
                    bool rel_var_processed = false;
                    {
                        //min_heap_type heap;
                        const auto& rel_vars = m_gao_size.get_related_variables(cur_var);
                        //(1). Linked / Related variables (Lonely vars are implicitly excluded).
                        size_type selected_var = "";
                        size_type min_weight = -1ULL;
                        for(const auto &rel_var : rel_vars){
                            if(!is_var_bound(rel_var, b_vars)){
                                rel_var_processed = true;
                                //All iterators of 'var'
                                auto iters =  m_var_to_iterators.find(rel_var);
                                if(iters != m_var_to_iterators.end()){
                                    std::vector<ltj_iter_type*> var_iters = iters->second;
                                    for(ltj_iter_type* it : var_iters){
                                        //The iterator has a reference to its triple pattern.
                                        //const triple_pattern& triple_pattern = *(it->get_triple_pattern());
                                        const ltj_iter_type &iter = *it;
                                        size_type weight = 0;
                                        if(util::configuration.uses_muthu()){
                                            weight = util::get_num_diff_values<ring_type, ltj_iter_type>(rel_var, m_ptr_ring, iter);
                                        } else {
                                            weight = util::get_size_interval<ltj_iter_type>(iter);
                                        }
                                        if(weight < min_weight){
                                            min_weight = weight;
                                            selected_var = rel_var;
                                        }
                                    }
                                }
                            }
                        }
                        if(rel_var_processed){
                            //assert(!heap.empty());
                            //var_type next_var = heap.top().second;
                            //return next_var;
                            return selected_var;
                        }
                        //(4). Lonely variables.
                        const auto & lonely_vars = m_gao_size.get_lonely_variables();
                        var_type next_var = -1;
                        for(const var_type& var : lonely_vars){
                            if(!is_var_bound(var, b_vars)){
                                next_var = var;
                                break;
                            }
                        }
                        assert(next_var != -1);
                        return next_var;
                    }
                }
            }
            else{
                return m_gao[j];
            }
        }
        var_type is_var_bound(const size_type var, const std::unordered_map<var_type,bool> &m_gao_vars) const{
            auto it = m_gao_vars.find(var);
            //auto it = std::find(m_gao_vars.begin(), m_gao_vars.end(), var);
            if (it != m_gao_vars.end())
                if(it->second)
                    return true;
            return false;
        }

        void push_var_to_stack(const var_type& x_j){
            //assert (m_gao_stack.top() == x_j);
            m_gao_stack.push(x_j);
            m_gao_vars[x_j]=true;
        }

        void pop_var_of_stack(const var_type& x_j){
            if(!m_gao_stack.empty() && m_gao_stack.size() > 1){
                auto v = m_gao_stack.top();
                m_gao_stack.pop();
                m_gao_vars[v]=false;
            }
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
                        //2. Going down in the trie by setting x_j = c (\mu(t_i) in paper)
                        itrs[0]->down(x_j, c);
                        //2. Search with the next variable x_{j+1}
                        ok = search(j + 1, tuple, res, start, limit_results, timeout_seconds);
                        if(!ok) return false;
                        //4. Going up in the trie by removing x_j = c
                        itrs[0]->up(x_j);
                        //pop_var_of_stack(x_j);
                    }
                }else {
                    value_type c = seek(x_j);
                    //std::cout << "Seek (init): (" << (uint64_t) x_j << ": " << c << ")" <<std::endl;
                    while (c != 0) { //If empty c=0
                        //1. Adding result to tuple
                        tuple[j] = {x_j, c};
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
                        //pop_var_of_stack(x_j);
                    }
                }
                pop_var_of_stack(x_j);
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
