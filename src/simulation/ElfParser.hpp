#ifndef __ELFPARSER_HH__
#define __ELFPARSER_HH__

#include "elfio/elfio.hpp"
#include "uncore/memory/BaseMemory.hpp"
#include <string>

class ELFParser
{
private:
    ELFIO::elfio reader;

public:
    ELFParser(const std::string elfPath);

    ~ELFParser();

    ELFIO::section *GetSymbolTable();

    uint64_t ReadSymbolTableByName(const std::string ref_name);

    uint64_t GetStart();

    uint64_t GetSize();

    uint64_t GetTohost();

    uint64_t GetFromhost();

    void Load(char *mem);

    friend void operator<< (TimingModel::BaseMemory&, ELFParser&);
};


#endif
