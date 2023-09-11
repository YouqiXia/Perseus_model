#ifndef STAGE_HH_
#define STAGE_HH_

#include <memory>
#include <vector>
#include "temp/Register.hh"
#include "DynInst.hh"

typedef std::shared_ptr<std::vector<Register<DynInst>>> RegistersPtr;

class Stage {
public:
    // All the interface of the method of Stage should be DynInst list
    Stage(
            RegistersPtr input_queue,
            RegistersPtr output_queue
    );

    void Evaluate();

protected:
    // To see if all the data in input Register is valid
    bool IsValid() const;

    /**
     * @brief To see if all the Register is ready to be written into
     */
    void RequestReady(DynInst &) const;

    /**
     * @brief To see if some instruction read from Register is permitted to operate
     */
    void SetPermission(DynInst &) const;

    void Process(DynInst &) const;

    virtual void Execute(DynInst &) const;

    void Accept(DynInst &);

protected:
    RegistersPtr input_queue_;
    RegistersPtr output_queue_;
};


#endif //STAGE_HH_