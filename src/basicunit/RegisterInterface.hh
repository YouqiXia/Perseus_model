#ifndef MODEL_REGISTERINTERFACE_HH
#define MODEL_REGISTERINTERFACE_HH

#include "common/InstPkg.hh"

namespace Emulator {

    class RegisterInterface {
    public:
        RegisterInterface() = default;

        virtual ~RegisterInterface() = default;

    // ctrl port
        /**
         * @brief Reset the Register
         */
        virtual void Reset() = 0;
    // Read port
        /**
         * @brief Write instruction from inner data structure into InstPkgPtr
         */
        virtual void Produce(InstPkgPtr) = 0;

        /**
         * @brief Process instruction in InstPkg which won't generate stall signal
         */
        virtual void Process(InstPkgPtr) = 0;

        /**
         * @brief Handshake signal \n
         * @brief To see if the instruction read from Register is permitted to operate \n
         * @brief For ctrl unit, if it is not permitted, set 'stall = true' and pop it out of InstPkg \n
         * @brief For storage unit like fifo, if it is empty, set 'stall = true' and pop it out of InstPkg
         */
        virtual void SetPermission(InstPkgPtr) = 0;

    // Write port
        /**
         * @brief To see if the Register of a corresponding instruction is ready to be written into \n
         * @brief If true, set 'stall = true' and Accept if the Register is ready but do not update
         */
        virtual void Accept(InstPkgPtr) = 0;

        /**
         * @brief the Register will do Advance is according to ctrl signal \n
         * @brief Update data from instruction queue according to timing
         */
        virtual void Advance() = 0;
    };

}


#endif //MODEL_REGISTERINTERFACE_HH
