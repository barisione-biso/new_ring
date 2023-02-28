/*
 * ring.hpp
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

#ifndef RING_SPO
#define RING_SPO

#include <cstdint>
#include "bwt.hpp"
#include "bwt_interval.hpp"
#include "crc_arrays.hpp"
#include <stdio.h>
#include <stdlib.h>

namespace ring {

    template <class bwt_t = bwt<>, class crc_arrays_t = crc_arrays<>>
    class ring {
    public:
        typedef uint64_t size_type;
        typedef uint64_t value_type;
        typedef bwt_t bwt_type;
        typedef crc_arrays_t crc_arrays_type;
        typedef std::tuple<uint32_t, uint32_t, uint32_t> spo_triple_type;

        bwt_type m_bwt_s; //POS
        bwt_type m_bwt_p; //OSP
        bwt_type m_bwt_o; //SPO
        //uint64_t fw_count;
    private:

        size_type m_max_s;
        size_type m_max_p;
        size_type m_max_o;
        size_type m_n_triples;  // number of triples
        size_type m_sigma_s;
        size_type m_sigma_p;
        size_type m_sigma_o;

        crc_arrays_type m_crc_arrays;

        void copy(const ring &o) {
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
            m_crc_arrays = o.m_crc_arrays;
        }

    public:
        ring() = default;

        // Assumes the triples have been stored in a vector<spo_triple>
        // This constructor is used to index the data.
        ring(vector<spo_triple_type> &D) {
            typedef typename vector<spo_triple_type>::iterator triple_iterator;
            size_type i;
            triple_iterator it, triple_begin = D.begin(), triple_end = D.end();
            size_type n = m_n_triples = triple_end - triple_begin;
            sdsl::int_vector<32> bwt_aux(3 * n);

            //cout << "  > Determining alphabets of S; P; O..."; fflush(stdout);
            {
                set<uint64_t> alphabet_S, alphabet_P, alphabet_O;
                for (it = triple_begin, i = 0; i < n; i++, it++) {
                    alphabet_S.insert(std::get<0>(*it));
                    alphabet_P.insert(std::get<1>(*it));
                    alphabet_O.insert(std::get<2>(*it));
                }

                //cout << "Done" << endl; fflush(stdout);
                m_sigma_s = alphabet_S.size();
                m_sigma_p = alphabet_P.size();
                m_sigma_o = alphabet_O.size();
                m_max_s = *alphabet_S.rbegin();
                m_max_p = *alphabet_P.rbegin();
                m_max_o = *alphabet_O.rbegin();
                alphabet_S.clear();
                alphabet_P.clear();
                alphabet_O.clear();
            }
            //cout << "sigma S = " << sigma_S << endl;
            //cout << "sigma P = " << m_sigma_p << endl;
            //cout << "sigma O = " << sigma_O << endl;
            //cout << "  > Determining number of elements per symbol..."; fflush(stdout);
            uint64_t alphabet_SO = (m_max_s < m_max_o) ? m_max_o : m_max_s;

            std::map<uint64_t, uint64_t> M_O, M_S, M_P;

            for (i = 1; i <= alphabet_SO; i++) {
                M_O[i] = 0;
                M_S[i] = 0;
            }

            for (i = 1; i <= m_max_p; i++)
                M_P[i] = 0;

            for (it = triple_begin, i = 0; i < n; i++, it++) {
                M_O[std::get<2>(*it)] = M_O[std::get<2>(*it)] + 1;
                M_S[std::get<0>(*it)] = M_S[std::get<0>(*it)] + 1;
                M_P[std::get<1>(*it)] = M_P[std::get<1>(*it)] + 1;
            }
            //cout << "Done" << endl; fflush(stdout);
            //cout << "  > Sorting out triples..."; fflush(stdout);
            // Sorts the triples lexycographically
            sort(triple_begin, triple_end);
            //cout << "Done" << endl; fflush(stdout);
            {
                sdsl::int_vector<> t(3 * n + 2);
                //cout << "  > Generating int vector of the triples..."; fflush(stdout);
                for (i = 0, it = triple_begin; it != triple_end; it++, i++) {
                    t[3 * i] = std::get<0>(*it);
                    t[3 * i + 1] = std::get<1>(*it) + m_max_s;
                    t[3 * i + 2] = std::get<2>(*it) + m_max_s + m_max_p;
                }
                t[3 * n] = m_max_s + m_max_p + m_max_o + 1;
                t[3 * n + 1] = 0;
                //D.clear();
                //D.shrink_to_fit();
                sdsl::util::bit_compress(t);

                {
                    sdsl::int_vector<> sa;
                    sdsl::qsufsort::construct_sa(sa, t);

                    //cout << "  > Suffix array built " << size_in_bytes(sa) << " bytes" <<  endl;
                    //cout << "  > Building the global BWT" << endl;

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
            //cout << "  > Building m_bwt_o" << endl; fflush(stdout);
            // First O
            {
                sdsl::int_vector<> O(n + 1);
                uint64_t j = 1, c;
                vector<uint64_t> C_O;
                O[0] = 0;
                for (i = 1; i < n; i++)
                    O[j++] = bwt_aux[i] - (m_max_s + m_max_p);

                // This is for making the bwt of triples circular
                O[j] = bwt_aux[0] - (m_max_s + m_max_p);
                sdsl::util::bit_compress(O);

                uint64_t cur_pos = 1;

                C_O.push_back(0); // Dummy value
                C_O.push_back(cur_pos);
                for (c = 2; c <= alphabet_SO; c++) {
                    cur_pos += M_S[c - 1];
                    C_O.push_back(cur_pos);
                }
                C_O.push_back(n + 1);
                C_O.shrink_to_fit();

                M_S.clear();

                // builds the WT for BWT(O)
                m_bwt_o = bwt_type(O, C_O);
            }

            //cout << "  > Building m_bwt_s" << endl; fflush(stdout);
            // Then S
            {
                sdsl::int_vector<> S(n + 1);
                uint64_t j = 1, c;
                vector<uint64_t> C_S;

                S[0] = 0;
                while (i < 2 * n) {
                    S[j++] = bwt_aux[i++];
                }
                sdsl::util::bit_compress(S);

                uint64_t cur_pos = 1;
                C_S.push_back(0);  // Dummy value
                C_S.push_back(cur_pos);
                for (c = 2; c <= m_max_p; c++) {
                    cur_pos += M_P[c - 1];
                    C_S.push_back(cur_pos);
                }
                C_S.push_back(n + 1);
                C_S.shrink_to_fit();

                M_P.clear();

                m_bwt_s = bwt_type(S, C_S);
            }

            //cout << "  > Building m_bwt_p" << endl; fflush(stdout);
            // Then P
            {
                sdsl::int_vector<> P(n + 1);
                uint64_t j = 1, c;
                vector<uint64_t> C_P;

                P[0] = 0;
                while (i < 3 * n) {
                    P[j++] = bwt_aux[i++] - m_max_s;
                }
                sdsl::util::bit_compress(P);

                uint64_t cur_pos = 1;
                C_P.push_back(0);  // Dummy value
                C_P.push_back(cur_pos);
                for (c = 2; c <= alphabet_SO; c++) {
                    cur_pos += M_O[c - 1];
                    C_P.push_back(cur_pos);
                }
                C_P.push_back(n + 1);
                C_P.shrink_to_fit();

                M_O.clear();

                m_bwt_p = bwt_type(P, C_P);
            }
    
            /*std::cout << "bwt_L_o : " << m_bwt_o.get_L() << std::endl;
            std::cout << "bwt_L_p : " << m_bwt_p.get_L() << std::endl;
            std::cout << "bwt_L_s : " << m_bwt_s.get_L() << std::endl;*/
            cout << "-- Index constructed successfully" << endl;
            fflush(stdout);
        };


        //! Copy constructor
        ring(const ring &o) {
            copy(o);
        }

        //! Move constructor
        ring(ring &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        ring &operator=(const ring &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        ring &operator=(ring &&o) {
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
                m_crc_arrays = std::move(o.m_crc_arrays);
            }
            return *this;
        }
        void swap(ring &o) {
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
            std::swap(m_crc_arrays, o.m_crc_arrays);
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
            //Currently crc arrays are stored in a different file.
            //written_bytes += sdsl::write_member(m_crc_arrays, out, child, "crc_arrays");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }
        void store_Ls () const{
            auto L_s = m_bwt_s.get_L();
            auto L_p = m_bwt_p.get_L();
            auto L_o = m_bwt_o.get_L();
            sdsl::store_to_file(L_s, "wm_s");
            sdsl::store_to_file(L_p, "wm_p");
            sdsl::store_to_file(L_o, "wm_o");
        }
        void load(std::istream &in) {
            m_bwt_s.load(in);
            //auto response = sdsl::intersect(m_bwt_s.get_L(), {{307367827,312059634}, {424239487, 427535141}, {427535142, 429206404}, {434197698, 435423606}, {430985497, 433965531}});
            m_bwt_p.load(in);
            m_bwt_o.load(in);
            sdsl::read_member(m_max_s, in);
            sdsl::read_member(m_max_p, in);
            sdsl::read_member(m_max_o, in);
            sdsl::read_member(m_n_triples, in);
            sdsl::read_member(m_sigma_s, in);
            sdsl::read_member(m_sigma_p, in);
            sdsl::read_member(m_sigma_o, in);
            //Currently crc arrays are stored in a different file.
            //m_crc_arrays.load(in);
        }
        void load_crc_arrays(std::string file){
            sdsl::load_from_file(m_crc_arrays, file+".crc");
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

        bwt_interval open_PSO() {
            //return bwt_interval(2 * m_n_triples + 1, 3 * m_n_triples);
            return bwt_interval( 1, m_n_triples);
        }

        /**********************************/
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

        std::vector<uint64_t>
        all_O_in_range(bwt_interval &I) {
            return m_bwt_o.values_in_range(I.left(), I.right());
        }

        /**********************************/
        // Functions for OPS
        //

        bwt_interval open_OPS() {
            return bwt_interval(1, m_n_triples);
        }


        /**********************************/
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


        /**********************************/
        // Function for SOP
        //

        bwt_interval open_SOP() {
            return bwt_interval(1,  m_n_triples);
        }



        /**********************************/
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

        std::vector<uint64_t>
        all_P_in_range(bwt_interval &I) {
            return m_bwt_p.values_in_range(I.left(), I.right());
        }


        /**********************************/
        // Functions for SPO
        //
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
            //std::cout << "SPO forward down_S_P" << std::endl;
            //fw_count++;
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
            //std::cout << "SPO forward min_P_in_S" << std::endl;
            //fw_count++;
            q = m_bwt_s.select_next(1, s_value, m_bwt_o.nElems(s_value));
            uint64_t b = m_bwt_s.bsearch_C(q.first) - 1;
            I.set_stored_values(b, q.second);
            return b;
        }

        uint64_t next_P_in_S(bwt_interval &I, uint64_t s_value, uint64_t p_value) {
            //std::cout << "SPO forward next_P_in_S" << std::endl;
            //fw_count++;
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

        /**********************************/
        // Functions for POS
        //

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
            //std::cout << "SPO forward down_P_O" << std::endl;
            //fw_count++;
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
            //std::cout << "SPO forward min_O_in_P" << std::endl;
            //fw_count++;
            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_p.select_next(1, p_value, m_bwt_s.nElems(p_value));
            uint64_t b = m_bwt_p.bsearch_C(q.first) - 1;
            p_int.set_stored_values(b, q.second);
            return b;
        }

        uint64_t next_O_in_P(bwt_interval &I, uint64_t p_value, uint64_t o_value) {
            //std::cout << "SPO forward next_O_in_P" << std::endl;
            //fw_count++;
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

        /**********************************/
        // Functions for OSP
        //

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
            //std::cout << "SPO forward down_O_S" << std::endl;
            //fw_count++;
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
            //std::cout << "SPO forward min_S_in_O" << std::endl;
            //fw_count++;
            std::pair<uint64_t, uint64_t> q;
            q = m_bwt_o.select_next(1, o_value, m_bwt_p.nElems(o_value));
            uint64_t b = m_bwt_o.bsearch_C(q.first) - 1;
            o_int.set_stored_values(b, q.second);
            return b;
        }

        uint64_t next_S_in_O(bwt_interval &I, uint64_t o_value, uint64_t s_value) {
            //std::cout << "SPO forward next_S_in_O" << std::endl;
            //fw_count++;
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

        bwt_type get_m_bwt_s() const{
            return m_bwt_s;
        }

        bwt_type get_m_bwt_p() const{
            return m_bwt_p;
        }

        bwt_type get_m_bwt_o() const{
            return m_bwt_o;
        }
        std::vector<uint64_t> get_P_given_S(uint64_t symbol_id){
            std::vector<u_int64_t> results;
            //based on ring query debugging. at minimum one needs to call min_P_in_S (1 time) and next_P_in_S (N -1 times).
            auto num_elems = m_bwt_o.nElems(symbol_id);
            bwt_interval aux_i = bwt_interval(symbol_id, num_elems);
            auto current_p = min_P_in_S(aux_i, symbol_id);
            uint64_t old_p = 0;
            results.push_back(current_p);
            for(uint64_t q=0; q < num_elems; q++){
                current_p = next_P_in_S(aux_i, symbol_id, ++current_p);
                if(current_p <= old_p){
                    break;
                }
                old_p = current_p;
                results.push_back(current_p);
            }
            results.shrink_to_fit();
            //std::cout << "range_values: " << results << endl;
            return results;
        }
        //! TODO:
        /*!
        * \returns std::vector<uint64_t>
        */
        std::vector<uint64_t> get_S_given_O(uint64_t symbol_id){
            std::vector<u_int64_t> results;
            //based on ring query debugging. check getPGivenS
            auto num_elems = m_bwt_p.nElems(symbol_id);
            bwt_interval aux_i = bwt_interval(symbol_id, num_elems);
            auto current_p = min_S_in_O(aux_i, symbol_id);
            uint64_t old_p = 0;
            results.push_back(current_p);
            for(uint64_t q=0; q < num_elems; q++){
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
        //! TODO:
        /*!
        * \returns std::vector<uint64_t>
        */
        std::vector<uint64_t> get_O_given_P(uint64_t symbol_id){
            std::vector<u_int64_t> results;
            //based on ring query debugging. check getPGivenS
            auto num_elems = m_bwt_s.nElems(symbol_id);
            bwt_interval aux_i = bwt_interval(symbol_id, num_elems);
            auto current_p = min_O_in_P(aux_i, symbol_id);
            uint64_t old_p = 0;
            results.push_back(current_p);
            for(uint64_t q=0; q < num_elems; q++){
                current_p = next_O_in_P(aux_i, symbol_id, ++current_p);
                if(current_p <= old_p){
                    break;
                }
                old_p = current_p;
                results.push_back(current_p);
            }
            results.shrink_to_fit();
            return results;
        }
        //CRC arrays wrapper functions
        value_type get_number_distinct_values_spo_BWT_S(value_type l, value_type r) const{
            return m_crc_arrays.get_number_distinct_values_spo_BWT_S(l, r);
        }
        value_type get_number_distinct_values_spo_BWT_P(value_type l, value_type r) const{
            return m_crc_arrays.get_number_distinct_values_spo_BWT_P(l, r);
        }
        value_type get_number_distinct_values_spo_BWT_O(value_type l, value_type r) const{
            return m_crc_arrays.get_number_distinct_values_spo_BWT_O(l, r);
        }
        value_type get_number_distinct_values_sop_BWT_S(value_type l, value_type r) const{
            return m_crc_arrays.get_number_distinct_values_sop_BWT_S(l, r);
        }
        value_type get_number_distinct_values_sop_BWT_P(value_type l, value_type r) const{
            return m_crc_arrays.get_number_distinct_values_sop_BWT_P(l, r);
        }
        value_type get_number_distinct_values_sop_BWT_O(value_type l, value_type r) const{
            return m_crc_arrays.get_number_distinct_values_sop_BWT_O(l, r);
        }
        size_type get_sigma_s() const{
            return m_sigma_s;
        }
        size_type get_sigma_p() const{
            return m_sigma_p;
        }
        size_type get_sigma_o() const{
            return m_sigma_o;
        }
        void clear_cache(){
            m_crc_arrays.clear_cache();
        }
    };

    typedef ring<bwt<sdsl::rrr_vector<15>>> c_ring;

}

#endif
