#include <cmath>
#include "PhaseGenerator.h"

namespace tinyalg::waveu {

PhaseGenerator::PhaseGenerator(uint32_t sampleRate)
    : sampleRate_(sampleRate) {}

void PhaseGenerator::setFrequency(float frequency) {
    frequency_ = frequency;
    phaseIncrement_ = (uint32_t)((double)frequency_ / (double)sampleRate_
                           * ((double)(1ULL << N_BITS)));
}

float PhaseGenerator::getFrequency() {
    return frequency_;
}

void PhaseGenerator::updatePhase() {
    phase_ += phaseIncrement_;
}

uint32_t PhaseGenerator::getPhase() const {
    return phase_;
}

void PhaseGenerator::reset() {
    phase_ = 0;
}

void PhaseGenerator::reset(double elapsedTime) {
    const uint64_t phaseScale = 1ULL << N_BITS;

    // Calculate phase in cycles and wrap to [0, 1) range
    double phaseCycles = std::fmod(frequency_ * elapsedTime, 1.0);

    // Scale the phase to fixed-point representation
    phase_ = (uint32_t)(phaseCycles * phaseScale);
}

} // namespace tinyalg::waveu
