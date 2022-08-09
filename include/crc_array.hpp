/*
 * crc_array.hpp
 * Copyright (C) 2022, Fabrizio
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

#ifndef CRC_ARRAY
#define CRC_ARRAY

#include "configuration.hpp"
#include <exception> // std::exception
#include <unordered_map>


namespace ring {

    template <class bwt_bit_vector_t = bit_vector>//<class wm_bit_vector_t = bit_vector>
    class crc{
        public:
            typedef int_vector<>::size_type size_type;
            typedef int_vector<>::value_type value_type;
            typedef sdsl::wm_int<bwt_bit_vector_t> wm_type; //TODO: no deberia ser typedef sdsl::wm_int<wm_bit_vector_t> y wm_type?
        private:
            wm_type m_L;//Reference to BWT's L, useful for crc WM creation.
            wm_type m_crc_L;

            void copy(const crc &o) {
                m_L = o.m_L;
                m_crc_L = o.m_crc_L;
            }
            //! Buiding Colored Range Counting Array (CRC WM) based on the given Wavelet Matrix.
            /*!
            * \returns boolean depending whether the CRC WM is created successfully or not. If true then the CRC WM are available as the 'crc_L' member.
            */
            bool build_crc_wm(value_type x_s, value_type x_e){
                sdsl::int_vector<> C(x_e - x_s);
                //std::cout << "L.sigma : " << m_L.sigma << ", L.size() : " << m_L.size() << " x_s : " << x_s << " x_e : " << x_e << std::endl;
                //std::cout << "Building int vector to store CRC (size = " << C.size() << ")." << std::endl;
                // O ( (x_e - x_s) * log sigma)
                //  CORE >>
                {
                    std::unordered_map<value_type, value_type> hash_map;
                    for (value_type i = x_s; i < x_e; i++)
                    {
                        auto it = hash_map.find(m_L[i]);
                        if (it == hash_map.end())
                        {
                            hash_map.insert({m_L[i], i});
                            // C positions must start from 0 until x_e - x_s.
                            C[i - x_s] = 0;
                            // std::cout << C[i - x_s] << " ";fflush(stdout);
                        }
                        else
                        {
                            C[i - x_s] = it->second + 1;
                            it->second = i;
                            // std::cout << C[i - x_s] << " ";fflush(stdout);
                        }
                    }
                }
                // CORE <<
                //std::cout << "C = " << C << std::endl;
                //std::cout << "Building the CRC WM based on the CRC int vector." << std::endl;
                construct_im(m_crc_L, C);
                try
                {
                    //std::cout << "CRC WM size : " << crc_L.size() << std::endl;
                    if (m_crc_L.size() > 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                catch (std::exception &e)
                {
                    return false;
                }
            }
        public:
            crc() = default;
            crc(const wm_type &wm_l, bool build_index = true){
                m_L = wm_l;
                if (build_index)
                {
                    build_crc_wm(0, m_L.size() - 1);
                }
            }
            value_type get_number_distinct_values_on_range(value_type x_s, value_type x_e, value_type rng_s, value_type rng_e){
                return m_crc_L.count_range_search_2d(x_s, x_e, rng_s, rng_e);
            }
            //! Calculates the number of different values on range [l,r).
            /*!
            * \param value_type l : left
            * \param value_type r : right
            * \returns number of distinct values on range [l,r) the WMs.
            */
            value_type get_number_distinct_values(value_type l, value_type r){
                assert(l > 0);
                // std::cout << "Calling get_number_distinct_values with range : [" << l << ", " << r << "]." << std::endl;
                value_type num_dist_values = 0;
                // Build the crc wm for the entire original WT
                //bool result = build_crc_wm(l, r);
                value_type rng_s = 0;
                value_type rng_e = (l == 0) ? 0 : l - 1;

                num_dist_values = get_number_distinct_values_on_range(l, r, rng_s, rng_e);
                //num_dist_values = get_number_distinct_values_on_range(0, crc_L.size() - 1, 0, 0);
                //std::cout << "Num of distinct values : " << num_dist_values << std::endl;
                return num_dist_values;
            }
            void print(){
                std::cout << m_crc_L << std::endl;
            }

            //! Copy constructor
            crc(const crc &o) {
                copy(o);
            }

            //! Move constructor
            crc(crc &&o) {
                *this = std::move(o);
            }

            //! Copy Operator=
            crc &operator=(const crc &o) {
                if (this != &o) {
                    copy(o);
                }
                return *this;
            }

            //! Move Operator=
            crc &operator=(crc &&o) {
                if (this != &o) {
                    m_L = std::move(o.m_L);
                    m_crc_L = std::move(o.m_crc_L);
                }
                return *this;
            }

            void swap(crc &o) {
                // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
                std::swap(m_L, o.m_L);
                std::swap(m_crc_L, o.m_crc_L);
            }

            //! Serializes the data structure into the given ostream
            size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
                sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
                size_type written_bytes = 0;
                written_bytes += m_crc_L.serialize(out, child, "crc_L");
                sdsl::structure_tree::add_size(child, written_bytes);
                return written_bytes;
            }

            void load(std::istream &in) {
                m_crc_L.load(in);
            }
    };
}
#endif
