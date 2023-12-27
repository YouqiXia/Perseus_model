// <Inst.h> -*- C++ -*-

#pragma once

#include "sparta/memory/AddressTypes.hpp"
#include "sparta/resources/Scoreboard.hpp"
#include "sparta/resources/Queue.hpp"
#include "sparta/pairs/SpartaKeyPairs.hpp"
#include "sparta/simulation/State.hpp"
#include "sparta/utils/SpartaSharedPointer.hpp"
#include "sparta/utils/SpartaSharedPointerAllocator.hpp"
#include "mavis/OpcodeInfo.h"

#include "olympia/InstArchInfo.hpp"
#include "olympia/CoreTypes.hpp"
#include "olympia/MiscUtils.hpp"

#include <cstdlib>
#include <ostream>
#include <map>
#include <vector>

#include "basic/Instruction.hpp"

namespace TimingModel
{
    /* for schduler */
    typedef uint8_t  IssueNum;

    typedef uint64_t RobIdx;

    typedef uint64_t RSIdx;

    typedef uint64_t FreeListIdx;

    typedef uint64_t BusyTableIdx;

    typedef uint64_t DispatchQueueIdx;

    /*!
     * \class Inst
     * \brief Example instruction that flows through the example/CoreModel
     */

    class Inst {
    public:

        // Used by Mavis
        using PtrType = sparta::SpartaSharedPointer<Inst>;

        /*!
         * \brief Construct an Instruction
         * \param opcode_info    Mavis Opcode information
         * \param inst_arch_info Pointer to the static data of instruction
         * \param clk            Core clock
         *
         * Called by Mavis when an opcode is decoded to a particular
         * instruction.
        */
        Inst(const mavis::OpcodeInfo::PtrType& opcode_info,
             const InstArchInfo::PtrType     & inst_arch_info,
             const sparta::Clock             * clk);

        // This is needed by Mavis as an optimization.  Try NOT to
        // implement it and let the compiler do it for us for speed.
        Inst(const Inst& other) = default;

        InstArchInfo::TargetUnit getUnit() const {
            return inst_arch_info_->getTargetUnit();
        }

        InstArchInfo::TargetPipe getPipe() const {
            return inst_arch_info_->getTargetPipe();
        }

        // Set the instructions unique ID.  This ID in constantly
        // incremented and does not repeat.  The same instruction in a
        // trace can have different unique IDs (due to flushing)
        void     setUniqueID(uint64_t uid) { unique_id_ = uid; }
        uint64_t getUniqueID() const       { return unique_id_; }

        // Set the instruction's Program ID.  This ID is specific to
        // an instruction's retire pointer.  The same instruction in a
        // trace will have the same program ID (as compared to
        // UniqueID).
        void     setProgramID(uint64_t prog_id) { program_id_ = prog_id; }
        uint64_t getProgramID() const           { return program_id_; }

        // Set the instruction's PC
        void setPC(sparta::memory::addr_t inst_pc) { inst_pc_ = inst_pc; inst_.pc = inst_pc; }
        sparta::memory::addr_t getPC() const       { return inst_pc_; }

        // Set the instruction's target PC (branch target or load/store target)
        void     setTargetVAddr(sparta::memory::addr_t target_vaddr) { target_vaddr_ = target_vaddr; }
        sparta::memory::addr_t getTargetVAddr() const                { return target_vaddr_; }

        // Opcode information
        std::string getMnemonic() const { return opcode_info_->getMnemonic(); }
        std::string getDisasm()   const { return opcode_info_->dasmString(); }
        uint32_t    getOpCode()   const { return static_cast<uint32_t>(opcode_info_->getOpcode()); }

        // Operand information
        using OpInfoList = mavis::DecodedInstructionInfo::OpInfoList;
        const OpInfoList& getSourceOpInfoList() const { return opcode_info_->getSourceOpInfoList(); }
        const OpInfoList& getDestOpInfoList()   const { return opcode_info_->getDestOpInfoList(); }

        // old type inst
        InstInfo & getInstInfo() { return inst_; }

        void setIsRvcInst(bool IsRvcInst) { inst_.IsRvcInst = IsRvcInst; }
        bool getIsRvcInst() { return inst_.IsRvcInst; }

        void   setCompressedInst(Inst_t CompressedInst) { inst_.CompressedInst = CompressedInst; }
        Inst_t getCompressedInst() { return inst_.CompressedInst; }

        void   setUncompressedInst(Inst_t UncompressedInst) { inst_.UncompressedInst = UncompressedInst; }
        Inst_t getUncompressedInst() { return inst_.UncompressedInst; }

        void        setIsaRs1(IsaRegId_t IsaRs1) { inst_.IsaRs1 = IsaRs1; }
        IsaRegId_t  getIsaRs1() { return inst_.IsaRs1; }

        void        setIsaRs2(IsaRegId_t IsaRs2) { inst_.IsaRs2 = IsaRs2; }
        IsaRegId_t  getIsaRs2() { return inst_.IsaRs2; }

        void        setIsaRd(IsaRegId_t IsaRd) { inst_.IsaRd = IsaRd; }
        IsaRegId_t  getIsaRd() { return inst_.IsaRd; }

        void  setImm(Imm_t imm) { inst_.imm = imm; }
        Imm_t getImm() { return inst_.imm; } 

        void setRs1Type(RegType_t Rs1Type) { inst_.Rs1Type = Rs1Type; }
        RegType_t getRs1Type() { return inst_.Rs1Type; }
        
        void setRs2Type(RegType_t Rs2Type) { inst_.Rs2Type = Rs2Type; }
        RegType_t getRs2Type() { return inst_.Rs2Type; }

        void setRdType(RegType_t RdType) { inst_.RdType = RdType; }
        RegType_t getRdType() { return inst_.RdType; }

        void setFuType(FuncType func_type) { inst_.Fu = func_type; }
        FuncType getFuType() { return inst_.Fu; }

        void setSubOp(uint8_t sub_op) { inst_.SubOp = sub_op; }
        uint8_t getSubOp() { return inst_.SubOp; }

        void setRobTag(uint64_t RobTag) { inst_.RobTag = RobTag; }
        uint64_t getRobTag() { return inst_.RobTag; }

        void setPhyRs1(PhyRegId_t PhyRs1) { inst_.PhyRs1 = PhyRs1; }
        uint64_t getPhyRs1() { return inst_.PhyRs1; }

        void setPhyRs2(PhyRegId_t PhyRs2) { inst_.PhyRs2 = PhyRs2; }
        uint64_t getPhyRs2() { return inst_.PhyRs2; }

        void setPhyRd(PhyRegId_t PhyRd) { inst_.PhyRd = PhyRd; }
        uint64_t getPhyRd() { return inst_.PhyRd; }

        void setLPhyRd(PhyRegId_t LPhyRd) { inst_.LPhyRd = LPhyRd; }
        uint64_t getLPhyRd() { return inst_.LPhyRd; }

        void setOperand1(xReg_t Operand1) { inst_.Operand1 = Operand1; }
        xReg_t getOperand1() { return inst_.Operand1; }

        void setOperand2(xReg_t Operand2) { inst_.Operand2 = Operand2; }
        xReg_t getOperand2() { return inst_.Operand2; }

        void setRdResult(xReg_t RdResult) { inst_.RdResult = RdResult; }
        xReg_t getRdResult() { return inst_.RdResult; }

        // Static instruction information
        bool        isLoadStoreInst() const {return inst_arch_info_->isLoadStore(); }
        uint32_t    getExecuteTime() const { return inst_arch_info_->getExecutionTime(); }

        uint64_t    getRAdr() const        { return target_vaddr_ | 0x8000000; } // faked
        bool        isSpeculative() const  { return is_speculative_; }
        bool        isTransfer() const     { return is_transfer_; }

    private:
        mavis::OpcodeInfo::PtrType opcode_info_;
        InstArchInfo::PtrType      inst_arch_info_;

        sparta::memory::addr_t inst_pc_       = 0; // Instruction's PC
        sparta::memory::addr_t target_vaddr_  = 0; // Instruction's Target PC (for branches, loads/stores)
        bool                   is_oldest_       = false;
        uint64_t               unique_id_     = 0; // Supplied by Fetch
        uint64_t               program_id_    = 0; // Supplied by a trace Reader or execution backend
        bool                   is_speculative_ = false; // Is this instruction soon to be flushed?
        const bool             is_transfer_;  // Is this a transfer instruction (F2I/I2F)
        sparta::Scheduleable * ev_retire_    = nullptr;
        InstInfo               inst_;
    };

    using InstPtr = Inst::PtrType;
    using InstQueue = sparta::Queue<InstPtr>;
    using InstGroup = std::vector<InstPtr>;

    inline std::ostream & operator<<(std::ostream & os, const Inst & inst) {
        os << "uid: " << inst.getUniqueID()
           << " " << std::setw(10)
           << " " << std::hex << inst.getPC() << std::dec
           << " pid: " << inst.getProgramID() << " '" << inst.getDisasm() << "' ";
        return os;
    }

    inline std::ostream & operator<<(std::ostream & os, const InstPtr & inst) {
        os << *inst;
        return os;
    }

    // Instruction allocators
    using InstAllocator         = sparta::SpartaSharedPointerAllocator<Inst>;
    using InstArchInfoAllocator = sparta::SpartaSharedPointerAllocator<InstArchInfo>;

    struct RobEntry
    {
        InstPtr inst_ptr;
        bool is_valid;
        bool is_finished;
        bool is_issued;
    };

    struct ScalarRSEntry
    {
        RSIdx rs_idx;
        InstPtr inst_ptr;
        RobIdx forwardRobidx[2];
        bool is_forwarding[2];
        bool is_OperandReady[2];
    };

}
