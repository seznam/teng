/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 *
 * DESCRIPTION
 * Teng processor funcction (like len, round or formatDate)
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (jan)
 *             Created.
 */

#ifndef TENGUDF_H
#define TENGUDF_H

#include <vector>
#include <string>
#include <tr1/functional>
#include "tengconfig.h"

namespace Teng {

/**
 * Class for passing argument to/from user definbed functions
 */
class UDFValue_t {
    public:
        enum  UDFType_t {
            Integer, ///< Integer type
            Real, ///< Real type
            String ///< String type
        };

        /// Constructs value as integer
        UDFValue_t(IntType_t i)
        : m_type(Integer), m_iValue(i) {};

        /// Constructs value as double
        UDFValue_t(double d)
        : m_type(Real), m_fValue(d) {};

        /// Constructs value as string
        UDFValue_t(const std::string &s)
        : m_type(String), m_sValue(s) {};

        /**
         * @returns integer value
        */
        IntType_t getInt() const {
            return m_iValue;
        }

        /**
         * @returns real value
        */
        double getReal() const {
            return m_fValue;
        }

        /**
         * @returns string value
        */
        const std::string &getString() const {
            return m_sValue;
        }

        /**
         * @returns value type @see UDFType_t
        */
        int getType() const {
            return m_type;
        }

        /// sets integer value
        void setInt(IntType_t i) {
            m_type = Integer;
            m_iValue = i;
        }

        /// sets real value
        void setReal(double d) {
            m_type = Real;
            m_fValue = d;
        }

        /// sets string value
        void setString(const std::string &s) {
            m_type = String;
            m_sValue = s;
        }
    protected:
        int m_type;
        std::string m_sValue;
        union {
            double m_fValue;
            IntType_t m_iValue;
        };

        UDFValue_t()
        : m_type(Integer), m_iValue(0) {};
};

typedef enum {E_OK = 0, E_ARGS = -1, E_OTHER = -2} UDF_Status_t;
typedef std::tr1::function<UDFValue_t (const std::vector<UDFValue_t> &)> UDFCallback_t;

/**
 * @short registers user-defined function
 * @param name name of the function
 * @param udf user-defined callable object
 */
void registerUDF(const std::string &name, UDFCallback_t udf);

/**
 * @short finds function in global UDF list, returns pointer or 0
 * @param name name of the function
 */
UDFCallback_t findUDF(const std::string &name);

}

#endif