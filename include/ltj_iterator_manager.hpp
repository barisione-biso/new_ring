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

    template<class ring_t, class reverse_ring_t, class var_t, class cons_t>
    class ltj_iterator_manager {

    public:
        typedef cons_t value_type;
        typedef var_t var_type;
        typedef ring_t ring_type;
        typedef reverse_ring_t reverse_ring_type;
        typedef uint64_t size_type;
        typedef ltj_iterator<ring_type, var_type, value_type> ltj_iter_type;
        typedef ltj_reverse_iterator<reverse_ring_type, var_type, value_type> ltj_reverse_iter_type;

    private:
        const triple_pattern *m_ptr_triple_pattern;
        ring_type *m_ptr_ring; //TODO: should be const
        reverse_ring_type *m_ptr_reverse_ring;
        bwt_interval m_i_s;
        bwt_interval m_i_o;
        bwt_interval m_i_p;
        value_type m_cur_s;
        value_type m_cur_o;
        value_type m_cur_p;
        bool m_is_empty = false;
        ltj_iter_type m_iter;
        ltj_reverse_iter_type m_reverse_iter;
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
            m_iter = o.m_iter;
            m_reverse_iter = o.m_reverse_iter;
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

        ltj_iterator_manager(const triple_pattern *triple, ring_type *ring, reverse_ring_type *reverse_ring) {
            m_ptr_triple_pattern = triple;
            m_ptr_ring = ring;
            m_ptr_reverse_ring = reverse_ring;
            m_cur_s = -1UL;
            m_cur_p = -1UL;
            m_cur_o = -1UL;
            /*
            m_i_p = m_ptr_ring->open_PSO();
            m_i_s = m_ptr_ring->open_SOP();
            m_i_o = m_ptr_ring->open_OPS();
            */
            m_iter = ltj_iter_type(triple, m_ptr_ring);
            m_reverse_iter = ltj_reverse_iter_type(triple, m_ptr_reverse_ring);
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
                m_iter = std::move(o.m_iter);
                m_reverse_iter = std::move(o.m_reverse_iter);
            }
            return *this;
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
            std::swap(m_iter, o.m_iter);
            std::swap(m_reverse_iter,o.m_reverse_iter);
        }

        void down(var_type var, size_type c) { //Go down in the trie
            m_iter.down(var,c);
        };
        //Reverses the intervals and variable weights. Also resets the current value.
        void up(var_type var) { //Go up in the trie
            m_iter.up(var);
        };

        value_type leap(var_type var) { //Return the minimum in the range
            return m_iter.leap(var);
        };

        value_type leap(var_type var, size_type c) { //Return the next value greater or equal than c in the range
            return m_iter.leap(var,c);
        }

        bool in_last_level(){
            return m_iter.in_last_level();
        }

        //Solo funciona en último nivel, en otro caso habría que reajustar
        std::vector<uint64_t> seek_all(var_type var){
            return m_iter.seek_all(var);
        }
        /*
        bwt_interval get_i_s() const{
            return m_i_s;
        }
        bwt_interval get_i_p() const{
            return m_i_p;
        }
        bwt_interval get_i_o() const{
            return m_i_o;
        }*/
    };

}

#endif 