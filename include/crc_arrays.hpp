/*
 * crc_arrays.hpp
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

#ifndef CRC_ARRAYS
#define CRC_ARRAYS

#include "configuration.hpp"
#include <exception> // std::exception
#include <unordered_map>
#include "crc_array.hpp"

namespace ring {
    template <class bwt_bit_vector_t = sdsl::bit_vector>//Is this needed?
    class crc_arrays
    {
    public:
        typedef uint64_t value_type;
        typedef uint64_t size_type;
        typedef sdsl::wm_int<bwt_bit_vector_t> wm_type; //TODO: no deberia ser typedef sdsl::wm_int<wm_bit_vector_t> y wm_type?
    private:
        void copy(const crc_arrays &o) {
            spo_BWT_S = std::move(o.spo_BWT_S);
            spo_BWT_P = std::move(o.spo_BWT_P);
            spo_BWT_O = std::move(o.spo_BWT_O);

            sop_BWT_S = std::move(o.sop_BWT_S);
            sop_BWT_O = std::move(o.sop_BWT_O);
            sop_BWT_P = std::move(o.sop_BWT_P);
        }
    public:
        std::unique_ptr<crc<>> spo_BWT_S;
        std::unique_ptr<crc<>> spo_BWT_P;
        std::unique_ptr<crc<>> spo_BWT_O;

        std::unique_ptr<crc<>> sop_BWT_S;
        std::unique_ptr<crc<>> sop_BWT_P;
        std::unique_ptr<crc<>> sop_BWT_O;
        crc_arrays() = default;
        void build_spo_arrays(const wm_type &spo_bwt_s_L,const wm_type &spo_bwt_p_L,const wm_type &spo_bwt_o_L)
        {
            spo_BWT_S = std::move(std::unique_ptr<crc<>>{new crc<>(spo_bwt_s_L)});
            spo_BWT_P = std::move(std::unique_ptr<crc<>>{new crc<>(spo_bwt_p_L)});
            spo_BWT_O = std::move(std::unique_ptr<crc<>>{new crc<>(spo_bwt_o_L)});
        }
        void build_sop_arrays(const wm_type &sop_bwt_s_L,const wm_type &sop_bwt_o_L,const wm_type &sop_bwt_p_L)
        {
            sop_BWT_S = std::move(std::unique_ptr<crc<>>{new crc<>(sop_bwt_s_L)});
            sop_BWT_O = std::move(std::unique_ptr<crc<>>{new crc<>(sop_bwt_o_L)});
            sop_BWT_P = std::move(std::unique_ptr<crc<>>{new crc<>(sop_bwt_p_L)});
        }
        void print_arrays()
        {
            std::cout << "spo S : "<< std::endl;
            spo_BWT_S->print();
            std::cout << "spo P : "<< std::endl;
            spo_BWT_P->print();
            std::cout << "spo O: "<< std::endl;
            spo_BWT_O->print();


            std::cout << "sop S : "<< std::endl;
            sop_BWT_S->print();
            std::cout << "sop P : "<< std::endl;
            sop_BWT_P->print();
            std::cout << "sop O : "<< std::endl;
            sop_BWT_O->print();
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_spo_BWT_S(value_type l, value_type r) const
        {
            return spo_BWT_S->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_spo_BWT_P(value_type l, value_type r) const
        {
            return spo_BWT_P->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_spo_BWT_O(value_type l, value_type r) const
        {
            return spo_BWT_O->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_sop_BWT_S(value_type l, value_type r) const
        {
            return sop_BWT_S->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_sop_BWT_P(value_type l, value_type r) const
        {
            return sop_BWT_P->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_sop_BWT_O(value_type l, value_type r) const
        {
            return sop_BWT_O->get_number_distinct_values(l, r);
        }
        //! Copy constructor
        crc_arrays(const crc_arrays &o) {
            copy(o);
        }

        //! Move constructor
        crc_arrays(crc_arrays &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        crc_arrays &operator=(const crc_arrays &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        crc_arrays &operator=(crc_arrays &&o) {
            if (this != &o) {
                spo_BWT_S = std::move(o.spo_BWT_S);
                spo_BWT_P = std::move(o.spo_BWT_P);
                spo_BWT_O = std::move(o.spo_BWT_O);

                sop_BWT_S = std::move(o.sop_BWT_S);
                sop_BWT_O = std::move(o.sop_BWT_O);
                sop_BWT_P = std::move(o.sop_BWT_P);
            }
            return *this;
        }

        void swap(crc_arrays &o) {
            std::swap(spo_BWT_S, o.spo_BWT_S);
            std::swap(spo_BWT_P, o.spo_BWT_P);
            std::swap(spo_BWT_O, o.spo_BWT_O);

            std::swap(sop_BWT_S, o.sop_BWT_S);
            std::swap(sop_BWT_O, o.sop_BWT_O);
            std::swap(sop_BWT_P, o.sop_BWT_P);
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            //SPO Ring : The following two if statements must exists otherwise a call to 'serialize' via 'size_in_bytes' will trigger a segfault if is done while SOP ring is not built yet.
            if(spo_BWT_S && spo_BWT_P && spo_BWT_O){
                written_bytes += spo_BWT_S->serialize(out, child, "spo_BWT_S");
                written_bytes += spo_BWT_P->serialize(out, child, "spo_BWT_P");
                written_bytes += spo_BWT_O->serialize(out, child, "spo_BWT_O");
            }
            //SOP (Reverse Ring)
            if(sop_BWT_S && sop_BWT_P && sop_BWT_O){
                written_bytes += sop_BWT_S->serialize(out, child, "sop_BWT_S");
                written_bytes += sop_BWT_O->serialize(out, child, "sop_BWT_O");
                written_bytes += sop_BWT_P->serialize(out, child, "sop_BWT_P");
            }
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream &in) {
            spo_BWT_S = std::unique_ptr<crc<>>{new crc<>};
            spo_BWT_P = std::unique_ptr<crc<>>{new crc<>};
            spo_BWT_O = std::unique_ptr<crc<>>{new crc<>};
            sop_BWT_S = std::unique_ptr<crc<>>{new crc<>};
            sop_BWT_O = std::unique_ptr<crc<>>{new crc<>};
            sop_BWT_P = std::unique_ptr<crc<>>{new crc<>};

            spo_BWT_S->load(in);
            spo_BWT_P->load(in);
            spo_BWT_O->load(in);

            sop_BWT_S->load(in);
            sop_BWT_O->load(in);
            sop_BWT_P->load(in);
        }
    };
}
#endif
