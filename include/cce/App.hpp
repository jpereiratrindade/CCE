#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace cce {

struct SemanticSignal {
    std::vector<std::string> implicitTensions;
    std::vector<std::string> semanticFindings;
    std::vector<std::string> revisionSuggestions;
    double intuitionStabilizationOverlap{0.0};
    double teleologyRisk{0.0};
    double normativityRisk{0.0};
    double linearityRisk{0.0};
    bool weakStabilization{false};
};

struct MemoryMatch {
    std::string scenarioId;
    std::string title;
    double score{0.0};
    std::string rationale;
};

struct IntuitionSeed {
    std::string statement;
    std::string intent;
};

struct TensionFinding {
    std::string label;
    std::string description;
};

struct ConstraintCheck {
    std::string rule;
    std::string assessment;
    bool satisfied{false};
};

struct ReflexiveNote {
    std::string perspective;
    std::string implication;
};

struct ReconfigurationStep {
    std::string shift;
    std::string effect;
};

struct StabilizedConcept {
    std::string title;
    std::string formulation;
    std::string defense;
};

struct TraceEvent {
    std::string stage;
    std::string observation;
};

struct ScenarioDescriptor {
    std::string id;
    std::string label;
    std::string description;
};

struct ConstructionCycle {
    std::string scenarioId;
    IntuitionSeed intuition;
    std::vector<TensionFinding> tensions;
    std::vector<ConstraintCheck> constraints;
    std::vector<ReflexiveNote> expansions;
    std::vector<ReconfigurationStep> reconfigurations;
    StabilizedConcept stabilized;
    std::vector<TraceEvent> trace;
    SemanticSignal semantic;
};

struct ConstructionReport {
    std::string scenarioId;
    std::string scenarioLabel;
    std::string initialStatement;
    std::string stabilizedTitle;
    std::string stabilizedFormulation;
    std::string decision;
    std::size_t tensionsCount{0};
    std::size_t constraintViolations{0};
    std::size_t traceCount{0};
    bool passedExplicitTension{false};
    bool passedConstraintLayer{false};
    bool reflexivityPresent{false};
    double intuitionStabilizationOverlap{0.0};
    double teleologyRisk{0.0};
    double normativityRisk{0.0};
    double linearityRisk{0.0};
    bool weakStabilization{false};
    std::size_t implicitTensionsCount{0};
    std::vector<std::string> semanticFindings;
    std::vector<std::string> implicitTensions;
    std::vector<std::string> revisionSuggestions;
    std::vector<MemoryMatch> memoryMatches;
};

struct CycleArtifacts {
    std::filesystem::path cyclePath;
    std::filesystem::path tracePath;
    std::filesystem::path reportPath;
};

[[nodiscard]] std::string buildEngineStatusMessage();
[[nodiscard]] std::vector<ScenarioDescriptor> listBuiltInScenarios();
[[nodiscard]] ConstructionCycle createConceptualConstructionDemo(
    std::string_view scenarioId = "knowledge-reality");
[[nodiscard]] ConstructionReport runConceptualConstructionDemo(
    std::string_view scenarioId = "knowledge-reality");
[[nodiscard]] ConstructionCycle ingestConceptualConstructionDocument(const std::filesystem::path& inputPath);
[[nodiscard]] ConstructionReport runConceptualConstructionDocument(const std::filesystem::path& inputPath);
[[nodiscard]] std::string renderConstructionReport(const ConstructionReport& report);
[[nodiscard]] CycleArtifacts recordConceptualConstructionCycle(
    const std::filesystem::path& outputDir,
    std::string_view scenarioId = "knowledge-reality");
[[nodiscard]] CycleArtifacts recordConceptualConstructionDocument(
    const std::filesystem::path& outputDir,
    const std::filesystem::path& inputPath);

} // namespace cce
