#include "ElfParser.hpp"

using namespace ELFIO;

ELFParser::ELFParser(const std::string elfPath){
    if(!reader.load(elfPath)){
        std::cerr << "can't load elf file" << std::endl;
        exit(-1);
    }
}

ELFParser::~ELFParser(){};

section* 
ELFParser::GetSymbolTable(){
    for ( int i = 0; i < reader.sections.size(); ++i ) {
        section* psec = reader.sections[i];
        if(psec->get_name() == ".symtab"){
            return psec;
        }
    }
    std::cerr << "can't get symbol table" << std::endl;
    exit(-1);
}

uint64_t 
ELFParser::ReadSymbolTableByName(const std::string ref_name){
    const symbol_section_accessor symbols( reader, GetSymbolTable() );
    for ( unsigned int j = 0; j < symbols.get_symbols_num(); ++j ) {
        std::string name;
        Elf64_Addr value;
        Elf_Xword size;
        unsigned char bind;
        unsigned char type;
        Elf_Half section_index;
        unsigned char other;
        symbols.get_symbol( j, name, value, size, bind,type, section_index, other ); 
        if(name == ref_name){
            return value;
        }
    }
    std::cerr << "can't get symbol table by name: " << ref_name << std::endl;
    exit(-1);
}

uint64_t 
ELFParser::GetStart(){
    return ReadSymbolTableByName("_start");
}

uint64_t 
ELFParser::GetSize(){
    return ReadSymbolTableByName("_end") - ReadSymbolTableByName("_start");
}

uint64_t 
ELFParser::GetTohost(){
    return ReadSymbolTableByName("tohost");
}

uint64_t 
ELFParser::GetFromhost(){
    return ReadSymbolTableByName("fromhost");
}

void 
ELFParser::Load(char* mem){
    for( size_t i = 0; i < reader.segments.size(); i++){
        const segment* pseg = reader.segments[i];
        const size_t   size = pseg->get_file_size();
        const uint64_t addr = pseg->get_physical_address() - GetStart();
        const char*    data = pseg->get_data();
        if(data != NULL){
            memcpy(mem+addr,data,size);
        }
    }
}

void operator<< (TimingModel::BaseMemory& base_memory, ELFParser& elf_parser) {
    assert(base_memory.size() >= elf_parser.GetSize());
    for( size_t i = 0; i < elf_parser.reader.segments.size(); i++){
        const segment* pseg = elf_parser.reader.segments[i];
        const size_t   size = pseg->get_file_size();
        const uint64_t addr = pseg->get_physical_address();
        const char*    data = pseg->get_data();
        if(data){
            base_memory.Write(addr, data, size);
        }
    }
}

