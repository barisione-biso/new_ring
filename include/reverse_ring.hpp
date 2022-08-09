/*
 * reverse_ring.hpp
 * Copyright (C) 2022, Fabrizio
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

#ifndef RING_SOP
#define RING_SOP

#include <cstdint>
#include "bwt.hpp"
#include "bwt_interval.hpp"

#include <stdio.h>
#include <stdlib.h>

namespace ring {

    template <class bwt_t = bwt<>>
    class reverse_ring {
    public:
        typedef uint64_t size_type;
        typedef uint64_t value_type;
        typedef bwt_t bwt_type;
        typedef std::tuple<uint32_t, uint32_t, uint32_t> spo_triple_type;

        bwt_type m_bwt_s; //POS
        bwt_type m_bwt_p; //OSP
        bwt_type m_bwt_o; //SPO
    private:

        size_type m_max_s;
        size_type m_max_p;
        size_type m_max_o;
        size_type m_n_triples;  // number of triples
        size_type m_sigma_s;
        size_type m_sigma_p;
        size_type m_sigma_o;

        void copy(const reverse_ring &o) {
            m_bwt_s = o.m_bwt_s;
            m_bwt_p = o.m_bwt_p;
            m_bwt_o = o.m_bwt_o;
            m_max_s = o.m_max_s;
            m_max_p = o.m_max_p;
            m_max_o = o.m_max_o;
            m_n_triples = o.m_n_triples;
            m_sigma_s = o.m_sigma_s;
            m_sigma_p = o.m_sigma_p;
            m_sigma_o = o.m_sigma_o;
        }

    public:
        reverse_ring() = default;

        // Assumes the triples have been stored in a vector<spo_triple>
        reverse_ring(vector<spo_triple_type> &D) {
            typedef typename vector<spo_triple_type>::iterator triple_iterator;
            size_type i;
            triple_iterator it, triple_begin = D.begin(), triple_end = D.end();
            size_type n = m_n_triples = triple_end - triple_begin;
            int_vector<32> bwt_aux(3 * n);

            //cout << "  > Determining alphabets of S; O; P..."; fflush(stdout);
            {
                set<uint64_t> alphabet_S, alphabet_P, alphabet_O;
                for (it = triple_begin, i = 0; i < n; i++, it++) {
                    alphabet_S.insert(std::get<0>(*it));
                    alphabet_O.insert(std::get<1>(*it));
                    alphabet_P.insert(std::get<2>(*it));
                }

                //cout << "Done" << endl; fflush(stdout);
                m_sigma_s = alphabet_S.size();
                m_sigma_o = alphabet_O.size();
                m_sigma_p = alphabet_P.size();
                m_max_s = *alphabet_S.rbegin();
                m_max_o = *alphabet_O.rbegin();
                m_max_p = *alphabet_P.rbegin();
                alphabet_S.clear();
                alphabet_O.clear();
                alphabet_P.clear();
                //std::cout << " Maximum sizes. S = " << m_max_s << ", O = " << m_max_o << ", P = " << m_max_p << std::endl;
            }
            /*std::cout << "sigma S = " << m_sigma_s << std::endl;
            std::cout << "sigma O = " << m_sigma_o << std::endl;
            std::cout << "sigma P = " << m_sigma_p << std::endl;
            std::cout << "  > Determining number of elements per symbol..."; fflush(stdout);*/
            uint64_t alphabet_SO = (m_max_s < m_max_o) ? m_max_o : m_max_s;

            std::map<uint64_t, uint64_t> M_O, M_S, M_P;

            for (i = 1; i <= alphabet_SO; i++) {
                M_O[i] = 0;
                M_S[i] = 0;
            }

            for (i = 1; i <= m_max_p; i++)
                M_P[i] = 0;

            for (it = triple_begin, i = 0; i < n; i++, it++) {
                M_S[std::get<0>(*it)] = M_S[std::get<0>(*it)] + 1;
                M_O[std::get<1>(*it)] = M_O[std::get<1>(*it)] + 1;
                M_P[std::get<2>(*it)] = M_P[std::get<2>(*it)] + 1;
            }
            //std::cout << "Done" << std::endl; fflush(stdout);
            //std::cout << "  > Sorting out triples..."; fflush(stdout);
            // Sorts the triples lexycographically
            sort(triple_begin, triple_end);
            //std::cout << "Done" << std::endl; fflush(stdout);
            {
                int_vector<> t(3 * n + 2);
                //std::cout << "  > Generating int vector of the triples..."; fflush(stdout);
                for (i = 0, it = triple_begin; it != triple_end; it++, i++) {
                    t[3 * i] = std::get<0>(*it);//S
                    t[3 * i + 1] = std::get<1>(*it) + m_max_s; //O
                    t[3 * i + 2] = std::get<2>(*it) + m_max_s + m_max_o; //P
                }
                t[3 * n] = m_max_s + m_max_p + m_max_o + 1;
                t[3 * n + 1] = 0;
                D.clear();
                D.shrink_to_fit();
                util::bit_compress(t);

                {
                    int_vector<> sa;
                    qsufsort::construct_sa(sa, t);

                    //std::cout << "  > Suffix array built " << size_in_bytes(sa) << " bytes" <<  std::endl;
                    //std::cout << "  > Building the global BWT" << std::endl;

                    size_type j;
                    for (j = i = 0; i < sa.size(); i++) {
                        if (sa[i] >= 3 * n) continue;
                        if (sa[i] == 0)
                            bwt_aux[j] = t[t.size() - 3];
                        else bwt_aux[j] = t[sa[i] - 1];
                        j++;
                    }
                }
            }
            //cout << "  > Building m_bwt_p" << endl; fflush(stdout);
            // First P
            {
                int_vector<> P(n + 1);
                uint64_t j = 1, c;
                vector<uint64_t> C_P;
                P[0] = 0;
                for (i = 1; i < n; i++)
                    P[j++] = bwt_aux[i] - (m_max_s + m_max_o);

                // This is for making the bwt of triples circular
                P[j] = bwt_aux[0] - (m_max_s + m_max_o);
                util::bit_compress(P);
                //pre requisites to build the C bitmap. We use Map of S since they represent the range of P.
                uint64_t cur_pos = 1;

                C_P.push_back(0); // Dummy value
                C_P.push_back(cur_pos);
                for (c = 2; c <= alphabet_SO; c++) {
                    cur_pos += M_S[c - 1];
                    C_P.push_back(cur_pos);
                }
                C_P.push_back(n + 1);
                C_P.shrink_to_fit();

                M_S.clear();

                // builds the WT for BWT(P)
                m_bwt_p = bwt_type(P, C_P);
            }

            //cout << "  > Building m_bwt_s" << endl; fflush(stdout);
            // Then S
            {
                int_vector<> S(n + 1);
                uint64_t j = 1, c;
                vector<uint64_t> C_S;

                S[0] = 0;
                while (i < 2 * n) {
                    S[j++] = bwt_aux[i++];
                }
                util::bit_compress(S);

                uint64_t cur_pos = 1;
                C_S.push_back(0);  // Dummy value
                C_S.push_back(cur_pos);
                for (c = 2; c <= alphabet_SO; c++) {
                    cur_pos += M_O[c - 1];
                    C_S.push_back(cur_pos);
                }
                C_S.push_back(n + 1);
                C_S.shrink_to_fit();

                M_O.clear();

                m_bwt_s = bwt_type(S, C_S);
            }

            //cout << "  > Building m_bwt_o" << endl; fflush(stdout);
            // Then P
            {
                int_vector<> O(n + 1);
                uint64_t j = 1, c;
                vector<uint64_t> C_O;

                O[0] = 0;
                while (i < 3 * n) {
                    O[j++] = bwt_aux[i++] - m_max_s;
                }
                util::bit_compress(O);

                uint64_t cur_pos = 1;
                C_O.push_back(0);  // Dummy value
                C_O.push_back(cur_pos);
                for (c = 2; c <= m_max_p; c++) {
                    cur_pos += M_P[c - 1];
                    C_O.push_back(cur_pos);
                }
                C_O.push_back(n + 1);
                C_O.shrink_to_fit();

                M_P.clear();

                m_bwt_o = bwt_type(O, C_O);
            }
            cout << "-- Index constructed successfully" << endl;
            fflush(stdout);
        };


        //! Copy constructor
        reverse_ring(const reverse_ring &o) {
            copy(o);
        }

        //! Move constructor
        reverse_ring(reverse_ring &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        reverse_ring &operator=(const reverse_ring &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        reverse_ring &operator=(reverse_ring &&o) {
            if (this != &o) {
                m_bwt_s = std::move(o.m_bwt_s);
                m_bwt_p = std::move(o.m_bwt_p);
                m_bwt_o = std::move(o.m_bwt_o);
                m_max_s = o.m_max_s;
                m_max_p = o.m_max_p;
                m_max_o = o.m_max_o;
                m_n_triples = o.m_n_triples;
                m_sigma_s = o.m_sigma_s;
                m_sigma_p = o.m_sigma_p;
                m_sigma_o = o.m_sigma_o;
            }
            return *this;
        }

        void swap(reverse_ring &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_bwt_s, o.m_bwt_s);
            std::swap(m_bwt_p, o.m_bwt_p);
            std::swap(m_bwt_o, o.m_bwt_o);
            std::swap(m_max_s, o.m_max_s);
            std::swap(m_max_p, o.m_max_p);
            std::swap(m_max_o, o.m_max_o);
            std::swap(m_n_triples, o.m_n_triples);
            std::swap(m_sigma_s, o.m_sigma_s);
            std::swap(m_sigma_p, o.m_sigma_p);
            std::swap(m_sigma_o, o.m_sigma_o);
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_bwt_s.serialize(out, child, "bwt_s");
            written_bytes += m_bwt_p.serialize(out, child, "bwt_p");
            written_bytes += m_bwt_o.serialize(out, child, "bwt_o");
            written_bytes += sdsl::write_member(m_max_s, out, child, "max_s");
            written_bytes += sdsl::write_member(m_max_p, out, child, "max_p");
            written_bytes += sdsl::write_member(m_max_o, out, child, "max_o");
            written_bytes += sdsl::write_member(m_n_triples, out, child, "n_triples");
            written_bytes += sdsl::write_member(m_sigma_s, out, child, "sigma_s");
            written_bytes += sdsl::write_member(m_sigma_p, out, child, "sigma_p");
            written_bytes += sdsl::write_member(m_sigma_o, out, child, "sigma_o");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream &in) {
            m_bwt_s.load(in);
            m_bwt_p.load(in);
            m_bwt_o.load(in);
            sdsl::read_member(m_max_s, in);
            sdsl::read_member(m_max_p, in);
            sdsl::read_member(m_max_o, in);
            sdsl::read_member(m_n_triples, in);
            sdsl::read_member(m_sigma_s, in);
            sdsl::read_member(m_sigma_p, in);
            sdsl::read_member(m_sigma_o, in);
        }

        // The following init funtions work with suffix array positions
        // (i.e., positions in the global interval [1, 3*m_n_triples] )
        pair<uint64_t, uint64_t> init_no_constants() const {
            return {1, 3 * m_n_triples};
        }

        //Given a Suffix returns its range in BWT O
        pair<uint64_t, uint64_t> init_S(uint64_t S) const {
            return m_bwt_o.backward_search_1_interval(S);
        }

        //Given a Predicate returns its range in BWT S
        pair<uint64_t, uint64_t> init_P(uint64_t P) const {
            return m_bwt_s.backward_search_1_interval(P);
            //return {I.first + m_n_triples, I.second + m_n_triples};
        }

        //Given an Object returns its range in BWT P
        pair<uint64_t, uint64_t> init_O(uint64_t O) const {
            return m_bwt_p.backward_search_1_interval(O);
            //return {I.first + 2 * m_n_triples, I.second + 2 * m_n_triples};
        }


        //POS m_bwt_s
        //OSP m_bwt_p
        //SPO m_bwt_o

        //POS -> SPO
        pair<uint64_t, uint64_t> init_SP(uint64_t S, uint64_t P) const {
            auto I = m_bwt_s.backward_search_1_rank(P, S); //POS
            return m_bwt_o.backward_search_2_interval(S, I); //SPO
        }

        //SPO -> OSP
        pair<uint64_t, uint64_t> init_SO(uint64_t S, uint64_t O) const {
            auto I = m_bwt_o.backward_search_1_rank(S, O); //SPO
            return m_bwt_p.backward_search_2_interval(O, I); //OSP
           // return {I.first + 2 * m_n_triples, I.second + 2 * m_n_triples};
        }


        //OSP -> POS
        pair<uint64_t, uint64_t> init_PO(uint64_t P, uint64_t O) const {
            auto I = m_bwt_p.backward_search_1_rank(O, P); //OSP
            return m_bwt_s.backward_search_2_interval(P, I); //POS
            //return {I.first + m_n_triples, I.second + m_n_triples};
        }

        //OSP -> POS -> SPO
        pair<uint64_t, uint64_t> init_SPO(uint64_t S, uint64_t P, uint64_t O) const {
            auto I = m_bwt_p.backward_search_1_rank(O, P); //OSP
            I = m_bwt_s.backward_search_2_rank(P, S, I); //POS
            return m_bwt_o.backward_search_2_interval(S, I); //SPO
        }

        /**********************************/
        // Functions for PSO
        //
        /*
        bwt_interval open_PSO() {
            //return bwt_interval(2 * m_n_triples + 1, 3 * m_n_triples);
            return bwt_interval( 1, m_n_triples);
        }

        // P->S  (simulates going down in the trie)
        // Returns an interval within m_bwt_o
        bwt_interval down_P_S(bwt_interval &p_int, uint64_t s) {
            auto I = m_bwt_s.backward_step(p_int.left(), p_int.right(), s);
            uint64_t c = m_bwt_o.get_C(s);
            return bwt_interval(I.first + c, I.second + c);
        }

        uint64_t min_O_in_S(bwt_interval &I) {
            return I.begin(m_bwt_o);
        }

        uint64_t next_O_in_S(bwt_interval &I, uint64_t O) {
            if (O > m_max_o) return 0;

            uint64_t nextv = I.next_value(O, m_bwt_o);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bool there_are_O_in_S(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }

        uint64_t min_O_in_PS(bwt_interval &I) {
            return I.begin(m_bwt_o);
        }

        uint64_t next_O_in_PS(bwt_interval &I, uint64_t O) {
            if (O > m_max_o) return 0;

            uint64_t nextv = I.next_value(O, m_bwt_o);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bool there_are_O_in_PS(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }
        */
        std::vector<uint64_t>
        all_O_in_range(bwt_interval &I) {
            return m_bwt_o.values_in_range(I.left(), I.right());
        }
        /**********************************/
        // Functions for OPS
        //
        /*
        bwt_interval open_OPS() {
            return bwt_interval(1, m_n_triples);
        }

        // O->P  (simulates going down in the trie)
        // Returns an interval within m_bwt_s
        bwt_interval down_O_P(bwt_interval &o_int, uint64_t p) {
            auto I = m_bwt_p.backward_step(o_int.left(), o_int.right(), p);
            uint64_t c = m_bwt_s.get_C(p);
            return bwt_interval(I.first + c, I.second + c);
        }

        uint64_t min_S_in_OP(bwt_interval &I) {
            return I.begin(m_bwt_s);
        }

        uint64_t next_S_in_OP(bwt_interval &I, uint64_t s_value) {
            if (s_value > m_max_s) return 0;

            uint64_t nextv = I.next_value(s_value, m_bwt_s);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bool there_are_S_in_OP(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }

        uint64_t min_S_in_P(bwt_interval &I) {
            return I.begin(m_bwt_s);
        }

        uint64_t next_S_in_P(bwt_interval &I, uint64_t s_value) {
            if (s_value > m_max_s) return 0;

            uint64_t nextv = I.next_value(s_value, m_bwt_s);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bool there_are_S_in_P(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }

        std::vector<uint64_t>
        all_S_in_range(bwt_interval &I) {
            return m_bwt_s.values_in_range(I.left(), I.right());
        }
        */

        /**********************************/
        // Function for SOP
        //
        /*
        bwt_interval open_SOP() {
            return bwt_interval(1,  m_n_triples);
        }

        // S->O  (simulates going down in the trie)
        // Returns an interval within m_bwt_p
        bwt_interval down_S_O(bwt_interval &s_int, uint64_t o) {
            pair<uint64_t, uint64_t> I = m_bwt_o.backward_step(s_int.left(), s_int.right(), o);
            uint64_t c = m_bwt_p.get_C(o);
            return bwt_interval(I.first + c, I.second + c);
        }

        uint64_t min_P_in_SO(bwt_interval &I) {
            return I.begin(m_bwt_p);
        }

        uint64_t next_P_in_SO(bwt_interval &I, uint64_t p_value) {
            if (p_value > m_max_p) return 0;

            uint64_t nextv = I.next_value(p_value, m_bwt_p);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bool there_are_P_in_SO(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }

        uint64_t min_P_in_O(bwt_interval &I) {
            return I.begin(m_bwt_p);
        }

        uint64_t next_P_in_O(bwt_interval &I, uint64_t p_value) {
            if (p_value > m_max_p) return 0;

            uint64_t nextv = I.next_value(p_value, m_bwt_p);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bool there_are_P_in_O(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }
        */
        std::vector<uint64_t>
        all_P_in_range(bwt_interval &I) {
            return m_bwt_p.values_in_range(I.left(), I.right());
        }
        /**********************************/
        // Functions for SPO
        //
        /*
        bwt_interval open_SPO() {
            return bwt_interval(1, m_n_triples);
        }

        uint64_t min_S(bwt_interval &I) {
            return I.begin(m_bwt_s);
        }

        uint64_t next_S(bwt_interval &I, uint64_t s_value) {
            if (s_value > m_max_s) return 0;

            return I.next_value(s_value, m_bwt_s);
        }

        bwt_interval down_S(uint64_t s_value) {
            pair<uint64_t, uint64_t> i = init_S(s_value);
            return bwt_interval(i.first, i.second);
        }


        // S->P  (simulates going down in the trie, for the order SPO)
        // Returns an interval within m_bwt_p
        bwt_interval down_S_P(bwt_interval &s_int, uint64_t s_value, uint64_t p_value) {
            std::pair<uint64_t, uint64_t> q = s_int.get_stored_values();
            uint64_t b = q.first;
            if (q.first == (uint64_t) -1) {
                q = m_bwt_s.select_next(p_value, s_value, m_bwt_o.nElems(s_value));
                b = m_bwt_s.bsearch_C(q.first) - 1;
            }
            uint64_t nE = m_bwt_s.rank(b + 1, s_value) - m_bwt_s.rank(b, s_value);
            uint64_t start = q.second;

            return bwt_interval(s_int.left() + start, s_int.left() + start + nE - 1);
        }

        uint64_t min_P_in_S(bwt_interval &I, uint64_t s_value) {
            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_s.select_next(1, s_value, m_bwt_o.nElems(s_value));
            uint64_t b = m_bwt_s.bsearch_C(q.first) - 1;
            I.set_stored_values(b, q.second);
            return b;
        }

        uint64_t next_P_in_S(bwt_interval &I, uint64_t s_value, uint64_t p_value) {
            if (p_value > m_max_p) return 0;

            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_s.select_next(p_value, s_value, m_bwt_o.nElems(s_value));
            if (q.first == 0 && q.second == 0) {
                return 0;
            }

            uint64_t b = m_bwt_s.bsearch_C(q.first) - 1;
            I.set_stored_values(b, q.second);
            return b;
        }


        uint64_t min_O_in_SP(bwt_interval &I) {
            return I.begin(m_bwt_o);
        }

        uint64_t next_O_in_SP(bwt_interval &I, uint64_t O) {
            if (O > m_max_o) return 0;

            uint64_t next_v = I.next_value(O, m_bwt_o);
            if (next_v == 0)
                return 0;
            else
                return next_v;
        }

        bool there_are_O_in_SP(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }
        */
        /**********************************/
        // Functions for POS
        //
        /*
        bwt_interval open_POS() {
            return bwt_interval( 1, m_n_triples);
        }

        uint64_t min_P(bwt_interval &I) {
            //bwt_interval I_aux(I.left() - 2 * m_n_triples, I.right() - 2 * m_n_triples);
            //return I_aux.begin(m_bwt_p);
            return I.begin(m_bwt_p);
        }

        uint64_t next_P(bwt_interval &I, uint64_t p_value) {
            if (p_value > m_max_p) return 0;

            //bwt_interval I_aux(I.left() - 2 * m_n_triples, I.right() - 2 * m_n_triples);
            //uint64_t nextv = I_aux.next_value(p_value, m_bwt_p);
            uint64_t nextv = I.next_value(p_value, m_bwt_p);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bwt_interval down_P(uint64_t p_value) {
            pair<uint64_t, uint64_t> i = init_P(p_value);
            return bwt_interval(i.first, i.second);
        }

        // P->O  (simulates going down in the trie, for the order POS)
        // Returns an interval within m_bwt_p
        bwt_interval down_P_O(bwt_interval &p_int, uint64_t p_value, uint64_t o_value) {
            std::pair<uint64_t, uint64_t> q = p_int.get_stored_values();
            uint64_t b = q.first;
            if (q.first == (uint64_t) -1) {
                q = m_bwt_p.select_next(o_value, p_value, m_bwt_s.nElems(p_value));
                b = m_bwt_p.bsearch_C(q.first) - 1;
            }
            uint64_t nE = m_bwt_p.rank(b + 1, p_value) - m_bwt_p.rank(b, p_value);
            uint64_t start = q.second;

            return bwt_interval(p_int.left() + start, p_int.left() + start + nE - 1);
        }

        uint64_t min_O_in_P(bwt_interval &p_int, uint64_t p_value) {
            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_p.select_next(1, p_value, m_bwt_s.nElems(p_value));
            uint64_t b = m_bwt_p.bsearch_C(q.first) - 1;
            p_int.set_stored_values(b, q.second);
            return b;
        }

        uint64_t next_O_in_P(bwt_interval &I, uint64_t p_value, uint64_t o_value) {
            if (o_value > m_max_o) return 0;

            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_p.select_next(o_value, p_value, m_bwt_s.nElems(p_value));
            if (q.first == 0 && q.second == 0)
                return 0;

            uint64_t b = m_bwt_p.bsearch_C(q.first) - 1;
            I.set_stored_values(b, q.second);
            return b;
        }

        uint64_t min_S_in_PO(bwt_interval &I) {
           // bwt_interval I_aux = bwt_interval(I.left() - m_n_triples, I.right() - m_n_triples);
            //return I_aux.begin(m_bwt_s);
            return I.begin(m_bwt_s);
        }

        uint64_t next_S_in_PO(bwt_interval &I, uint64_t s_value) {
            if (s_value > m_max_s) return 0;

            //bwt_interval I_aux = bwt_interval(I.left() - m_n_triples, I.right() - m_n_triples);
            //return I_aux.next_value(s_value, m_bwt_s);
            return I.next_value(s_value, m_bwt_s);
        }

        bool there_are_S_in_PO(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }
        */
        /**********************************/
        // Functions for OSP
        //
        /*
        bwt_interval open_OSP() {
            return bwt_interval(1, m_n_triples);
        }

        uint64_t min_O(bwt_interval &I) {
            return I.begin(m_bwt_o);
        }

        uint64_t next_O(bwt_interval &I, uint64_t o_value) {
            if (o_value > m_max_o) return 0;

            uint64_t nextv = I.next_value(o_value, m_bwt_o);
            if (nextv == 0) return 0;
            else return nextv;
        }

        bwt_interval down_O(uint64_t o_value) {
            pair<uint64_t, uint64_t> i = init_O(o_value);
            return bwt_interval(i.first, i.second);
        }

        // P->O  (simulates going down in the trie, for the order OSP)
        // Returns an interval within m_bwt_p
        bwt_interval down_O_S(bwt_interval &o_int, uint64_t o_value, uint64_t s_value) {
            std::pair<uint64_t, uint64_t> q = o_int.get_stored_values();
            uint64_t b = q.first;
            if (q.first == (uint64_t) -1) {
                q = m_bwt_o.select_next(s_value, o_value, m_bwt_p.nElems(o_value));
                b = m_bwt_o.bsearch_C(q.first) - 1;
            }
            uint64_t nE = m_bwt_o.rank(b + 1, o_value) - m_bwt_o.rank(b, o_value);
            uint64_t start = q.second;

            return bwt_interval(o_int.left() + start, o_int.left() + start + nE - 1);
        }

        uint64_t min_S_in_O(bwt_interval &o_int, uint64_t o_value) {
            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_o.select_next(1, o_value, m_bwt_p.nElems(o_value));
            uint64_t b = m_bwt_o.bsearch_C(q.first) - 1;
            o_int.set_stored_values(b, q.second);
            return b;
        }

        uint64_t next_S_in_O(bwt_interval &I, uint64_t o_value, uint64_t s_value) {
            if (s_value > m_max_s) return 0;

            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_o.select_next(s_value, o_value, m_bwt_p.nElems(o_value));
            if (q.first == 0 && q.second == 0)
                return 0;

            uint64_t b = m_bwt_o.bsearch_C(q.first) - 1;
            I.set_stored_values(b, q.second);
            return b;
        }

        uint64_t min_P_in_OS(bwt_interval &I) {
            return I.begin(m_bwt_p);
        }

        uint64_t next_P_in_OS(bwt_interval &I, uint64_t p_value) {
            if (p_value > m_max_p) return 0;

            uint64_t nextv = I.next_value(p_value, m_bwt_p);
            if (nextv == 0)
                return 0;
            else
                return nextv;
        }

        bool there_are_P_in_OS(bwt_interval &I) {
            return I.get_cur_value() != I.end();
        }
        */

        bwt_type get_m_bwt_s() const{
            return m_bwt_s;
        }

        bwt_type get_m_bwt_p() const{
            return m_bwt_p;
        }

        bwt_type get_m_bwt_o() const{
            return m_bwt_o;
        }

        size_type get_n_triples() const{
            return m_n_triples;
        }
        /*
        std::vector<uint64_t> get_P_given_S(uint64_t symbol_id){
            std::vector<u_int64_t> results;
            //auto num_elems = BWT_P.nElems(symbol_id);
            bwt_interval aux_i = bwt_interval(BWT_P.get_C(symbol_id) , BWT_P.get_C(symbol_id + 1) -1);
            auto values = all_P_in_range(aux_i);
            for (auto& value : values) {
                //std::cout<< value.first << " - " << value.second << std::endl;
                results.push_back(value.second);
            }
        }

        std::vector<uint64_t> get_S_given_O(uint64_t symbol_id){
            std::vector<u_int64_t> results;
            //based on ring query debugging. check getPGivenS
            auto num_elems = BWT_P.nElems(symbol_id);
            bwt_interval aux_i = bwt_interval(symbol_id, num_elems);
            auto current_p = min_S_in_O(aux_i, symbol_id);
            uint64_t old_p = 0;
            results.push_back(current_p);
            for(int q=0; q < num_elems; q++){
                current_p = next_S_in_O(aux_i, symbol_id, ++current_p);
                if(current_p <= old_p){
                    break;
                }
                old_p = current_p;
                results.push_back(current_p);
            }
            results.shrink_to_fit();
            return results;
        }


        std::vector<uint64_t> get_O_given_P(uint64_t symbol_id){
            std::vector<u_int64_t> results;
            //based on ring query debugging. check getPGivenS
            auto num_elems = BWT_S.nElems(symbol_id);
            bwt_interval aux_i = bwt_interval(symbol_id, num_elems);
            auto current_p = min_O_in_P(aux_i, symbol_id);
            uint64_t old_p = 0;
            results.push_back(current_p);
            for(int q=0; q < num_elems; q++){
                current_p = next_O_in_P(aux_i, symbol_id, ++current_p);
                if(current_p <= old_p){
                    break;
                }
                old_p = current_p;
                results.push_back(current_p);
            }
            results.shrink_to_fit();
            return results;
        }*/
    };
}

#endif
