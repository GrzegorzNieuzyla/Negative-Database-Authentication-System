#pragma once
#include <vector>
#include <cassert>
#include <algorithm>

class Permutation
{

public:
    explicit Permutation(std::vector<int> permuted)
    : _permuted(std::move(permuted))
    {
        assert(std::all_of(_permuted.begin(), _permuted.end(), [&](int i){return i >=0 && i < _permuted.size();}));
        _original.resize(_permuted.size());
        for (int i = 0; i < _permuted.size(); ++i)
        {
            assert(_permuted[i] >= 0 && _permuted[i] < _permuted.size());
            _original[_permuted[i]] = i;
        }
    }

    static Permutation TrivialPermutation(int length)
    {
        std::vector<int> indices;
        indices.resize(length);
        for (int i = 0; i < length; ++i)
        {
            indices[i] = i;
        }
        return Permutation(indices);
    }

    template <class T> std::vector<T> Apply(const std::vector<T>& vector) const
    {
        assert(vector.size() == _permuted.size());
        std::vector<T> result;
        result.resize(vector.size());
        for (int i = 0; i < vector.size(); ++i)
        {
            result[_permuted[i]] = vector[i];
        }
        return result;
    }

    template <class T> std::vector<T> Inverse(const std::vector<T>& vector) const
    {
        assert(vector.size() == _permuted.size());
        std::vector<T> result;
        result.resize(vector.size());
        for (int i = 0; i < vector.size(); ++i)
        {
            result[_original[i]] = vector[i];
        }
        return result;
    }

    std::string Inverse(const std::string& string) const
    {
        assert(string.size() == _permuted.size());
        std::string result;
        result.resize(string.size());
        for (int i = 0; i < string.size(); ++i)
        {
            result[_original[i]] = string[i];
        }
        return result;
    }

    template <class T> T& GetItem(std::vector<T> vector, int index) const
    {
        return vector[_permuted[index]];
    }

    template <class T> T& GetItemInverse(std::vector<T> vector, int index) const
    {
        return vector[_original[index]];
    }

    int GetIndex(int index) const
    {
        return _permuted[index];
    }

    int GetInverseIndex(int index)
    {
        return _original[index];
    }

private:
    std::vector<int> _permuted;
    std::vector<int> _original;
};



