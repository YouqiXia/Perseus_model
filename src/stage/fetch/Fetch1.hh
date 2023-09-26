#ifndef MODEL_FETCH1_HH
#define MODEL_FETCH1_HH

#include <string>
#include "basicunit/Stage.hh"


namespace Emulator {

    /**
     * @brief produce: inner \n
     * @brief process: read CSR, read branch result, pre-decode for compress
     */
    class Fetch1 : Stage {
    public:
        Fetch1(std::string name);

    protected:
        void Produce() override;

        void Execute() override;

    private:
        Addr_t pc_;
    };

}

#endif //MODEL_FETCH1_HH
