#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdlib.h>
#include "assemble_st.h"

extern word_t dp_assembler(ass_t *assem);
extern word_t branch_assembler(ass_t *assem);
extern word_t mul_assembler(ass_t *assem);
extern word_t sdt_assembler(ass_t *assem);

#endif