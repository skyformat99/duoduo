#ifndef DUO_BLOCK_STRUCTURE_BIG_H_
#define DUO_BLOCK_STRUCTURE_BIG_H_

#include <string>
#include "../typedef.h"
#include "BlockStructure.h"

namespace DuoDuo
{
    class BlockStructure_Big : public BlockStructure
    {
    public:
        BlockStructure_Big(void) : BlockStructure(BlockStructure::eBlockType_Big) {}
    };
}

#endif
