#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "cce/App.hpp"
#include "cce/ConceptMemory.hpp"

namespace {

struct ParsedOptions {
    std::optional<std::filesystem::path> outputDir;
    std::optional<std::filesystem::path> inputPath;
    std::string scenarioId{"knowledge-reality"};
    std::size_t limit{3};
};

std::optional<ParsedOptions> parseOptions(int argc, char** argv, int startIndex) {
    ParsedOptions options;
    for (int i = startIndex; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --output\n";
                return std::nullopt;
            }
            options.outputDir = std::filesystem::path(argv[++i]);
            continue;
        }
        if (arg == "--scenario") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --scenario\n";
                return std::nullopt;
            }
            options.scenarioId = argv[++i];
            continue;
        }
        if (arg == "--input") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --input\n";
                return std::nullopt;
            }
            options.inputPath = std::filesystem::path(argv[++i]);
            continue;
        }
        if (arg == "--limit") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --limit\n";
                return std::nullopt;
            }
            options.limit = static_cast<std::size_t>(std::stoul(argv[++i]));
            continue;
        }
        std::cerr << "Unknown option: " << arg << "\n";
        return std::nullopt;
    }
    return options;
}

int runDemo(const ParsedOptions& options) {
    std::cout << cce::renderConstructionReport(cce::runConceptualConstructionDemo(options.scenarioId));
    return 0;
}

int runRecordCycle(const ParsedOptions& options) {
    const std::filesystem::path outputDir = options.outputDir.value_or(std::filesystem::current_path() / "output");
    const cce::CycleArtifacts artifacts =
        cce::recordConceptualConstructionCycle(outputDir, options.scenarioId);

    std::cout << "Conceptual construction cycle recorded:\n";
    std::cout << "- scenario: " << options.scenarioId << "\n";
    std::cout << "- cycle: " << artifacts.cyclePath.string() << "\n";
    std::cout << "- trace: " << artifacts.tracePath.string() << "\n";
    std::cout << "- report: " << artifacts.reportPath.string() << "\n";
    return 0;
}

int runListScenarios() {
    std::cout << "Built-in scenarios:\n";
    for (const auto& scenario : cce::listBuiltInScenarios()) {
        std::cout << "- " << scenario.id << ": " << scenario.description << "\n";
    }
    return 0;
}

int runIngest(const ParsedOptions& options) {
    if (!options.inputPath.has_value()) {
        std::cerr << "Missing required --input for ingest\n";
        return 1;
    }
    const cce::ConstructionCycle cycle = cce::ingestConceptualConstructionDocument(*options.inputPath);
    cce::persistCycleToDefaultConceptMemory(cycle, "ingested-cli", std::filesystem::current_path());
    std::cout << cce::renderConstructionReport(cce::runConceptualConstructionDocument(*options.inputPath));
    return 0;
}

int runRecordIngested(const ParsedOptions& options) {
    if (!options.inputPath.has_value()) {
        std::cerr << "Missing required --input for ingest-record\n";
        return 1;
    }
    const std::filesystem::path outputDir = options.outputDir.value_or(std::filesystem::current_path() / "output");
    const cce::ConstructionCycle cycle = cce::ingestConceptualConstructionDocument(*options.inputPath);
    cce::persistCycleToDefaultConceptMemory(cycle, "ingest-record-cli", std::filesystem::current_path());
    const cce::CycleArtifacts artifacts =
        cce::recordConceptualConstructionDocument(outputDir, *options.inputPath);

    std::cout << "Ingested conceptual construction recorded:\n";
    std::cout << "- input: " << options.inputPath->string() << "\n";
    std::cout << "- cycle: " << artifacts.cyclePath.string() << "\n";
    std::cout << "- trace: " << artifacts.tracePath.string() << "\n";
    std::cout << "- report: " << artifacts.reportPath.string() << "\n";
    std::cout << "- concept-memory: "
              << cce::defaultConceptMemoryPath(std::filesystem::current_path()).string() << "\n";
    return 0;
}

int runMemoryInspect() {
    const cce::ConceptMemory memory = cce::loadDefaultConceptMemory(std::filesystem::current_path());
    std::cout << "Concept memory entries:\n";
    for (const auto& entry : memory.entries()) {
        std::cout << "- " << entry.scenarioId
                  << " | " << entry.title
                  << " | source=" << entry.sourceKind
                  << " | tensions=" << entry.fingerprint.tensionsCount
                  << " | expansions=" << entry.fingerprint.expansionsCount
                  << " | reconfigurations=" << entry.fingerprint.reconfigurationsCount
                  << "\n";
    }
    std::cout << "Memory file: "
              << cce::defaultConceptMemoryPath(std::filesystem::current_path()).string() << "\n";
    return 0;
}

int runMemoryQuery(const ParsedOptions& options) {
    const cce::ConceptMemory memory = cce::loadDefaultConceptMemory(std::filesystem::current_path());

    cce::ConstructionCycle query;
    if (options.inputPath.has_value()) {
        query = cce::ingestConceptualConstructionDocument(*options.inputPath);
    } else {
        const auto* entry = memory.findByScenarioId(options.scenarioId);
        if (!entry) {
            std::cerr << "Scenario not found in concept memory: " << options.scenarioId << "\n";
            return 1;
        }
        query = entry->cycle;
    }

    const auto matches = memory.findSimilar(query, options.limit);
    std::cout << "Concept memory matches for " << query.scenarioId << ":\n";
    for (const auto& match : matches) {
        std::cout << "- " << match.title
                  << " (" << match.scenarioId << ")"
                  << " score=" << match.score
                  << " | " << match.rationale << "\n";
    }
    if (matches.empty()) {
        std::cout << "- no matches\n";
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc <= 1) {
            return runDemo(ParsedOptions{});
        }

        const std::string_view command = argv[1];
        if (command == "demo") {
            const auto options = parseOptions(argc, argv, 2);
            if (!options.has_value()) {
                std::cerr << "Usage: cce demo [--scenario <id>]\n";
                return 1;
            }
            return runDemo(*options);
        }
        if (command == "record-cycle") {
            const auto options = parseOptions(argc, argv, 2);
            if (!options.has_value()) {
                std::cerr << "Usage: cce record-cycle [--output <path>] [--scenario <id>]\n";
                return 1;
            }
            return runRecordCycle(*options);
        }
        if (command == "list-scenarios") {
            return runListScenarios();
        }
        if (command == "ingest") {
            const auto options = parseOptions(argc, argv, 2);
            if (!options.has_value()) {
                std::cerr << "Usage: cce ingest --input <path>\n";
                return 1;
            }
            return runIngest(*options);
        }
        if (command == "ingest-record") {
            const auto options = parseOptions(argc, argv, 2);
            if (!options.has_value()) {
                std::cerr << "Usage: cce ingest-record --input <path> [--output <path>]\n";
                return 1;
            }
            return runRecordIngested(*options);
        }
        if (command == "memory-inspect") {
            return runMemoryInspect();
        }
        if (command == "memory-query") {
            const auto options = parseOptions(argc, argv, 2);
            if (!options.has_value()) {
                std::cerr << "Usage: cce memory-query [--scenario <id> | --input <path>] [--limit <n>]\n";
                return 1;
            }
            return runMemoryQuery(*options);
        }

        std::cerr << "Unknown command: " << command << "\n";
        std::cerr << "Usage: cce [demo|record-cycle|list-scenarios|ingest|ingest-record|memory-inspect|memory-query]\n";
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "CCE error: " << ex.what() << "\n";
        return 1;
    }
}
