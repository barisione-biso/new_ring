/*
 * gao.hpp
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


#ifndef RING_GAO_HPP
#define RING_GAO_HPP

#include <ring.hpp>
#include <unordered_map>
#include <vector>
#include <utils.hpp>
#include <unordered_set>

namespace ring {

    template<class ring_t = ring<>, class var_t = uint8_t, class cons_t = uint64_t >
    class gao_size {

    public:
        typedef var_t var_type;
        typedef cons_t cons_type;
        typedef uint64_t size_type;
        typedef ring_t ring_type;
        typedef struct {
            var_type name;
            size_type weight;
            size_type n_triples;
            std::unordered_set<var_type> related;
        } info_var_type;
        std::vector<info_var_type> m_var_info;
        typedef ltj_iterator<ring_type, var_type, cons_type> ltj_iter_type;
        typedef std::pair<size_type, var_type> pair_type;
        typedef std::priority_queue<pair_type, std::vector<pair_type>, greater<pair_type>> min_heap_type;
        std::vector<var_type> m_lonely_variables;
        std::vector<var_type> m_linked_variables; //Non-lonely vars
        std::unordered_map<var_type, size_type> m_hash_table_position;
        size_type m_number_of_variables;
    private:
        const std::vector<triple_pattern>* m_ptr_triple_patterns;
        const std::vector<ltj_iter_type>* m_ptr_iterators;
        ring_type* m_ptr_ring;


        void var_to_vector(const var_type var, const size_type size,
                            std::unordered_map<var_type, size_type> &hash_table,
                            std::vector<info_var_type> &vec){

            auto it = hash_table.find(var);
            if(it == hash_table.end()){
                info_var_type info;
                info.name = var;
                info.weight = size;
                info.n_triples = 1;
                vec.emplace_back(info);
                hash_table.insert({var, vec.size()-1});
            }else{
                info_var_type& info = vec[it->second];
                ++info.n_triples;
                if(info.weight > size){
                    info.weight = size;
                }
            }
        }

        void var_to_related(const var_type var, const var_type rel,
                            std::unordered_map<var_type, size_type> &hash_table,
                            std::vector<info_var_type> &vec){

            auto pos_var = hash_table[var];
            vec[pos_var].related.insert(rel);
            auto pos_rel = hash_table[rel];
            vec[pos_rel].related.insert(var);
        }

        void fill_heap(const var_type var,
                        std::unordered_map<var_type, size_type> &hash_table,
                        std::vector<info_var_type> &vec,
                        std::vector<bool> &checked,
                        min_heap_type &heap){

            auto pos_var = hash_table[var];
            for(const auto &e : vec[pos_var].related){
                auto pos_rel = hash_table[e];
                if(!checked[pos_rel] && vec[pos_rel].n_triples > 1){
                    heap.push({vec[pos_rel].weight, e});
                    checked[pos_rel] = true;
                }
            }
        }

        struct compare_var_info
        {
            inline bool operator() (const info_var_type& linfo, const info_var_type& rinfo)
            {
                if(linfo.n_triples>1 && rinfo.n_triples==1){
                    return true;
                }
                if(linfo.n_triples==1 && rinfo.n_triples>1){
                    return false;
                }
                return linfo.weight < rinfo.weight;
            }
        };

        void copy(const gao_size &o) {
            m_ptr_triple_patterns = std::move(o.m_ptr_triple_patterns);
            m_ptr_iterators = std::move(o.m_ptr_iterators);
            m_ptr_ring = std::move(o.m_ptr_ring);
            m_var_info = std::move(o.var_info);
            m_lonely_variables = std::move(o.m_lonely_variables);
            m_linked_variables = std::move(o.m_linked_variables);
            m_hash_table_position = std::move(o.m_hash_table_position);
            m_number_of_variables = std::move(o.m_number_of_variables);
        }
    public:
        gao_size() = default;
        //Used exclusively for fixed gao.
        gao_size(const std::vector<triple_pattern>* triple_patterns,
                    const std::vector<ltj_iter_type>* iterators,
                    ring_type* r,
                    std::vector<var_type> &gao) : m_number_of_variables(0){
            m_ptr_triple_patterns = triple_patterns;
            m_ptr_iterators = iterators;
            m_ptr_ring = r;


            //1. Filling var_info with data about each variable
            //std::cout << "Filling... " << std::flush;
            //std::vector<info_var_type> var_info;
            //std::unordered_map<var_type, size_type> hash_table_position;
            size_type i = 0;
            for (const triple_pattern& triple_pattern : *m_ptr_triple_patterns) {
                var_type var_s, var_p, var_o;
                bool s = false, p = false, o = false;
                if(util::configuration.uses_get_size_interval()){
                    size_type size = util::get_size_interval<ltj_iter_type>(m_ptr_iterators->at(i));
                    if(triple_pattern.s_is_variable()){
                        s = true;
                        var_s = (var_type) triple_pattern.term_s.value;
                        var_to_vector(var_s, size,m_hash_table_position, m_var_info);
                    }
                    if(triple_pattern.p_is_variable()){
                        p = true;
                        var_p = (var_type) triple_pattern.term_p.value;
                        var_to_vector(var_p, size,m_hash_table_position, m_var_info);
                    }
                    if(triple_pattern.o_is_variable()){
                        o = true;
                        var_o = triple_pattern.term_o.value;
                        var_to_vector(var_o, size,m_hash_table_position, m_var_info);
                    }
                } else if(util::configuration.uses_muthu()){
                    auto var_size_map = util::get_num_diff_values<ring_type, ltj_iter_type>(m_ptr_ring, triple_pattern, m_ptr_iterators->at(i));
                    if(triple_pattern.s_is_variable()){
                        s = true;
                        var_s = (var_type) triple_pattern.term_s.value;
                        var_to_vector(var_s, var_size_map[var_s],m_hash_table_position, m_var_info);
                    }
                    if(triple_pattern.p_is_variable()){
                        p = true;
                        var_p = (var_type) triple_pattern.term_p.value;
                        var_to_vector(var_p, var_size_map[var_p],m_hash_table_position, m_var_info);
                    }
                    if(triple_pattern.o_is_variable()){
                        o = true;
                        var_o = triple_pattern.term_o.value;
                        var_to_vector(var_o, var_size_map[var_o],m_hash_table_position, m_var_info);
                    }
                }
                if(s && p){
                    var_to_related(var_s, var_p, m_hash_table_position, m_var_info);
                }
                if(s && o){
                    var_to_related(var_s, var_o, m_hash_table_position, m_var_info);
                }
                if(p && o){
                    var_to_related(var_p, var_o, m_hash_table_position, m_var_info);
                }
                ++i;
            }
            //std::cout << "Done. " << std::endl;

            //2. Sorting variables according to their weights.
            //std::cout << "Sorting... " << std::flush;
            std::sort(m_var_info.begin(), m_var_info.end(), compare_var_info());
            size_type lonely_start = m_var_info.size();
            m_number_of_variables = m_var_info.size();
            m_linked_variables.reserve(m_var_info.size());
            m_lonely_variables.reserve(m_var_info.size());
            for(i = 0; i < m_var_info.size(); ++i){
                m_hash_table_position[m_var_info[i].name] = i;
                if(m_var_info[i].n_triples == 1 && i < lonely_start){
                    lonely_start = i;
                    m_lonely_variables.emplace_back(m_var_info[i].name);
                }else{
                    m_linked_variables.emplace_back(m_var_info[i].name);
                }
            }
            //std::cout << "Done. " << std::endl;
            if(!util::configuration.uses_muthu()){
                //3. Choosing the variables
                i = 0;
                //std::cout << "Choosing GAO ... " << std::flush;
                std::vector<bool> checked(m_var_info.size(), false);
                gao.reserve(m_var_info.size());
                while(i < lonely_start){ //Related variables
                    if(!checked[i]){
                        gao.push_back(m_var_info[i].name); //Adding var to gao
                        checked[i] = true;
                        min_heap_type heap; //Stores the related variables that are related with the chosen ones
                        auto var_name = m_var_info[i].name;
                        fill_heap(var_name, m_hash_table_position, m_var_info, checked,heap);
                        while(!heap.empty()){
                            var_name = heap.top().second;
                            heap.pop();
                            gao.push_back(var_name);
                            fill_heap(var_name, m_hash_table_position, m_var_info, checked, heap);
                        }
                    }
                    ++i;
                }

                while(i < m_var_info.size()){ //Lonely variables
                    gao.push_back(m_var_info[i].name); //Adding var to gao
                    ++i;
                }
                assert(gao.size() == (m_linked_variables.size() + m_lonely_variables.size()));
            }
            //std::cout << "Done. " << std::endl;
        }

        //! Copy constructor
        gao_size(const gao_size &o) {
            copy(o);
        }

        //! Move constructor
        gao_size(gao_size &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        gao_size &operator=(const gao_size &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }
        //! Move Operator=
        gao_size &operator=(gao_size &&o) {
            if (this != &o) {
                m_ptr_triple_patterns = std::move(o.m_ptr_triple_patterns);
                m_ptr_iterators = std::move(o.m_ptr_iterators);
                m_ptr_ring = std::move(o.m_ptr_ring);
                m_var_info = std::move(o.m_var_info);
                m_lonely_variables = std::move(o.m_lonely_variables);
                m_linked_variables = std::move(o.m_linked_variables);
                m_hash_table_position = std::move(o.m_hash_table_position);
                m_number_of_variables = std::move(o.m_number_of_variables);
            }
            return *this;
        }

        void swap(gao_size &o) {
            std::swap(m_ptr_triple_patterns, o.m_ptr_triple_patterns);
            std::swap(m_ptr_iterators, o.m_ptr_iterators);
            std::swap(m_ptr_ring, o.m_ptr_ring);
            std::swap(m_var_info, o.m_var_info);
            std::swap(m_lonely_variables, o.m_lonely_variables);
            std::swap(m_linked_variables, o.m_linked_variables);
            std::swap(m_hash_table_position, o.m_hash_table_position);
            std::swap(m_number_of_variables, o.m_number_of_variables);
        }
        std::unordered_set<var_type> get_related_variables(const var_type& var){
            std::unordered_set<var_type> r;
            const auto& iter = m_hash_table_position.find(var);
            if(iter != m_hash_table_position.end()){
                size_type index = iter->second;
                const auto& v = m_var_info[index];
                r = v.related;
            }
            
            return r;
        }
        std::vector<var_type> get_lonely_variables() const{
            return m_lonely_variables;
        }
        std::vector<var_type> get_linked_variables() const{
            return m_linked_variables;
        }
    };

    /*
    template<class ring_t = ring<>, class var_t = uint8_t, class cons_t = uint64_t >
    class gao{
        typedef var_t var_type;
        typedef cons_t cons_type;
        typedef uint64_t size_type;
        typedef ring_t ring_type;
        typedef ltj_iterator<ring_type, var_type, cons_type> ltj_iter_type;
        private:
            std::vector<var_type> m_gao;
            const std::vector<triple_pattern>* m_ptr_triple_patterns;
            const std::vector<ltj_iter_type>* m_ptr_iterators;
            ring_type* m_ptr_ring;
        public:
            gao(){}
            gao(const std::vector<triple_pattern>* triple_patterns,
                    const std::vector<ltj_iter_type>* iterators,
                    ring_type* r) {
                m_ptr_triple_patterns = triple_patterns;
                m_ptr_iterators = iterators;
                m_ptr_ring = r;
                //Calculates gao.
                gao_size<ring_type> gao_sv2(triple_patterns, r, m_ptr_ring, m_gao);
            }
            var_type next() const{
                return 1;
            }
            std::string get_gao(std::unordered_map<uint8_t, std::string>& ht) const{
                std::string str = "";
                for(const auto& var : m_gao){
                    str += "?" + ht[var] + " ";
                }
                return str;
            }
            var_type size() const{
                //returns number of variables not m_gao.size()
                return 1;
            }
            var_type operator[](size_type i) const{
                return m_gao[i];
            }
    };*/
}

#endif //RING_GAO_HPP
