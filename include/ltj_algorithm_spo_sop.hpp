/*
 * ltj_algorithm_spo_sop.hpp
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



#ifndef RING_LTJ_SPO_SOP_ALGORITHM_HPP
#define RING_LTJ_SPO_SOP_ALGORITHM_HPP


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
    class ltj_algorithm_spo_sop {

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
        /*typedef struct {
            var_type var;
            std::vector<value_type> intersection;
            value_type index_next_val;
        } intersection_type;
        std::stack<intersection_type> m_intersection_cache;*/

        void copy(const ltj_algorithm_spo_sop &o) {
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

        /*
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
        void push_intersection(const var_type&x_j, const std::vector<value_type>&intersection){
            intersection_type i;
            i.var = x_j;
            i.intersection = std::move(intersection);
            i.index_next_val = 0;
            m_intersection_cache.push(i);
        }

        void pop_intersection(){
            m_intersection_cache.pop();
        }
        value_type get_next_value_intersection(){
            auto& top = m_intersection_cache.top();
            const auto& intersection = top.intersection;
            if(intersection.size() == 0){
                return 0;
            }else{
                if(top.index_next_val >= intersection.size())
                    return 0;

                value_type val = intersection[top.index_next_val++];
                return val;
            }
        }*/
        template<class t_wt, class range_type = sdsl::range_type>
        std::vector<typename t_wt::value_type>
        intersect_iter(const std::vector<t_wt*>& p_wts, const std::vector<range_type>& p_ranges)
        {
            //uint64_t count_nodes = 0;
            using std::get;
            using size_type      = typename t_wt::size_type;
            using value_type     = typename t_wt::value_type;
            using node_type      = typename t_wt::node_type;
            typedef struct {
                node_type node;
                range_type range;
            } tuple_type;
            using stack_vector_type = std::vector<tuple_type>;
            typedef std::stack<stack_vector_type> stack_type;

            std::vector<value_type> res;
            stack_vector_type vec;
            stack_type stack;

            if(p_wts.size() == 0){
               return res;
            }
            for(size_type i=0; i < p_wts.size(); i++){
                const t_wt& wt = *p_wts[i];
                //Can't be const & cause both node and ranges can get invalid / deleted during execution.
                node_type node = wt.root();
                range_type range = p_ranges[i];
                vec.emplace_back(tuple_type{node, range});
                //count_nodes++;
            }
            stack.emplace(vec);

            while (!stack.empty()) {
                bool symbol_reported = false;
                const stack_vector_type& wt_ranges_level_v = stack.top();
                bool empty_left_range = false, empty_right_range = false;
                stack_vector_type left_children_v;
                stack_vector_type right_children_v;

                if (p_wts[0]->is_leaf(wt_ranges_level_v[0].node)) {
                        res.emplace_back(p_wts[0]->sym(wt_ranges_level_v[0].node));
                        symbol_reported = true;
                }
                else{
                    for(size_type i = 0; i < wt_ranges_level_v.size() ; i++){
                        const t_wt& wt = *p_wts[i];
                        const node_type& node = wt_ranges_level_v[i].node;
                        const range_type& range = wt_ranges_level_v[i].range;

                        const auto& children = wt.expand(node);
                        const std::array<range_type, 2>& children_ranges = wt.expand(node, range);
                        //count_nodes++;
                        if(!empty_left_range){
                            //const range_type& left_range = std::get<0>(children_ranges);
                            //const range_type& right_range = std::get<1>(children_ranges);
                            if(sdsl::empty(children_ranges[0])){
                                empty_left_range = true;
                            }
                            left_children_v.emplace_back(tuple_type{get<0>(children), get<0>(children_ranges)});
                        }
                        if(!empty_right_range){
                            if(sdsl::empty(children_ranges[1])){
                                empty_right_range = true;
                            }
                            right_children_v.emplace_back(tuple_type{get<1>(children), get<1>(children_ranges)});
                        }

                        if(empty_left_range && empty_right_range){
                            break;
                        }
                    }
                }

                stack.pop();
                if(!symbol_reported && !empty_right_range){
                    stack.emplace(std::move(right_children_v));
                }
                if(!symbol_reported && !empty_left_range){
                    stack.emplace(std::move(left_children_v));
                }
            }
            //std::cout << "Intersection size: " << res.size() << " visited nodes : " << count_nodes << std::endl;
            return res;
        }        
    public:


        ltj_algorithm_spo_sop() = default;

        ltj_algorithm_spo_sop(const std::vector<triple_pattern>* triple_patterns, ring_type* ring, reverse_ring_type* reverse_ring){
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
            //m_gao = {'\000', '\001', '\003', '\002'};
            //m_gao = {'\000', '\002', '\001', '\003'};
            m_gao_vars.reserve(m_gao_size.m_number_of_variables);
            /*std::cout << "gao : ";
            for (auto& g : m_gao){
                std::cout << int(g) << " ";
            }
            std::cout << " " << std::endl;*/
        }

        //! Copy constructor
        ltj_algorithm_spo_sop(const ltj_algorithm_spo_sop &o) {
            copy(o);
        }

        //! Move constructor
        ltj_algorithm_spo_sop(ltj_algorithm_spo_sop &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        ltj_algorithm_spo_sop &operator=(const ltj_algorithm_spo_sop &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        ltj_algorithm_spo_sop &operator=(ltj_algorithm_spo_sop &&o) {
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

        void swap(ltj_algorithm_spo_sop &o) {
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
                /*std::cout << "tuple : ";
                for(auto& pair : tuple){
                    std::cout << int(pair.first) << " = " << pair.second << std::endl;
                }
                std::cout << " " << std::endl;*/
                res.emplace_back(tuple);
            }else{
                //assert(m_gao_stack.size() == m_gao_vars.size());
                var_type x_j = next(j);
                //std::cout << "next var: " << int(x_j) << std::endl;
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
                    //std::cout << "Intersecting ";
                    std::vector<wm_type*> wms;
                    std::vector<sdsl::range_type> ranges;
                    for(ltj_iter_type* iter : itrs){
                        //Set the index the algorithm will use before the first seek and only in the first level.
                        if(iter->get_index_permutation() == ""){
                            iter->set_iter(x_j);
                        }
                        //Getting the current interval and WMs of each iterator_x_j.
                        const auto& cur_interval = iter->get_current_interval(x_j);
                        const wm_type& current_wm  = iter->get_current_wm(x_j);
                        wms.emplace_back(&current_wm);
                        //assert (cur_interval.right() >= cur_interval.left() );
                        ranges.emplace_back(sdsl::range_type{cur_interval.left(), cur_interval.right()});
                        //std::cout << "iter used: " << iter->get_index_permutation() << " ( " << cur_interval.left() << " , " << cur_interval.right() << ")" ;
                    }
                    //std::cout << "" << std::endl;
                    /*
                    Fix: Sorting the wms vector by max_level ascendingly. This is key to support intersections from either S or O to P. 
                    The latter could have less (wt) levels than the first two.
                    If this situation is not handled accordingly then a segmentation fault happens.
                    */
                    std::sort(wms.begin(), wms.end(), [](wm_type* a, wm_type* b) {return a->max_level < b->max_level;});

                    const std::vector<value_type>&intersection = intersect_iter(wms,ranges);
                    for(value_type c : intersection){
                        //std::cout << "Seek : (" << (uint64_t) x_j << ": " << c << ")" <<std::endl;
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
                    }
                    //Unset type of iterator (SPO / SOP). See logic inside unset_iter() function.
                    if(util::configuration.is_adaptive()){
                        for(ltj_iter_type* iter : itrs){
                            iter->unset_iter(x_j);
                        }
                    }
                }
                if(util::configuration.is_adaptive()){
                    m_gao_size.set_previous_weight();
                }
                pop_var_of_stack();
            }
            return true;
        };


        /**
         *
         * @param x_j   Variable
         * @return      The next constant that matches the intersection between the triples of x_j.
         *              If the intersection is empty, it returns 0.
         */
        /*
        value_type seek(const var_type x_j, value_type c=-1UL){
            std::vector<ltj_iter_type*>& itrs = m_var_to_iterators[x_j];
            
            if(!is_intersection_calculated(x_j)){
                //std::cout << "Intersecting ";
                std::vector<wm_type*> wms;
                std::vector<sdsl::range_vec_type> ranges;
                for(ltj_iter_type* iter : itrs){
                    //Getting the current interval and WMs of each iterator_x_j.
                    const auto& cur_interval = iter->get_current_interval(x_j);
                    const wm_type& current_wm  = iter->get_current_wm(x_j);
                    wms.emplace_back(&current_wm);
                    //assert (cur_interval.right() >= cur_interval.left() );
                    ranges.emplace_back(sdsl::range_vec_type{{cur_interval.left(), cur_interval.right()}});
                    //std::cout << "iter used: " << iter->get_index_permutation() << " ( " << cur_interval.left() << " , " << cur_interval.right() << ")" ;
                }
                //std::cout << "" << std::endl;
                push_intersection(x_j, intersect_iter(wms,ranges));
            }

            size_type next_value = get_next_value_intersection();
            return next_value;
        } */
    };

}

#endif //RING_LTJ_ALGORITHM_HPP
