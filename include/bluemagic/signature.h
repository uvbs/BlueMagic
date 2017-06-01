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
        Signature(std::string signature)
        {
            String = signature;
            String.erase(std::remove_if(String.begin(), String.end(), ::isspace), String.end());
            std::transform(String.begin(), String.end(), String.begin(), ::tolower);
            StringSize = String.size();
            if (StringSize % 2 != 0)
                throw std::invalid_argument("Signature contains an invalid number of characters.");

            bool wildcard = false;
            for (char c : String)
            {
                if ((c < '0' || c > '9') &&
                    (c < 'a' || c > 'f') &&
                    (c != '?'))
                    throw std::invalid_argument("Signature contains invalid character(s).");

                if (c == '?')
                    wildcard = true;
            }

            if (!wildcard)
            {
                BytesSize = StringSize / 2;
                Bytes.resize(BytesSize);
                for (SIZE_T i = 0, j = 0; j < StringSize; ++i, j += 2)
                    Bytes[i] = std::stoul(String.substr(j, 2), nullptr, 16);
            }
        }

        std::string String;
        SIZE_T StringSize;
        std::vector<BYTE> Bytes;
        SIZE_T BytesSize;
};

}