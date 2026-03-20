#pragma once

#include "cce/App.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>

namespace cce {

struct ConceptFingerprint {
    std::vector<std::string> lexicalTokens;
    std::vector<std::string> conceptTags;
    std::size_t tensionsCount{0};
    std::size_t expansionsCount{0};
    std::size_t reconfigurationsCount{0};
};

struct EmbeddingRecord {
    std::string provider;
    std::string model;
    std::vector<float> values;
};

class ConceptEmbeddingProvider {
public:
    virtual ~ConceptEmbeddingProvider() = default;
    [[nodiscard]] virtual EmbeddingRecord buildEmbedding(const ConstructionCycle& cycle) const = 0;
};

struct ConceptMemoryEntry {
    std::string scenarioId;
    std::string title;
    std::string sourceKind;
    ConstructionCycle cycle;
    ConceptFingerprint fingerprint;
    EmbeddingRecord embedding;
};

class ConceptMemory {
public:
    void store(ConceptMemoryEntry entry);
    [[nodiscard]] bool loadFromFile(const std::filesystem::path& path);
    [[nodiscard]] bool saveToFile(const std::filesystem::path& path) const;
    [[nodiscard]] const std::vector<ConceptMemoryEntry>& entries() const { return m_entries; }
    [[nodiscard]] const ConceptMemoryEntry* findByScenarioId(const std::string& scenarioId) const;
    [[nodiscard]] std::vector<MemoryMatch> findSimilar(
        const ConstructionCycle& query,
        std::size_t limit = 3) const;

private:
    std::vector<ConceptMemoryEntry> m_entries;
};

[[nodiscard]] ConceptMemory seedBuiltInConceptMemory();
[[nodiscard]] std::filesystem::path defaultConceptMemoryPath(
    const std::filesystem::path& baseDir = std::filesystem::current_path());
[[nodiscard]] ConceptMemory loadDefaultConceptMemory(
    const std::filesystem::path& baseDir = std::filesystem::current_path());
bool persistCycleToDefaultConceptMemory(
    const ConstructionCycle& cycle,
    std::string sourceKind,
    const std::filesystem::path& baseDir = std::filesystem::current_path());

} // namespace cce
