#pragma once

#include "Waveu.h"       // Includes the Waveu class template
#include "ESP32Config.h"   // Includes the ESP32Config struct

namespace tinyalg::waveu {

// Alias for Waveu specialized with ESP32Config
template <typename WaveConfig>
using ESP32Waveu = Waveu<ESP32Config, WaveConfig>;

}  // namespace tinyalg::waveu

#include "DataTypes.h"
#include "LUTHelper.h"
#include "PhaseGenerator.h"
#include "WaveConfig.h"
#include "WaveConfigArgs.h"
#include "WaveConfigInterface.h"
