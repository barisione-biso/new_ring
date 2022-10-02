/*
 * ltj_iterator_manager.hpp
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

#ifndef RING_LTJ_ITERATOR_MANAGER_HPP
#define RING_LTJ_ITERATOR_MANAGER_HPP

#include <ltj_iterator.hpp>
#include <ltj_reverse_iterator.hpp>

#define VERBOSE 0

namespace ring {

    template<class ring_t,
    class reverse_ring_t,
    class var_t,
    class cons_t,
    class bwt_bit_vector_t = sdsl::bit_vector>
    class ltj_iterator_manager {

    public:
        typedef cons_t value_type;
        typedef var_t var_type;
        typedef ring_t ring_type;
        typedef reverse_ring_t reverse_ring_type;
        typedef uint64_t size_type;
        typedef ltj_iterator<ring_type, var_type, value_type> ltj_iter_type;
        typedef ltj_reverse_iterator<reverse_ring_type, var_type, value_type> ltj_reverse_iter_type;
        typedef sdsl::wm_int<bwt_bit_vector_t> wm_type;

    private:
        const triple_pattern *m_ptr_triple_pattern;
        ring_type *m_ptr_ring; //TODO: should be const
        reverse_ring_type *m_ptr_reverse_ring;
        bwt_interval m_i_s;//TODO: fix, Currently all the members bellow are used exclusively to precalculate gao and to do so we use SPO index values.
        bwt_interval m_i_o;//TODO: fix, Currently all the members bellow are used exclusively to precalculate gao and to do so we use SPO index values.
        bwt_interval m_i_p;//TODO: fix, Currently all the members bellow are used exclusively to precalculate gao and to do so we use SPO index values.
        value_type m_cur_s;//TODO: fix, Currently all the members bellow are used exclusively to precalculate gao and to do so we use SPO index values.
        value_type m_cur_o;//TODO: fix, Currently all the members bellow are used exclusively to precalculate gao and to do so we use SPO index values.
        value_type m_cur_p;//TODO: fix, Currently all the members bellow are used exclusively to precalculate gao and to do so we use SPO index values.
        bool m_is_empty = false;
        ltj_iter_type spo_iter;
        ltj_reverse_iter_type sop_iter;
        std::string m_last_iter;
        void copy(const ltj_iterator_manager &o) {
            m_ptr_triple_pattern = o.m_ptr_triple_pattern;
            m_ptr_ring = o.m_ptr_ring;
            m_ptr_reverse_ring = o.m_ptr_reverse_ring;
            m_i_s = o.m_i_s;
            m_i_p = o.m_i_p;
            m_i_o = o.m_i_o;
            m_cur_s = o.m_cur_s;
            m_cur_p = o.m_cur_p;
            m_cur_o = o.m_cur_o;
            m_is_empty = o.m_is_empty;
            spo_iter = o.spo_iter;
            sop_iter = o.sop_iter;
        }
    public:
        inline bool is_variable_subject(var_type var) {
            return m_ptr_triple_pattern->term_s.is_variable && var == m_ptr_triple_pattern->term_s.value;
        }

        inline bool is_variable_predicate(var_type var) {
            return m_ptr_triple_pattern->term_p.is_variable && var == m_ptr_triple_pattern->term_p.value;
        }

        inline bool is_variable_object(var_type var) {
            return m_ptr_triple_pattern->term_o.is_variable && var == m_ptr_triple_pattern->term_o.value;
        }

        const bool &is_empty = m_is_empty;
        const bwt_interval &i_s = m_i_s;
        const bwt_interval &i_p = m_i_p;
        const bwt_interval &i_o = m_i_o;
        const value_type &cur_s = m_cur_s;
        const value_type &cur_p = m_cur_p;
        const value_type &cur_o = m_cur_o;

        ltj_iterator_manager() = default;

        ltj_iterator_manager(const triple_pattern *triple, ring_type *ring, reverse_ring_type *reverse_ring) : m_last_iter(""){
            m_ptr_triple_pattern = triple;
            m_ptr_ring = ring;
            m_ptr_reverse_ring = reverse_ring;

            spo_iter = ltj_iter_type(triple, m_ptr_ring);
            //Currently all the members bellow are used exclusively to precalculate gao and to do so we use SPO index values.
            //>>
            m_cur_s = spo_iter.cur_s;
            m_cur_p = spo_iter.cur_p;
            m_cur_o = spo_iter.cur_o;
            m_i_p   = spo_iter.i_p;
            m_i_s   = spo_iter.i_s;
            m_i_o    =spo_iter.i_o;
            //<<
            sop_iter = ltj_reverse_iter_type(triple, m_ptr_reverse_ring);
        }
        const triple_pattern* get_triple_pattern() const{
            return m_ptr_triple_pattern;
        }
        //! Copy constructor
        ltj_iterator_manager(const ltj_iterator_manager &o) {
            copy(o);
        }

        //! Move constructor
        ltj_iterator_manager(ltj_iterator_manager &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        ltj_iterator_manager &operator=(const ltj_iterator_manager &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        ltj_iterator_manager &operator=(ltj_iterator_manager &&o) {
            if (this != &o) {
                m_i_s = std::move(o.m_i_s);
                m_i_p = std::move(o.m_i_p);
                m_i_o = std::move(o.m_i_o);
                m_cur_s = o.m_cur_s;
                m_cur_p = o.m_cur_p;
                m_cur_o = o.m_cur_o;
                m_is_empty = o.m_is_empty;
                m_ptr_triple_pattern = std::move(o.m_ptr_triple_pattern);
                m_ptr_ring = std::move(o.m_ptr_ring);
                m_ptr_reverse_ring = std::move(o.m_ptr_reverse_ring);
                spo_iter = std::move(o.spo_iter);
                sop_iter = std::move(o.sop_iter);
            }
            return *this;
        }
        std::string get_index_permutation() const{
            return m_last_iter;
        }
        void swap(ltj_iterator_manager &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_ptr_triple_pattern, o.m_ptr_triple_pattern);
            std::swap(m_ptr_ring, o.m_ptr_ring);
            std::swap(m_ptr_reverse_ring, o.m_ptr_reverse_ring);
            m_i_s.swap(o.m_i_s);
            m_i_o.swap(o.m_i_o);
            m_i_p.swap(o.m_i_p);
            std::swap(m_cur_s, o.m_cur_s);
            std::swap(m_cur_p, o.m_cur_p);
            std::swap(m_cur_o, o.m_cur_o);
            std::swap(m_is_empty, o.m_is_empty);
            std::swap(spo_iter, o.spo_iter);
            std::swap(sop_iter,o.sop_iter);
        }
        void set_iter(var_type var){
            if (is_variable_subject(var)) {
                if (m_cur_o != -1UL && m_cur_p != -1UL){
                    return;
                } else if (m_cur_o != -1UL) {
                    //OS->P
                    m_last_iter = "SOP";
                } else if (m_cur_p != -1UL) {
                    //PS->O
                    m_last_iter = "SPO";
                } 
            } else if (is_variable_predicate(var)) {
                if (m_cur_s != -1UL && m_cur_o != -1UL){
                    return;
                } else if (m_cur_o != -1UL) {
                    //OP->S
                    m_last_iter = "SPO";
                } else if (m_cur_s != -1UL) {
                    //SP->O
                    m_last_iter = "SOP";
                }
            } else if (is_variable_object(var)) {
                if (m_cur_s != -1UL && m_cur_p != -1UL){
                    return;
                }
                if (m_cur_p != -1UL) {
                    //PO->S
                    m_last_iter = "SOP";
                } else if (m_cur_s != -1UL) {
                    //SO->P
                    m_last_iter = "SPO";
                }
            }
        }
        void down(var_type var, size_type c) { //Go down in the trie
            //spo_iter.down(var,c);
            if (is_variable_subject(var)) {
                if (m_cur_o != -1UL && m_cur_p != -1UL){
                    /*if(m_last_iter == "SPO"){
                        spo_iter.down(var,c);
                    } else{
                        sop_iter.down(var,c);
                    }*/
                    return;
                } else if (m_cur_o != -1UL) {
                    //OS->P
                    sop_iter.down(var,c);
                    m_last_iter = "SOP";
                } else if (m_cur_p != -1UL) {
                    //PS->O
                    spo_iter.down(var,c);
                    m_last_iter = "SPO";
                } else {
                    //S->{OP,PO} same range in SOP and SPO
                    spo_iter.down(var,c);
                    sop_iter.down(var,c);
                }
                m_cur_s = c;
            } else if (is_variable_predicate(var)) {
                if (m_cur_s != -1UL && m_cur_o != -1UL){
                    /*if(m_last_iter == "SPO"){
                        spo_iter.down(var,c);
                    } else{
                        sop_iter.down(var,c);
                    }*/
                    return;
                } else if (m_cur_o != -1UL) {
                    //OP->S
                    spo_iter.down(var,c);
                    m_last_iter = "SPO";
                } else if (m_cur_s != -1UL) {
                    //SP->O
                    sop_iter.down(var,c);
                    m_last_iter = "SOP";
                } else {
                    //P->{OS,SO} same range in POS and PSO
                    spo_iter.down(var,c);
                    sop_iter.down(var,c);
                }
                m_cur_p = c;
            } else if (is_variable_object(var)) {
                if (m_cur_s != -1UL && m_cur_p != -1UL){
                    /*if(m_last_iter == "SPO"){
                        spo_iter.down(var,c);
                    } else{
                        sop_iter.down(var,c);
                    }*/
                    return;
                }
                if (m_cur_p != -1UL) {
                    //PO->S
                    sop_iter.down(var,c);
                    m_last_iter = "SOP";
                } else if (m_cur_s != -1UL) {
                    //SO->P
                    spo_iter.down(var,c);
                    m_last_iter = "SPO";
                } else {
                    //O->{PS,SP} same range in OPS and OSP
                    spo_iter.down(var,c);
                    sop_iter.down(var,c);
                }
                m_cur_o = c;
            }

        };
        //Reverses the intervals and variable weights. Also resets the current value.
        void up(var_type var) { //Go up in the trie
            //spo_iter.up(var);
            if (is_variable_subject(var)) {
                if (m_cur_o != -1UL && m_cur_p != -1UL){ //leaf of virtual trie.
                    if(m_last_iter == "SPO"){
                        spo_iter.up(var);
                    } else{
                        sop_iter.up(var);
                    }
                } else if (m_cur_o != -1UL || m_cur_p != -1UL) {//second level nodes.
                    if(m_last_iter == "SPO"){
                        spo_iter.up(var);
                    } else{
                        sop_iter.up(var);
                    }
                    //m_last_iter = "";
                } else {//first level nodes.
                    spo_iter.up(var);
                    sop_iter.up(var);
                }
                m_cur_s = -1UL;
            } else if (is_variable_predicate(var)) {
                if (m_cur_s != -1UL && m_cur_o != -1UL){
                    if(m_last_iter == "SPO"){
                        spo_iter.up(var);
                    } else{
                        sop_iter.up(var);
                    }
                } else if (m_cur_o != -1UL || m_cur_s != -1UL) {
                    if(m_last_iter == "SPO"){
                        spo_iter.up(var);
                    } else{
                        sop_iter.up(var);
                    }
                    //m_last_iter = "";
                } else {
                    spo_iter.up(var);
                    sop_iter.up(var);
                }
                m_cur_p = -1UL;
            } else if (is_variable_object(var)) {
                if (m_cur_s != -1UL && m_cur_p != -1UL){
                    if(m_last_iter == "SPO"){
                        spo_iter.up(var);
                    } else{
                        sop_iter.up(var);
                    }
                }
                if (m_cur_p != -1UL || m_cur_s != -1UL) {
                    if(m_last_iter == "SPO"){
                        spo_iter.up(var);
                    } else{
                        sop_iter.up(var);
                    }
                    //m_last_iter = "";
                } else {
                    spo_iter.up(var);
                    sop_iter.up(var);
                }
                m_cur_o = -1UL;
            }
        };

        bool in_last_level(){
            bool r = false;
            if(m_last_iter == "SPO"){
                r = spo_iter.in_last_level();
            }else{
                r = sop_iter.in_last_level();
            }
            return r;
        }

        //Solo funciona en último nivel, en otro caso habría que reajustar
        std::vector<uint64_t> seek_all(var_type var){
            if(m_last_iter == "SOP"){
                return sop_iter.seek_all(var);
            }else{
                return spo_iter.seek_all(var);
            }
        }
        bwt_interval get_current_interval(const var_type& var) const{
            if (is_variable_subject(var)){
                if(m_last_iter == "SOP"){
                    return sop_iter.get_i_s();
                }else{
                    return spo_iter.get_i_s();
                }
            }else if (is_variable_predicate(var)){
                if(m_last_iter == "SOP"){
                    return sop_iter.get_i_p();
                }else{
                    return spo_iter.get_i_p();
                }
            }else if (is_variable_object(var)){
                if(m_last_iter == "SOP"){
                    return sop_iter.get_i_o();
                }else{
                    return spo_iter.get_i_o();
                }
            }
        }

        const wm_type& get_current_wm(const var_type& var) const{
            if (is_variable_subject(var)){
                if(m_last_iter == "SOP"){
                    return m_ptr_reverse_ring->m_bwt_s.get_L();
                }else{
                    return m_ptr_ring->m_bwt_s.get_L();
                }
            }else if (is_variable_predicate(var)){
                 if(m_last_iter == "SOP"){
                    return m_ptr_reverse_ring->m_bwt_p.get_L();
                }else{
                    return m_ptr_ring->m_bwt_p.get_L();
                }
            }else if (is_variable_object(var)){
                 if(m_last_iter == "SOP"){
                    return m_ptr_reverse_ring->m_bwt_o.get_L();
                }else{
                    return m_ptr_ring->m_bwt_o.get_L();
                }
            }
        }
    };
}

#endif