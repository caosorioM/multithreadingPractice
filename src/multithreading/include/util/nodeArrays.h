#pragma once
#include <cstdint>
#include <array>
#include <stddef.h>


typedef uint64_t nodeTag;

template<typename node_t , size_t numberOfNodes>
class stampedNodesArray
{
public:

    node_t* GetNode(nodeTag tag)
    {
        return &_array[tag.index];
    }

     node_t* GetNode(uint32_t tag)
    {
        return &_array[tag.index];
    }

private:
    std::array<node_t , numberOfNodes> _array;
};

bool IsNull(nodeTag tag);

bool IsFastPathEnabled(nodeTag tag);

nodeTag EnableFastPath( nodeTag tag);
nodeTag DisableFastPath( nodeTag tag);

template<typename node_t>
nodeTag CreateNextTag(nodeTag predecesor , node_t* succesorNode);