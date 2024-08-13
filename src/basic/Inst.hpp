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
    typedef uint64_t FlushingCriteria;

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
        bool hasImmediate() const { return opcode_info_->hasImmediate(); }
        uint64_t getImmediate() const { return opcode_info_->getImmediate(); } //FIXME: uint64 for imm may cause bug.
        std::bitset<64> getIntSourceRegs() const { return opcode_info_->getIntSourceRegs(); }
        std::bitset<64> getFloatSourceRegs() const { return opcode_info_->getFloatSourceRegs(); }
        std::bitset<64> getIntDestRegs() const { return opcode_info_->getIntDestRegs(); }
        std::bitset<64> getFloatDestRegs() const { return opcode_info_->getFloatDestRegs(); }
        uint32_t numIntSourceRegs() const { return opcode_info_->numIntSourceRegs(); }
        uint32_t numFloatSourceRegs() const { return opcode_info_->numFloatSourceRegs(); }
        uint32_t numIntDestRegs() const { return opcode_info_->numIntDestRegs(); }
        uint32_t numFloatDestRegs() const { return opcode_info_->numFloatDestRegs(); }

        // old type inst
        InstInfo & getInstInfo() { return inst_; }

        Addr_t getPc() const { return inst_.pc; }

        void setLSQTag(uint64_t LSQTag) { inst_.LSQTag = LSQTag; }
        uint64_t getLSQTag() { return inst_.LSQTag; }

        void setIsRvcInst(bool IsRvcInst) { inst_.IsRvcInst = IsRvcInst; }
        bool getIsRvcInst() { return inst_.IsRvcInst; }

        void   setCompressedInst(Inst_t CompressedInst) { inst_.CompressedInst = CompressedInst; }
        Inst_t getCompressedInst() { return inst_.CompressedInst; }

        void   setUncompressedInst(Inst_t UncompressedInst) { inst_.UncompressedInst = UncompressedInst; }
        Inst_t getUncompressedInst() { return inst_.UncompressedInst; }

        void   setIsMissPrediction(bool IsMissPrediction) { inst_.IsMissPrediction = IsMissPrediction; }
        bool getIsMissPrediction() { return inst_.IsMissPrediction; }

        void   setSpikeNpc(Addr_t SpikeNpc) { inst_.SpikeNpc = SpikeNpc; }
        Addr_t getSpikeNpc() { return inst_.SpikeNpc; }

        void   setRegWrite(RegWrite_t reg_write) { inst_.reg_write = reg_write; }
        RegWrite_t getRegWrite() { return inst_.reg_write; }

        void   setMemRead(MemRead_t mem_read) { inst_.mem_read = mem_read; }
        MemRead_t getMemRead() { return inst_.mem_read; }

        void   setMemWrite(MemWrite_t mem_write) { inst_.mem_write = mem_write; }
        MemWrite_t getMemWrite() { return inst_.mem_write; }

        void  clearCommitInfo() {
            for(auto entry : inst_.reg_write) {
                free(std::get<2>(entry));
            }

            for(auto entry : inst_.mem_write) {
                free(std::get<2>(entry));
            }

            inst_.reg_write.clear(); 
            inst_.mem_read.clear();
            inst_.mem_write.clear();
        }

        void        setIsaRs1(IsaRegId_t IsaRs1) { inst_.IsaRs1 = IsaRs1; }
        IsaRegId_t  getIsaRs1() { return inst_.IsaRs1; }

        void        setIsaRs2(IsaRegId_t IsaRs2) { inst_.IsaRs2 = IsaRs2; }
        IsaRegId_t  getIsaRs2() { return inst_.IsaRs2; }

        void        setIsaRd(IsaRegId_t IsaRd) { inst_.IsaRd = IsaRd; }
        IsaRegId_t  getIsaRd() { return inst_.IsaRd; }

        void  setImm(Imm_t imm) { inst_.imm = imm; }
        Imm_t getImm() { return inst_.imm; } 

        void setRs1Type(RegType_t Rs1Type) { inst_.Rs1Type = Rs1Type; }
        RegType_t getRs1Type() const { return inst_.Rs1Type; }
        
        void setRs2Type(RegType_t Rs2Type) { inst_.Rs2Type = Rs2Type; }
        RegType_t getRs2Type() const { return inst_.Rs2Type; }

        void setRdType(RegType_t RdType) { inst_.RdType = RdType; }
        RegType_t getRdType() const { return inst_.RdType; }

        void setFuType(FuncType func_type) { inst_.Fu = func_type; }
        FuncType getFuType() const { return inst_.Fu; }

        void setSubOp(uint8_t sub_op) { inst_.SubOp = sub_op; }
        uint8_t getSubOp() const { return inst_.SubOp; }

        void setRobTag(uint64_t RobTag) { inst_.RobTag = RobTag; }
        uint64_t getRobTag() const { return inst_.RobTag; }

        void setIsRs1Forward(bool IsRs1Forward) { inst_.IsRs1Forward = IsRs1Forward; }
        bool getIsRs1Forward() const { return inst_.IsRs1Forward; }

        void setIsRs2Forward(bool IsRs2Forward) { inst_.IsRs2Forward = IsRs2Forward; }
        bool getIsRs2Forward() const { return inst_.IsRs2Forward; }

        void setPhyRs1(PhyRegId_t PhyRs1) { inst_.PhyRs1 = PhyRs1; }
        uint64_t getPhyRs1() const { return inst_.PhyRs1; }

        void setPhyRs2(PhyRegId_t PhyRs2) { inst_.PhyRs2 = PhyRs2; }
        uint64_t getPhyRs2() const { return inst_.PhyRs2; }

        void setPhyRd(PhyRegId_t PhyRd) { inst_.PhyRd = PhyRd; }
        uint64_t getPhyRd() const { return inst_.PhyRd; }

        void setLPhyRd(PhyRegId_t LPhyRd) { inst_.LPhyRd = LPhyRd; }
        uint64_t getLPhyRd() const { return inst_.LPhyRd; }

        void setOperand1(xReg_t Operand1) { inst_.Operand1 = Operand1; }
        xReg_t getOperand1() const { return inst_.Operand1; }

        void setOperand2(xReg_t Operand2) { inst_.Operand2 = Operand2; }
        xReg_t getOperand2() const { return inst_.Operand2; }

        void setRdResult(xReg_t RdResult) { inst_.RdResult = RdResult; }
        xReg_t getRdResult() const { return inst_.RdResult; }

        // Static instruction information
        bool        isLoadStoreInst() const {return inst_arch_info_->isLoadStore(); }
        uint32_t    getExecuteTime() const { return inst_arch_info_->getExecutionTime(); }

        uint64_t    getRAdr() const        { return target_vaddr_ | 0x8000000; } // faked
        bool        isStoreInst() const    { return is_store_; } 
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
        bool                   is_store_;
        const bool             is_transfer_;  // Is this a transfer instruction (F2I/I2F)
        sparta::Scheduleable * ev_retire_    = nullptr;
        InstInfo               inst_;
    };

    using InstPtr = Inst::PtrType;
    using InstQueue = sparta::Queue<InstPtr>;

    inline std::ostream & operator<<(std::ostream & os, const Inst & inst) {
        os << "uid: " << inst.getUniqueID()
           << " " << std::setw(4)
           << ", pc: " << std::hex << inst.getPC() << std::dec
           << ", pid: " << inst.getProgramID() << " '" << inst.getDisasm() << "' "
           << ", prd: " << inst.getPhyRd() << " ps1: " << inst.getPhyRs1()
           << " ps2: " << inst.getPhyRs2();
        return os;
    }

    inline std::ostream & operator<<(std::ostream & os, const InstPtr & inst) {
        os << *inst;
        return os;
    }

    // Instruction allocators
    using InstAllocator         = sparta::SpartaSharedPointerAllocator<Inst>;
    using InstArchInfoAllocator = sparta::SpartaSharedPointerAllocator<InstArchInfo>;

}
