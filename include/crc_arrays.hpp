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
#include "ring.hpp" //SPO order
#include "reverse_ring.hpp" //SOP order
#include "crc_array.hpp"
#include "bwt.hpp"

namespace ring {
    template <class bwt_t = bwt<>>//Is this needed?
    class crc_arrays
    {
    public:
        typedef uint64_t value_type;
        typedef ring<> ring_spo;
        typedef reverse_ring<> ring_sop;
        //typedef sdsl::wm_int<bwt_bit_vector_t> bwt_type;//Is this needed?
        typedef bwt_t bwt_type;
    private:
        ring_spo r_spo;
        ring_sop r_sop;

    public:
        unique_ptr<crc<>> spo_BWT_S;
        unique_ptr<crc<>> spo_BWT_P;
        unique_ptr<crc<>> spo_BWT_O;

        unique_ptr<crc<>> sop_BWT_S;
        unique_ptr<crc<>> sop_BWT_P;
        unique_ptr<crc<>> sop_BWT_O;

        void build_spo_arrays(ring_spo &spo)
        {
            r_spo = spo;
            spo_BWT_S = std::make_unique<crc<>>((r_spo.get_m_bwt_s()).get_L());
            spo_BWT_P = std::make_unique<crc<>>((r_spo.get_m_bwt_p()).get_L());
            spo_BWT_O = std::make_unique<crc<>>((r_spo.get_m_bwt_o()).get_L());
        }
        void build_sop_arrays(ring_sop &sop)
        {
            r_sop = sop;
            sop_BWT_S = std::make_unique<crc<>>((r_sop.get_m_bwt_s()).get_L());
            sop_BWT_O = std::make_unique<crc<>>((r_sop.get_m_bwt_o()).get_L());
            sop_BWT_P = std::make_unique<crc<>>((r_sop.get_m_bwt_p()).get_L());
        }
        void save_spo(string filename)
        {
            spo_BWT_S->save(filename + "_spo_crc_S");
            spo_BWT_P->save(filename + "_spo_crc_P");
            spo_BWT_O->save(filename + "_spo_crc_O");
        }
        void save_sop(string filename)
        {
            sop_BWT_S->save(filename + "_sop_crc_S");
            sop_BWT_O->save(filename + "_sop_crc_O");
            sop_BWT_P->save(filename + "_sop_crc_P");
        }
        void load(string filename)
        {
            spo_BWT_S = std::make_unique<crc<>>();
            spo_BWT_P = std::make_unique<crc<>>();
            spo_BWT_O = std::make_unique<crc<>>();

            sop_BWT_S = std::make_unique<crc<>>();
            sop_BWT_P = std::make_unique<crc<>>();
            sop_BWT_O = std::make_unique<crc<>>();

            spo_BWT_S->load(filename + "_spo_crc_S");
            spo_BWT_P->load(filename + "_spo_crc_P");
            spo_BWT_O->load(filename + "_spo_crc_O");

            sop_BWT_S->load(filename + "_sop_crc_S");
            sop_BWT_P->load(filename + "_sop_crc_P");
            sop_BWT_O->load(filename + "_sop_crc_O");
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
        value_type get_number_distinct_values_spo_BWT_S(value_type l, value_type r)
        {
            return spo_BWT_S->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_spo_BWT_P(value_type l, value_type r)
        {
            return spo_BWT_P->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_spo_BWT_O(value_type l, value_type r)
        {
            return spo_BWT_O->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_sop_BWT_S(value_type l, value_type r)
        {
            return sop_BWT_S->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_sop_BWT_P(value_type l, value_type r)
        {
            return sop_BWT_P->get_number_distinct_values(l, r);
        }
        //! Gets the number of distinct valuesfor a specific BWT.
        /*!
        * \param l : left value of the range.
        * \param r : right value of the range.
        * \returns value_type
        */
        value_type get_number_distinct_values_sop_BWT_O(value_type l, value_type r)
        {
            return sop_BWT_O->get_number_distinct_values(l, r);
        }
    };
}
#endif