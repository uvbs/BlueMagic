#pragma once
#pragma warning(disable:4244)

#include <windows.h>
#include <algorithm>
#include <string>
#include <vector>

namespace bluemagic
{

class Signature
{
public:
    Signature() : _string{ std::string() }, _bytes{ std::vector<BYTE>() }
    {
    }

    Signature(std::string signature) : _string{ signature }, _bytes{ std::vector<BYTE>() }
    {
        _string.erase(std::remove_if(_string.begin(), _string.end(), ::isspace), _string.end());
        std::transform(_string.begin(), _string.end(), _string.begin(), ::tolower);
        if (_string.size() % 2 != 0)
            throw std::invalid_argument("Signature contains an invalid number of characters.");

        bool wildcard = false;
        for (char c : _string)
        {
            if ((c < '0' || c > '9') &&
                (c < 'a' || c > 'f') &&
                (c != '?'))
                throw std::invalid_argument("Signature contains invalid characters.");

            if (c == '?')
                wildcard = true;
        }

        if (!wildcard)
        {
            _bytes.resize(_string.size() / 2);
            for (SIZE_T i = 0, j = 0; j < _string.size(); ++i, j += 2)
                _bytes[i] = std::stoul(_string.substr(j, 2), nullptr, 16);
        }
    }

    Signature(std::vector<BYTE> signature) : _string{ std::string() }, _bytes{ signature }
    {
        _string.resize(_bytes.size() * 2);
        for (SIZE_T i = 0, j = 0; i < _bytes.size(), j < _string.size(); ++i, j += 2)
            sprintf_s(&_string[j], 4, "%02x", _bytes[i]);
    }

    std::string ToString() const
    {
        return _string;
    }

    std::vector<BYTE> ToBytes() const
    {
        return _bytes;
    }

private:
    std::string _string;
    std::vector<BYTE> _bytes;
};

}
