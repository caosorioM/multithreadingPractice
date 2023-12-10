#pragma once
#include <cstdint>
#include <array>
#include <stddef.h>

template<typename node_t , size_t numberOfNodes>
class stampedNodesArray
{
public:
    struct nodeTag  
    {
        uint32_t index;
        uint32_t stamp;
    };

    node_t* GetNode(nodeTag tag)
    {
        return &_array[tag.index];
    }
private:
    std::array<node_t , numberOfNodes> _array;
};