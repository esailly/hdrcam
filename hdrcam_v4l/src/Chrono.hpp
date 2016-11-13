#ifndef CHRONO_HPP_
#define CHRONO_HPP_

#include <chrono>

using namespace std::chrono;

// chrono in milliseconds
class Chrono {

    private:

        time_point<system_clock> t0_;
        time_point<system_clock> t1_;

    public:
        Chrono() {
            t0_ = system_clock::now();
        }

        void restart() {
            t0_ = system_clock::now();
        }

        void stop() {
            t1_ = system_clock::now();
        }

        double elapsedStopped() const {
            auto d = duration_cast<milliseconds>(t1_ - t0_);
            return d.count();
        }

        double elapsedRunning() {
            t1_ = system_clock::now();
            auto d = duration_cast<milliseconds>(t1_ - t0_);
            return d.count();
        }

        double elapsedAndRestart() {
            t1_ = system_clock::now();
            auto d = duration_cast<milliseconds>(t1_ - t0_);
            t0_ = t1_;
            return d.count();
        }

};

#endif


