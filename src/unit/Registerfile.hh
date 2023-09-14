#ifndef MODEL_REGISTERFILE_HH
#define MODEL_REGISTERFILE_HH

#include "basicunit/Register.hh"
#include <vector>

class Registerfile : public Register {
public:
    Registerfile();

    void Process(InstPkgPtr &) override;

    void Accept(InstPkgPtr &) override;

    void Advance() override;

private:
    std::vector<uint64_t> register_file;
};


#endif //MODEL_REGISTERFILE_HH
