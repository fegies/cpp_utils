#pragma once

/**
 * This header contains an implemntation of a mathematical vector generic over the number type
 *
 * For the license see the end of the file
 */

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <utility>

namespace linalg
{
template <class T>
class Vector
{
public:
    std::vector<T> inner;

    template <typename O>
    Vector _binop(const Vector<T> &rhs, O op) const
    {
        if (this->inner.size() != rhs.inner.size())
            throw "Dimension error";

        std::vector<T> result_inner;
        result_inner.reserve(this->inner.size());

        std::transform(
            this->inner.begin(),
            this->inner.end(),
            rhs.inner.begin(),
            std::back_inserter(result_inner),
            op);
        return Vector(std::move(result_inner));
    }


    Vector(std::vector<T> &&inner) : inner(inner) {}

    template <typename F>
    Vector(std::initializer_list<F> items) : inner()
    {
        inner.reserve(items.size());
        for (auto item : items)
            inner.emplace_back(item);
    }

    size_t dimension() const
    {
        return this->inner.size();
    }

    Vector componentwise_multiplication(const Vector<T> &other) const
    {
        return this->_binop(other, [](T a, T b) { return a * b; });
    }

    Vector operator-(const Vector<T> &rhs) const
    {
        return this->_binop(rhs, [](T a, T b) { return a - b; });
    }

    Vector operator+(const Vector<T> &rhs) const
    {
        return this->_binop(rhs, [](T a, T b) { return a + b; });
    }

    T scalar_product(const Vector<T> &rhs) const
    {
        if (this->inner.size() != rhs.inner.size())
            throw "Dimension error on scalar multiplication";

        T res = std::inner_product(this->inner.begin(), this->inner.end(), rhs.inner.begin(), T());
        return res;
    }

    void operator+=(const Vector<T> &rhs)
    {
        if (this->inner.size() != rhs.inner.size())
            throw "Dimension error on +=";
        std::transform(this->inner.begin(),
                       this->inner.end(),
                       rhs.inner.begin(),
                       this->inner.begin(), [](T a, T b) { return a + b; });
    }

    void operator*=(const T &rhs)
    {
        std::for_each(this->inner.begin(), this->inner.end(),
                      [&](T &v) { v *= rhs; });
    }

    friend Vector operator*(const T lhs, const Vector<T> &rhs)
    {
        std::vector<T> result_inner;
        result_inner.reserve(rhs.inner.size());

        std::transform(rhs.inner.begin(),
                       rhs.inner.end(),
                       std::back_inserter(result_inner),
                       [&](T a) { return lhs * a; });

        return Vector(std::move(result_inner));
    }

    friend std::ostream &operator<<(std::ostream &out, const Vector<T> &vec)
    {
        out << '(';
        auto it = vec.inner.begin();
        out << *it++;
        for (; it != vec.inner.end(); ++it)
        {
            out << ',';
            out << *it;
        }
        out << ')';
        return out;
    }
};
} // namespace linalg

/*
Copyright 2020 Felix Giese

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

