#include "parser.h"
#include "assembler.h"

// STATIC HELPER METHOD DECLARATIONS FOR PARSER.C
static opcode_t mnemonic_to_op (mne_t);
static cond_t mnemonic_to_cond (mne_t);
static byte_t regbits(char*);
static byte_t exprbits(char*);
static void shiftexpr_op2(ass_t*, char*);
static void shiftreg_op2(ass_t*, tokens_t*);
static char* substring(const char*, int, int);

//Parses DATA PROCESSING Type instructions
void dp_parser(ass_t *assem, tokens_t *tokens) {
    opcode_t op = mnemonic_to_op(assem->mn_type);
    assem->op = op;
    assem->cond = 0xE;

    //ANDEQ instruction from SPL received
    if(assem->mn_type == ANDEQm) {
        op = AND;
        assem->cond = 0x0;
    }
    char *op2;
    //Switching instructions on the basis of opcodes
    switch (op)
    {
        case AND:
        case EOR:
        case SUB:
        case RSB:
        case ADD:
        case ORR:
            op2 = tokens->array_tokens[3];
            assem->rd = regbits(tokens->array_tokens[1]);
            assem->rn = regbits(tokens->array_tokens[2]);
            if(op2[0] == '#') {
                assem->i = 1;
                //substr to rem #
                shiftexpr_op2(assem, &op2[1]);
            }
            else {
                assem->i = 0;
                assem->op2 = regbits(op2);
            }
            break;
        case TST:
        case TEQ:
        case CMP:
            op2 = tokens->array_tokens[2];
            assem->sl = 1;
            assem->rn = regbits(tokens->array_tokens[1]);
            if(op2[0] == '#') {
                assem->i = 1;
                assem->rn = regbits(tokens->array_tokens[1]);
                shiftexpr_op2(assem, op2 + 1);
            } else {
                assem->i = 0;
                assem->op2 = regbits(op2);
            }
        break;   
        case MOV:
            op2 = tokens->array_tokens[2];
            assem->sl = 0;
            assem->rd = regbits(tokens->array_tokens[1]);
            //If the third token in tokens array starts with an expression
            if(op2[0] == '#') {
                assem->i = 1;
                shiftexpr_op2(assem, &op2[1]);
            }
            //If the third token in tokens array starts with a register
            else if (op2[0] == 'r') {
                if(tokens->size > 3) {
                // rd shiftname #0x1f
                    tokens->size = 3;
                    tokens->array_tokens[0] = tokens->array_tokens[1];
                    tokens->array_tokens[1] = tokens->array_tokens[3];
                    tokens->array_tokens[2] = tokens->array_tokens[4];
                    shiftreg_op2(assem, tokens);
                }
                else {
                    if(strstr(tokens->array_tokens[2], "0x")) {
                        assem->op2 = exprbits(tokens->array_tokens[2]);
                    }
                    else if(strstr(tokens->array_tokens[2], "r")) {
                        assem->op2 = regbits(tokens->array_tokens[2]);
                    }
                }
            }
    }
}

//Parses MULTIPLY Type instructions
void mul_parser(ass_t *assem, tokens_t *tokens) {
    assem->inst_type = MUL;
    assem->cond = 0xE;
    assem->sl = 0;
    assem->rd = regbits(tokens->array_tokens[1]);
    assem->rm = regbits(tokens->array_tokens[2]);
    assem->rs = regbits(tokens->array_tokens[3]);
    // MUL instruction
    if(assem->mn_type == MULm) {
        assem->ua = 0;
    }
    // MLA instruction
    else {
        assem->ua = 1;
        assem->rn = regbits(tokens->array_tokens[4]);
    }
}

//Parses SINGLE DATA PROCESSING Type instructions
void sdt_parser(ass_t *assem, tokens_t *tokens, word_arr_t *more_words, int curr_line, int total_lines) {
    assem->rd = regbits(tokens->array_tokens[1]);
    if(assem->mn_type == LDRm) {
        if(tokens->array_tokens[2][0] == '=') {
            word_t cons = strtol(&tokens->array_tokens[2][1], (char **) NULL, 16);
            if(cons <= 0xFF) {
                assem->inst_type = DP;
                assem->op = MOV;
                assem->mn_type = MOVm;
                strcpy(tokens->array_tokens[0], "mov");
                tokens->array_tokens[1] = tokens->array_tokens[1];
                tokens->array_tokens[2][0] = '#';

                dp_parser(assem, tokens);
            }
            // Large numbers which need to be stored as 32-bit words and added to extra words array 
            else {
                assem->sl = 1;
                assem->cond = 0xE;
                assem->i = 0;
                assem->p = 1;
                assem->ua = 1;
                assem->rn = PC;
                assem->offset = ((total_lines + more_words->size - curr_line) << 2) - PIPELINE_OFFSET;
                add_word_arr(more_words, strtol(&tokens->array_tokens[2][1], (char **)NULL, 16));
            }
        }
    } 
    //Store type instructions
    else if(assem->mn_type == STRm) {
        if(tokens->size > 3) {
            if(strstr(tokens->array_tokens[3], "r")) {
                assem->i = 1;
            }
        }
        else if(strstr(tokens->array_tokens[2], "r")) {
                assem->i = 0;
        }
        assem->cond = 0xE;
        assem->sl = 0;
        assem->ua = 1;
    }

    if (tokens->array_tokens[2][0] == '[') {
        assem->cond = 0xE;
        assem->p = 1;
        if (assem->mn_type == LDRm) {
            assem->sl = 1;
        } else {
            assem->sl = 0;
        }
        // optional case includes subtraction of #exprs
        assem->ua = 1; 
        //pre-indexed
        if(tokens->size == 3) {
            //[rn] zero offset pre
            assem->offset = 0;
            char* reg = substring(tokens->array_tokens[2], 1, strlen(tokens->array_tokens[2]) - 1);
            assem->rn = regbits(reg);
        }
        else if (tokens->array_tokens[tokens->size - 1][strlen(tokens->array_tokens[tokens->size - 1]) - 1] == ']') {
            //pre
            if (tokens->array_tokens[3][1] == '-') {
                //its a negative number
                assem->ua = 0;
                char* expr = substring(tokens->array_tokens[3], 0, strlen(tokens->array_tokens[3]) - 1);
                expr++;
                *expr = '#';
                assem->offset = exprbits(expr); 
            } else {
                char* expr = substring(tokens->array_tokens[3], 0, strlen(tokens->array_tokens[3]) - 1);
                if(expr[0] == 'r') {
                    assem->offset = regbits(expr);
                } else {
                    assem->offset = exprbits(expr); 
                }
            }
            char* reg = substring(tokens->array_tokens[2], 1, strlen(tokens->array_tokens[2]));
            assem->rn = regbits(reg);       
        }
        else if (tokens->array_tokens[2][strlen(tokens->array_tokens[2]) - 1] == ']') {
            //post
            assem->p = 0;
            char* reg = substring(tokens->array_tokens[2], 1, strlen(tokens->array_tokens[2]) - 1);
            assem->rn = regbits(reg);
            //printf("\n Getting inside post - %s", tokens->array_tokens[3]);
            if(tokens->array_tokens[3][0] == 'r') {
                assem->offset = regbits(tokens->array_tokens[3]);
            }
            else {
                assem->offset = exprbits(tokens->array_tokens[3]);
            }
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
}

//Parses BRANCH Type instructions
void branch_parser(ass_t *assem, tokens_t *tokens, label_map *map, word_t addr) {
    assem->cond = mnemonic_to_cond(assem->mn_type);
    char *expr = tokens->array_tokens[1];
    word_t off = 0;
    if (*expr != '#') {
        // can check for colon for label confirmation
        char *label = strcat(expr, ":");
        off = lookup(map, label);
    } 
    else {
        expr++;
        //check if hex and make address accordingly
        if (*(expr) == '0' && *(expr + 1) == 'x') {
            off = strtol(expr, NULL, 16);
        } 
        else {
            off = strtol(expr, NULL, 10);
        }
    }
    off = off - addr - PIPELINE_OFFSET;
    off &= 0x3FFFFFF;
    assem->branch_offset = off >> 2;
}

//Parses SPECIAL Type - LSL and ANDEQ instructions
void special_parser(ass_t *assem, tokens_t *tokens) {
    assem->inst_type = DP;
    if(assem->mn_type == ANDEQm) {
        dp_parser(assem, tokens);
    }
    else {
        assem->mn_type = MOVm;
        tokens->size = 5;
        tokens->array_tokens[4] = tokens->array_tokens[2];
        tokens->array_tokens[3] = tokens->array_tokens[0];
        tokens->array_tokens[2] = tokens->array_tokens[1];
        tokens->array_tokens[0] = "mov";
        dp_parser(assem, tokens);
    }
}
//Accepts MNEMONIC TYPE as a parameter and returns respective CONDITION CODE
static cond_t mnemonic_to_cond (mne_t mnemonic) {
    switch (mnemonic) {
        case BEQm:
            return EQ;
        case BNEm:
            return NE;
        case BGEm:
            return GE;
        case BLTm:
            return LT;
        case BGTm:
            return GT;
        case BLEm:
            return LE;
        case Bm:
            return AL;
        default:
            fprintf(stderr, "Mnemonic does not have a branch cond");
    }
    exit(EXIT_FAILURE);
}

//Accepts MNEMONIC TYPE as a parameter and returns respective OPCODE
static opcode_t mnemonic_to_op (mne_t mnemonic) {
    switch (mnemonic) {
        case ANDEQm:
        case ANDm:
            return AND;
        case EORm:
            return EOR;
        case SUBm:
            return SUB;
        case RSBm:
            return RSB;
        case ADDm:
            return ADD;
        case TSTm:
            return TST;
        case TEQm:
            return TEQ;
        case CMPm:
            return CMP;
        case ORRm:
            return ORR;
        case MOVm:
            return MOV;
        default:
            fprintf(stderr, "Mnemonic does not have an opcode");
    }
    exit(EXIT_FAILURE);
}

// Converts register to bits
static byte_t regbits(char *regs) {
    if(regs[0] == 'r') {
        long l = strtol(&regs[1], (char **) NULL, 16);
        return l;
    }
    fprintf(stderr, "Not a register (ERROR regbits)");
    exit(EXIT_FAILURE);
}

// Converts expression to bits 
static byte_t exprbits(char *expr) {
    if(expr[0] == '#') {
        //hexadecimal
        if(strstr(&expr[1],"0x")) {
            return strtol(&expr[1], (char **)NULL, 16);
        }
        //decimal
        else {
            return strtol(&expr[1], (char **)NULL, 10);
        }
    }
    fprintf(stderr, "Not an op2 (ERROR exprbits)");
    exit(EXIT_FAILURE);
}

static void shiftexpr_op2(ass_t *assem, char *expr) {
    //Process whether hex or dec
    //Convert to 32 bit binary
    word_t val;
    byte_t shift_amt = 0x20;
    if(strstr(expr,"0x")) {
        val = strtol(expr, (char **)NULL, 16);
    } 
    else {
        val = strtol(expr, (char **)NULL, 10);
    }
    halfword_t op2;
    // Immediate Value stored in op2
    // shifter:
    // find first 1 from the lsb
    // store 8 imm bit seq towards msb
    // make sure no 1's from there (first 1 + pos8) to msb
    // double shift seq and decrement shift(originally set to 32) by 1 
    if(val > 0xFF) {
        while (!(val & 0x3))
        {   
            shift_amt--;
            val >>= 2;
        }
        op2 = shift_amt;
        op2 = (op2 << 8) | val; 
    } else {
        op2 = val;
    }
    //reform op2 and allocate 12 bits accordingly
    // Immediate Value stored in op2
    op2 &= 0xFFF;
    assem->op2 = op2;
    // shifter:
    // find first 1 from the lsb
    // store 8 imm bit seq towards msb
    // make sure no 1's from there (first 1 + pos8) to msb
    // double shift seq and decrement shift(originally set to 32) by 1
}

// If shift consits register then processes accordingly and assigns it to the updated assem struct 
static void shiftreg_op2(ass_t *assem, tokens_t *new_tokens) {
    // rd, lsl #expr
    halfword_t op2;
    shift_t shift;
    char *shift_str = new_tokens->array_tokens[1];
    GET_SHIFT("lsl", LSL);
    GET_SHIFT("lsr", LSR);
    GET_SHIFT("asr", ASR);
    GET_SHIFT("ror", ROR);
    char *expr = substring(new_tokens->array_tokens[2], 1, strlen(new_tokens->array_tokens[2]));
    word_t val;

    //If expr is in hexadecimal format
    if(strstr(expr,"0x")) {
        val = strtol(expr, (char **)NULL, 16);
    } 
    //If expr is in decimal format
    else {
        val = strtol(expr, (char **)NULL, 10);
    }
    if(val > 0x1F) {
        fprintf(stderr, "Expr value cannot be greater than 0x1f");
    }//s1
    op2 = val;
    //s2 -> set 2 shift bits
    op2 <<= 2;
    switch (shift) {
    case LSL:
        break;
    case LSR:
        op2 |= 0x1;
        break;
    case ASR:
        op2 |= 0x2;
        break;
    case ROR:
        op2 |= 0x3;
        break;
    default:
        fprintf(stderr, "Illegal shifttype");
    }
    //s3 -> set 0 at end
    op2 <<= 1;
    //s4 -> set rd
    op2 = (op2 << 4) | assem->rd;
    assem->op2 = op2;
}

// Finds substring from index m (inclusive) to index n (exclusive) in String
static char* substring(const char *src, int m, int n) {   
    int len = n - m;
    char *dest = malloc(sizeof(char) * (len + 1));
    if (!dest) {  //added this
        perror("Mem error while creating substring\n");
        exit(EXIT_FAILURE);
    }
    for (int i = m; i < n && (*(src + i) != '\0'); i++) {
        *dest = *(src + i);
        dest++;
    }
    *dest = '\0';
    return dest - len;
}