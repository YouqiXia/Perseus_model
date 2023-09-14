#ifndef REGISTER_HH_
#define REGISTER_HH_

#include "Common/InstPkg.hh"

/**
 * @brief Simulate the behavior of register(storage unit in rtl) \n
 * @brief Including ctrl(valid, ready) + data
 */
class Register {
public:
    Register();
// ctrl port
    /**
     * @brief Reset the Register
     */
    void Reset();
// Read port
    /**
     * @brief Write instruction from inner data structure into InstPkgPtr
     */
    virtual void Produce(InstPkgPtr &) {}

    /**
     * @brief Process instruction in InstPkg
     * @brief if any unit is empty, set 'stall = true' and pop it out of InstPkg
     */
    virtual void Process(InstPkgPtr &) {}

    /**
     * @brief To see if the instruction read from Register is permitted to operate \n
     * @brief If it is not permitted, set 'stall = true' and pop it out of InstPkg
     */
    virtual void SetPermission(InstPkgPtr &) {}

// Write port
    /**
     * @brief To see if the Register of a corresponding instruction is ready to be written into \n
     * @brief If true, set 'stall = true' and Accept if the Register is ready but do not update
     */
    virtual void Accept(InstPkgPtr &) {}

    /**
     * @brief the Register will do Advance is according to ctrl signal \n
     * @brief Update data from instruction queue according to timing
     */
    virtual void Advance() {}

// Private interface
    void SetTickQueue(InstPkgEntryPtr &);

    void SetWriteQueue(InstPkgEntryPtr &);

    InstPkgEntryPtr &GetTickQueue();

    InstPkgEntryPtr &GetWriteQueue();

    uint64_t GetTickQueueSize();

    uint64_t GetWriteQueueSize();

private:
    std::vector<InstPkgEntryPtr> tick_queue_;
    std::vector<InstPkgEntryPtr> write_queue_;
};

#endif // REGISTER_HH_