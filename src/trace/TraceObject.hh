#ifndef TRACEOBJECT_HH_
#define TRACEOBJECT_HH_


#include <string>

namespace Emulator {
    namespace Trace {

        class TraceObject {
        private:

            const std::string name_;

        public:

            TraceObject(std::string name)
                    : name_(std::move(name)) {};

            ~TraceObject() = default;

            const std::string Name() {
                return name_;
            };

        };


    } // namespace Trace

} // namespace Emulator


#endif //TRACEOBJECT_HH_