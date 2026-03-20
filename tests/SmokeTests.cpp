#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "cce/App.hpp"
#include "cce/ConceptMemory.hpp"

namespace fs = std::filesystem;

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        std::exit(1);
    }
}

} // namespace

int main() {
    require(cce::buildEngineStatusMessage() == "CCE Conceptual Construction Engine pronto",
            "status message should reflect conceptual construction scope");

    const auto scenarios = cce::listBuiltInScenarios();
    require(scenarios.size() >= 3, "CCE should expose multiple built-in scenarios");

    const cce::ConstructionReport report = cce::runConceptualConstructionDemo();
    require(report.scenarioId == "knowledge-reality", "default scenario should stay stable");
    require(report.tensionsCount == 4, "default cycle should expose explicit and semantic tensions");
    require(report.constraintViolations == 0, "default cycle should satisfy all constraints");
    require(report.traceCount == 8, "default cycle should keep a full construction trace plus semantic analysis and memory lookup");
    require(report.passedExplicitTension, "explicit tension should be mandatory");
    require(report.passedConstraintLayer, "constraint layer should pass in the demo");
    require(report.reflexivityPresent, "reflexive layer should be present");
    require(report.intuitionStabilizationOverlap > 0.0,
            "semantic layer should estimate overlap between intuition and stabilization");
    require(report.implicitTensionsCount == 1,
            "default demo should expose one semantic implicit tension");
    require(!report.semanticFindings.empty(),
            "semantic layer should expose human-readable findings");
    require(!report.implicitTensions.empty(),
            "semantic layer should expose explicit implicit tension details");
    require(!report.memoryMatches.empty(),
            "default demo should retrieve at least one related construction from concept memory");
    require(report.decision == "stabilize", "valid conceptual construction should stabilize");
    require(!report.stabilizedFormulation.empty(), "stabilized formulation should be generated");

    const cce::ConstructionReport educationReport =
        cce::runConceptualConstructionDemo("education-formation");
    require(educationReport.scenarioId == "education-formation",
            "education scenario should be available");
    require(educationReport.decision == "stabilize",
            "education scenario should also stabilize");

    const cce::ConstructionReport socioecologicalReport =
        cce::runConceptualConstructionDemo("socioecological-reflexivity");
    require(socioecologicalReport.scenarioId == "socioecological-reflexivity",
            "socioecological scenario should be available");
    require(socioecologicalReport.tensionsCount >= 4,
            "socioecological scenario should capture richer tensions");
    require(socioecologicalReport.traceCount >= 8,
            "socioecological scenario should keep a full trace plus semantic analysis and memory lookup");
    require(socioecologicalReport.decision == "stabilize",
            "socioecological scenario should stabilize");
    require(socioecologicalReport.teleologyRisk >= 0.0,
            "semantic layer should expose teleology risk");

    const fs::path tempDir = fs::temp_directory_path() / "cce-cycle-output";
    std::error_code ec;
    fs::remove_all(tempDir, ec);

    const cce::CycleArtifacts artifacts =
        cce::recordConceptualConstructionCycle(tempDir, "knowledge-reality");
    require(fs::exists(artifacts.cyclePath), "cycle artifact should be written");
    require(fs::exists(artifacts.tracePath), "trace artifact should be written");
    require(fs::exists(artifacts.reportPath), "report artifact should be written");
    require(artifacts.cyclePath.filename() == "knowledge-reality-cycle.md",
            "cycle artifact filename should match scenario id");

    const cce::CycleArtifacts socioecologicalArtifacts =
        cce::recordConceptualConstructionCycle(tempDir, "socioecological-reflexivity");
    require(fs::exists(socioecologicalArtifacts.cyclePath),
            "socioecological cycle artifact should be written");
    require(socioecologicalArtifacts.cyclePath.filename() ==
                "socioecological-reflexivity-cycle.md",
            "socioecological artifact filename should match scenario id");

    const fs::path docPath =
        fs::weakly_canonical(fs::path(__FILE__)).parent_path().parent_path() /
        "examples" / "resilience_ontological_test_doc.md";
    const cce::ConstructionReport ingestedReport =
        cce::runConceptualConstructionDocument(docPath);
    require(ingestedReport.scenarioId == "resilience-ontological-test-doc",
            "ingested document should derive a stable scenario id from file name");
    require(ingestedReport.passedExplicitTension,
            "ingested document should expose explicit tensions");
    require(ingestedReport.reflexivityPresent,
            "ingested document should expose reflexive expansion");
    require(ingestedReport.intuitionStabilizationOverlap > 0.0,
            "ingested document should compute semantic overlap");
    require(!ingestedReport.semanticFindings.empty(),
            "ingested document should expose semantic findings");
    require(!ingestedReport.revisionSuggestions.empty(),
            "ingested document should expose actionable revision suggestions when risks are found");
    require(!ingestedReport.memoryMatches.empty(),
            "ingested document should retrieve related constructions from concept memory");
    require(ingestedReport.decision == "stabilize",
            "ingested document should stabilize");

    cce::ConceptMemory memory = cce::seedBuiltInConceptMemory();
    memory.store({
        "resilience-local",
        "Resilience Local",
        "test",
        cce::ingestConceptualConstructionDocument(docPath),
        {},
        {}
    });
    const auto similar = memory.findSimilar(cce::createConceptualConstructionDemo("socioecological-reflexivity"));
    require(!similar.empty(), "concept memory should retrieve similar local constructions");
    require(memory.findByScenarioId("resilience-local") != nullptr,
            "concept memory should allow lookup by scenario id");

    const fs::path memoryBase = fs::temp_directory_path() / "cce-memory-output";
    fs::remove_all(memoryBase, ec);
    require(cce::persistCycleToDefaultConceptMemory(
                cce::ingestConceptualConstructionDocument(docPath),
                "test-persist",
                memoryBase),
            "concept memory persistence should succeed");
    const fs::path memoryPath = cce::defaultConceptMemoryPath(memoryBase);
    require(fs::exists(memoryPath), "concept memory file should be written");
    std::ifstream memoryIn(memoryPath);
    require(memoryIn.good(), "concept memory file should be readable");

    const cce::CycleArtifacts ingestedArtifacts =
        cce::recordConceptualConstructionDocument(tempDir, docPath);
    require(fs::exists(ingestedArtifacts.cyclePath),
            "ingested cycle artifact should be written");
    require(ingestedArtifacts.cyclePath.filename() ==
                "resilience-ontological-test-doc-cycle.md",
            "ingested artifact filename should derive from document name");

    fs::remove_all(tempDir, ec);
    fs::remove_all(memoryBase, ec);

    std::cout << "OK\n";
    return 0;
}
