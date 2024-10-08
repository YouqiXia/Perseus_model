#pragma once

#include <memory>
#include <vector>

namespace TimingModel {

struct InstInfo;

typedef uint64_t Credit;

typedef uint16_t ThreadId;

typedef uint64_t Addr_t;

typedef uint32_t Inst_t;

typedef uint16_t CompressedInst_t;

typedef uint8_t  IsaRegId_t;

typedef uint64_t PhyRegId_t;

typedef int32_t  Imm_t;

typedef uint64_t xlen_t;

typedef uint64_t xReg_t;

typedef uint64_t RobIdx_t;

typedef std::vector<xReg_t> PhysicalReg;

typedef std::vector<std::tuple<std::string, int, size_t*>> RegWrite_t;

typedef std::vector<std::tuple<size_t, int>> MemRead_t;

typedef std::vector<std::tuple<size_t, int, size_t*>> MemWrite_t;

enum FuncType {
    NO_TYPE=0, ALU, MUL, DIV, BRU, CSR, LDU, STU, FPU, LAST
};

/* exception */
struct Exception
{
    bool    valid;
    xlen_t  Cause;
    xlen_t  Tval;
};

// function sub opcode
#define BRU_JAR         0
#define BRU_JALR        1
#define BRU_BEQ         2
#define BRU_BNE         3
#define BRU_BLT         4
#define BRU_BGE         5
#define BRU_BLTU        6
#define BRU_BGEU        7

#define CSR_CSRRW       0
#define CSR_CSRRS       1
#define CSR_CSRRC       2
#define CSR_CSRR        3
#define CSR_FENCEI      4
#define CSR_ECALL       5
#define CSR_EBREAK      6
#define CSR_MRET        7
#define CSR_SRET        8
#define CSR_FENCE       9

#define LDU_LB          0
#define LDU_LH          1
#define LDU_LW          2
#define LDU_LD          3
#define LDU_LBU         4
#define LDU_LHU         5
#define LDU_LWU         6

#define STU_SB          0
#define STU_SH          1
#define STU_SW          2
#define STU_SD          3

#define ALU_ADD         0
#define ALU_SUB         1
#define ALU_SLL         2
#define ALU_SLT         3
#define ALU_SLTU        4
#define ALU_XOR         5
#define ALU_SRL         6
#define ALU_SRA         7
#define ALU_OR          8
#define ALU_AND         9

#define ALU_ADDW        10
#define ALU_SUBW        11
#define ALU_SLLW        12
#define ALU_SRLW        13
#define ALU_SRAW        14


#define MUL_MUL         0
#define MUL_MULH        1
#define MUL_MULHSU      2
#define MUL_MULHU       3

#define MUL_MULW        4

#define DIV_DIV         0
#define DIV_DIVU        1
#define DIV_REM         2
#define DIV_REMU        3

#define DIV_DIVW        4
#define DIV_DIVUW       5
#define DIV_REMW        6
#define DIV_REMUW       7

enum RegType_t{
    NONE, INT, FLOAT
};

struct InstInfo {

    /* fetch info */
    Addr_t       pc;
    bool         IsRvcInst;
    Inst_t       CompressedInst;
    Inst_t       UncompressedInst;

    /* branch prediction info */
    bool         IsMissPrediction = false;
    Addr_t       SpikeNpc;

    /* decode info */
    IsaRegId_t   IsaRs1 = 0;
    IsaRegId_t   IsaRs2 = 0;
    IsaRegId_t   IsaRd  = 0;
    Imm_t        imm    = 0;

    RegType_t    Rs1Type;
    RegType_t    Rs2Type;
    RegType_t    RdType;

    FuncType     Fu;
    uint8_t      SubOp;

    /* Scheduler info */
    RobIdx_t     RobTag;
    bool         IsRs1Forward = false;
    bool         IsRs2Forward = false;

    uint64_t     LSQTag;

    PhyRegId_t   PhyRs1 = 0;
    PhyRegId_t   PhyRs2 = 0;
    PhyRegId_t   PhyRd = 0;
    PhyRegId_t   LPhyRd = 0;

    /* function unit info */
    xReg_t       Operand1 = 0;
    xReg_t       Operand2 = 0;
    xReg_t       RdResult = 0;

    /* mem op */

    /* exception info */
    Exception    Excp;

    /* commit info */
    RegWrite_t reg_write;
    MemRead_t  mem_read;
    MemWrite_t mem_write;
};

}
