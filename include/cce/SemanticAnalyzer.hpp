#pragma once

#include "cce/App.hpp"

namespace cce {

[[nodiscard]] SemanticSignal analyzeCycleSemantics(const ConstructionCycle& cycle);
void enrichCycleWithSemantics(ConstructionCycle& cycle, const SemanticSignal& signal);

} // namespace cce
