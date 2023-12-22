#ifndef YIHAI_SIMULATION_HPP_
#define YIHAI_SIMULATION_HPP_

#include "sparta/app/Simulation.hpp"
#include "sparta/simulation/ResourceFactory.hpp"

namespace TimingModel {

    const uint64_t max_run_time = 200;

    class Simulation : public sparta::app::Simulation {
    public:
        Simulation(sparta::Scheduler& scheduler);

        virtual ~Simulation();

        void test();

    private:
        void buildTree_() override;

        //! Configure the tree and apply any last minute parameter changes
        void configureTree_() override;

        //! The tree is now configured, built, and instantiated.  We need
        //! to bind things together.
        void bindTree_() override;
    };
}

#endif // YIHAI_SIMULATION_HPP_