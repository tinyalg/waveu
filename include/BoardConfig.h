#pragma once

// Namespace tinyalg for organizing all project-related classes and functions
namespace tinyalg {
    namespace waveu {

// Base template for BoardConfig as an interface
// Specific boards will specialize this template to implement the required functions.
template <typename Board>
class BoardConfig;

    } // namespace waveu
} // namespace tinyalg
