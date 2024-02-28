
#include "InstGenerator.hpp"
#include "json.hpp"  // From Mavis
#include "mavis/Mavis.h"

namespace TimingModel
{
    std::unique_ptr<InstGenerator> InstGenerator::createGenerator(MavisType * mavis_facade,
                                                                  const std::string & filename,
                                                                  const bool skip_nonuser_mode)
    {
        const std::string json_ext = "json";
        if((filename.size() > json_ext.size()) && filename.substr(filename.size()-json_ext.size()) == json_ext) {
            std::cout << "olympia: JSON file input detected" << std::endl;
            return std::unique_ptr<InstGenerator>(new JSONInstGenerator(mavis_facade, filename));
        }

        const std::string stf_ext = "stf";  // Should cover both zstf and stf
        if((filename.size() > stf_ext.size()) && filename.substr(filename.size()-stf_ext.size()) == stf_ext) {
            std::cout << "olympia: STF file input detected" << std::endl;
            return std::unique_ptr<InstGenerator>(new TraceInstGenerator(mavis_facade, filename, skip_nonuser_mode));
        }

        // Dunno what it is...
        sparta_assert(false, "Unknown file extension for '" << filename
                      << "'.  Expected .json or .[z]stf");
        return nullptr;
    }

    std::unique_ptr<InstGenerator> InstGenerator::createGenerator(MavisType * mavis_facade,
                                                                  const std::string & filename)
    {
        if(filename.size() > 0) {
            std::cout << "spike elf file input detected" << std::endl;
            return std::unique_ptr<InstGenerator>(new SpikeInstGenerator(mavis_facade, filename));
        }
        return nullptr;
    }
    void InstGenerator::InsnComplete(InstPtr& inst) const {
        switch(inst->getUnit()){
            case InstArchInfo::TargetUnit::LSU:
                if(inst->isStoreInst()){
                    inst->setFuType(FuncType::STU);
                }else{
                    inst->setFuType(FuncType::LDU);
                }
                break;
            case InstArchInfo::TargetUnit::BR:
                inst->setFuType(FuncType::BRU);
                break;
            case InstArchInfo::TargetUnit::ALU:
                inst->setFuType(FuncType::ALU);
                break;
            case InstArchInfo::TargetUnit::FPU:
                inst->setFuType(FuncType::FPU);
                break;
            default:
                inst->setFuType(FuncType::ALU);
        }
        uint32_t intSourceNum = inst->numIntSourceRegs();
        uint32_t fpSourceNum  = inst->numFloatSourceRegs();
        uint32_t intDestNum   = inst->numIntDestRegs();
        uint32_t fpDestNum    = inst->numFloatDestRegs();
        std::bitset<64> int_source_stream   = inst->getIntSourceRegs();
        std::bitset<64> float_source_stream = inst->getFloatSourceRegs();
        std::bitset<64> int_dst_stream      = inst->getIntDestRegs();
        std::bitset<64> float_dst_stream    = inst->getFloatDestRegs();
        uint8_t cnt = 1;
        if(intSourceNum > 0) {
            switch (intSourceNum) {
                case 1:
                    for(int i = 0; i<64; i++){
                        if(int_source_stream[i]){
                            inst->setIsaRs1(i);
                            inst->setRs1Type(RegType_t::INT);
                            break;
                        }
                    }
                    break;
                case 2:
                    for(int i = 0; i<64; i++){
                        if(int_source_stream[i] && (cnt == 1)){
                            inst->setIsaRs1(i);
                            inst->setRs1Type(RegType_t::INT);
                            ++cnt;
                        }else if(int_source_stream[i] && (cnt == 2)){
                            inst->setIsaRs2(i);
                            inst->setRs2Type(RegType_t::INT);
                            ++cnt;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        cnt = 1;
        if(fpSourceNum > 0) {
            switch (fpSourceNum) {
                case 1:
                    for(int i = 0; i<64; i++){
                        if(float_source_stream[i]){
                            inst->setIsaRs2(i);
                            inst->setRs2Type(RegType_t::FLOAT);
                            break;
                        }
                    }
                    break;
                case 2:
                    for(int i = 0; i<64; i++){
                        if(float_source_stream[i] && (cnt == 1)){
                            inst->setIsaRs1(i);
                            inst->setRs1Type(RegType_t::FLOAT);
                            ++cnt;
                        }else if(float_source_stream[i] && (cnt == 2)){
                            inst->setIsaRs2(i);
                            inst->setRs2Type(RegType_t::FLOAT);
                            ++cnt;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        if(intDestNum > 0) {
            switch (intDestNum) {
                case 1:
                    for(int i = 0; i<64; i++){
                        if(int_dst_stream[i]){
                            inst->setIsaRd(i);
                            inst->setRdType(RegType_t::INT);
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        if(fpDestNum > 0) {
            switch (fpDestNum) {
                case 1:
                    for(int i = 0; i<64; i++){
                        if(float_dst_stream[i]){
                            inst->setIsaRd(i);
                            inst->setRdType(RegType_t::FLOAT);
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

    }

    ////////////////////////////////////////////////////////////////////////////////
    // JSON Inst Generator
    JSONInstGenerator::JSONInstGenerator(MavisType * mavis_facade,
                                         const std::string & filename) :
        InstGenerator(mavis_facade)
    {
        std::ifstream fs;
        std::ios_base::iostate exceptionMask = fs.exceptions() | std::ios::failbit;
        fs.exceptions(exceptionMask);

        try {
            fs.open(filename);
        } catch (const std::ifstream::failure &e) {
            throw sparta::SpartaException("ERROR: Issues opening ") << filename << ": " << e.what();
        }

        jobj_.reset(new nlohmann::json);
        fs >> *jobj_;
        n_insts_ = jobj_->size();
    }

    bool JSONInstGenerator::isDone() const {
        return (curr_inst_index_ == n_insts_);
    }

    InstPtr JSONInstGenerator::getNextInst(const sparta::Clock * clk)
    {
        if(SPARTA_EXPECT_FALSE(isDone())) {
            return nullptr;
        }

        // Get the JSON record at the current index
        nlohmann::json jinst = jobj_->at(curr_inst_index_);

        if (jinst.find("mnemonic") == jinst.end()) {
            throw sparta::SpartaException() << "Missing mnemonic at " << curr_inst_index_;
        }
        const std::string mnemonic = jinst["mnemonic"];

        auto addElement =  [&jinst] (mavis::OperandInfo & operands,
                                     const std::string & key,
                                     const mavis::InstMetaData::OperandFieldID operand_field_id,
                                     const mavis::InstMetaData::OperandTypes operand_type) {
                               if(jinst.find(key) != jinst.end()) {
                                   operands.addElement(operand_field_id,
                                                       operand_type,
                                                       jinst[key].get<uint64_t>());
                               }
                           };

        mavis::OperandInfo srcs;
        addElement(srcs, "rs1", mavis::InstMetaData::OperandFieldID::RS1, mavis::InstMetaData::OperandTypes::LONG);
        addElement(srcs, "fs1", mavis::InstMetaData::OperandFieldID::RS1, mavis::InstMetaData::OperandTypes::DOUBLE);
        addElement(srcs, "rs2", mavis::InstMetaData::OperandFieldID::RS2, mavis::InstMetaData::OperandTypes::LONG);
        addElement(srcs, "fs2", mavis::InstMetaData::OperandFieldID::RS2, mavis::InstMetaData::OperandTypes::DOUBLE);

        mavis::OperandInfo dests;
        addElement(dests, "rd", mavis::InstMetaData::OperandFieldID::RD, mavis::InstMetaData::OperandTypes::LONG);
        addElement(dests, "fd", mavis::InstMetaData::OperandFieldID::RD, mavis::InstMetaData::OperandTypes::DOUBLE);

        InstPtr inst;
        if(jinst.find("imm") != jinst.end()) {
            const uint64_t imm = jinst["imm"].get<uint64_t>();
            mavis::ExtractorDirectOpInfoList ex_info(mnemonic, srcs, dests, imm);
            inst = mavis_facade_->makeInstDirectly(ex_info, clk);
        }
        else {
            mavis::ExtractorDirectOpInfoList ex_info(mnemonic, srcs, dests);
            inst = mavis_facade_->makeInstDirectly(ex_info, clk);
        }

        if (jinst.find("vaddr") != jinst.end()) {
            uint64_t vaddr = std::strtoull(jinst["vaddr"].get<std::string>().c_str(), nullptr, 0);
            inst->setTargetVAddr(vaddr);
        }

        ++curr_inst_index_;
        if (inst != nullptr) {
            inst->setUniqueID(++unique_id_);
            inst->setProgramID(unique_id_);
        }
        return inst;

    }

    ////////////////////////////////////////////////////////////////////////////////
    // STF Inst Generator
    TraceInstGenerator::TraceInstGenerator(MavisType * mavis_facade,
                                           const std::string & filename,
                                           const bool skip_nonuser_mode) :
        InstGenerator(mavis_facade)
    {
        std::ifstream fs;
        std::ios_base::iostate exceptionMask = fs.exceptions() | std::ios::failbit;
        fs.exceptions(exceptionMask);

        try {
            fs.open(filename);
        } catch (const std::ifstream::failure &e) {
            throw sparta::SpartaException("ERROR: Issues opening ") << filename << ": " << e.what();
        }

        // If true, search for an stf-pte file alongside this trace.
        constexpr bool CHECK_FOR_STF_PTE = false;

        // Filter out mode change events regardless of skip_nonuser_mode
        // value. Required for traces that stay in machine mode the entire
        // time
        constexpr bool FILTER_MODE_CHANGE_EVENTS = true;
        constexpr size_t BUFFER_SIZE             = 4096;
        reader_.reset(new stf::STFInstReader(filename,
                                             skip_nonuser_mode,
                                             CHECK_FOR_STF_PTE,
                                             FILTER_MODE_CHANGE_EVENTS,
                                             BUFFER_SIZE));

        next_it_ = reader_->begin();
    }

    bool TraceInstGenerator::isDone() const {
        return next_it_ == reader_->end();
    }
    InstPtr TraceInstGenerator::getNextInst(const sparta::Clock * clk)
    {
        if(SPARTA_EXPECT_FALSE(isDone())) {
            return nullptr;
        }

        mavis::Opcode opcode = next_it_->opcode();

        try {
            InstPtr inst = mavis_facade_->makeInst(opcode, clk);
            inst->setPC(next_it_->pc());
            inst->setUniqueID(++unique_id_);
            inst->setProgramID(unique_id_);
            inst->setIsRvcInst(next_it_->isOpcode16());
            inst->setCompressedInst(inst->getOpCode());
            inst->setUncompressedInst(inst->getOpCode());
            inst->setImm(inst->getImmediate());
            InsnComplete(inst);
            if (const auto& mem_accesses = next_it_->getMemoryAccesses(); !mem_accesses.empty())
            {
                using VectorAddrType = std::vector<sparta::memory::addr_t>;
                VectorAddrType addrs;
                std::for_each(next_it_->getMemoryAccesses().begin(),
                              next_it_->getMemoryAccesses().end(),
                              [&addrs] (const auto & ma) {
                                  addrs.emplace_back(ma.getAddress());
                              });
                inst->setTargetVAddr(addrs.front());
                //For misaligns, more than 1 address is provided
                //inst->setVAddrVector(std::move(addrs));
            }
            // std::cout << "GenInsn: uid[" << inst->getUniqueID() << "], Pc[0x" << std::hex << inst->getPc() 
            //           << "], IsRvcInsn: " << std::dec << inst->getIsRvcInst() << ", UnCompressedInst[0x"
            //           << std::hex << inst->getUncompressedInst() << std::dec << "], Fu: " << inst->getFuType() 
            //           << ", IsaRs1[" << unsigned(inst->getIsaRs1()) << "]-Type[" << inst->getRs1Type() 
            //           << "], IsaRs2[" << unsigned(inst->getIsaRs2()) << "]-Type[" << inst->getRs2Type() 
            //           << "], IsaRd[" << unsigned(inst->getIsaRd()) << "]-Type[" << inst->getRdType() 
            //           << "], Imm[0x" << std::hex << inst->getImm() << std::dec << "], insn: '" << inst->getDisasm() << "'" << std::endl;
            ++next_it_;
            return inst;
        }
        catch(std::exception & excpt) {
            std::cerr << "ERROR: Mavis failed decoding: 0x"
                      << std::hex << opcode << " for STF It PC: 0x"
                      << next_it_->pc() << " STFID: " << std::dec
                      << next_it_->index() << " err: "
                      << excpt.what() << std::endl;
            throw;
        }
        return nullptr;
    }

    SpikeInstGenerator::SpikeInstGenerator(MavisType * mavis_facade,
                           const std::string & filename):
        InstGenerator(mavis_facade),
        spike_adapter_(spikeAdapter::getSpikeAdapter())
    {
        std::vector<std::string> commandLineArgs;

        commandLineArgs.push_back("spike");
        commandLineArgs.push_back("--dtb=default.dtb");
        commandLineArgs.push_back("--log-commits");
        commandLineArgs.push_back(filename);

        spike_adapter_->spikeInit(commandLineArgs);
        spike_adapter_->spikeRunStart();

    }

    InstPtr SpikeInstGenerator::getNextInst(const sparta::Clock * clk){

        if(!isDone()){
            spike_adapter_->spikeStep(spike_adapter_->spikeTunnelAvailCnt());
            spikeInsnPtr sinsn = spike_adapter_->spikeGetNextInst();
            InstPtr mavis_inst = mavis_facade_->makeInst(sinsn->spike_insn_.insn.bits(), clk);
            mavis_inst->setPC(sinsn->getPc());
            mavis_inst->setUniqueID(++unique_id_);
            mavis_inst->setProgramID(unique_id_);
            mavis_inst->setIsRvcInst(sinsn->spike_insn_.insn.bits());
            mavis_inst->setCompressedInst(sinsn->spike_insn_.insn.bits());
            mavis_inst->setUncompressedInst(sinsn->spike_insn_.insn.bits());
            mavis_inst->setImm(mavis_inst->getImmediate());
            InsnComplete(mavis_inst);
            return mavis_inst;
        }
        return nullptr;
    }
    bool SpikeInstGenerator::isDone() const { return spike_adapter_->is_done; }
}
