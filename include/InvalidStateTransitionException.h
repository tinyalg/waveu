#pragma once

#include <stdexcept>
#include <string>

namespace tinyalg::waveu {

/**
 * @brief Exception for invalid state transitions in the waveform generator.
 */
class InvalidStateTransitionException : public std::logic_error {
public:
    /**
     * @brief Constructs an InvalidStateTransitionException.
     * 
     * @param message Detailed message explaining the invalid transition.
     */
    explicit InvalidStateTransitionException(const std::string& message)
        : std::logic_error(message) {}
};

} // namespace tinyalg::waveu
