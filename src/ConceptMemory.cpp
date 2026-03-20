#include "cce/ConceptMemory.hpp"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace cce {
namespace {

namespace fs = std::filesystem;

std::string normalizeToken(std::string token) {
    std::string out;
    for (char ch : token) {
        const bool isAlphaNum =
            (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
        if (isAlphaNum) out.push_back(static_cast<char>((ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch));
    }
    return out;
}

std::set<std::string> tokenizeUnique(const std::string& text) {
    std::set<std::string> tokens;
    std::istringstream in(text);
    std::string raw;
    while (in >> raw) {
        std::string token = normalizeToken(raw);
        if (token.size() >= 4) tokens.insert(std::move(token));
    }
    return tokens;
}

std::string cycleSignatureText(const ConstructionCycle& cycle) {
    std::ostringstream out;
    out << cycle.intuition.statement << ' ' << cycle.intuition.intent << ' ';
    out << cycle.stabilized.title << ' ' << cycle.stabilized.formulation << ' ';
    for (const auto& tension : cycle.tensions) out << tension.label << ' ' << tension.description << ' ';
    for (const auto& note : cycle.expansions) out << note.perspective << ' ' << note.implication << ' ';
    for (const auto& step : cycle.reconfigurations) out << step.shift << ' ' << step.effect << ' ';
    return out.str();
}

std::set<std::string> deriveConceptTags(const ConstructionCycle& cycle) {
    const std::string text = cycleSignatureText(cycle);
    const std::string low = [&]() {
        std::string tmp = text;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char ch) {
            if (ch >= 'A' && ch <= 'Z') return static_cast<char>(ch - 'A' + 'a');
            return static_cast<char>(ch);
        });
        return tmp;
    }();

    const std::vector<std::pair<std::string, std::vector<std::string>>> patterns = {
        {"resilience", {"resilien", "perturb", "reconfigur", "coerenc"}},
        {"ecology", {"ecologic", "socioecolog"}},
        {"production", {"produtiv", "producao"}},
        {"social", {"social", "coletiv"}},
        {"knowledge", {"conhecimento", "knowledge"}},
        {"reflexivity", {"reflexiv", "meta-epistem", "inteligibil"}},
        {"humanities", {"humanidades", "humanit"}},
        {"formation", {"formacao", "educa", "aprendiz"}},
        {"normativity", {"normativ", "melhor", "desej"}},
        {"teleology", {"teleolog", "garant", "necessari"}}
    };

    std::set<std::string> tags;
    for (const auto& [tag, needles] : patterns) {
        for (const auto& needle : needles) {
            if (low.find(needle) != std::string::npos) {
                tags.insert(tag);
                break;
            }
        }
    }
    return tags;
}

ConceptFingerprint buildFingerprint(const ConstructionCycle& cycle) {
    ConceptFingerprint fingerprint;
    const auto lexical = tokenizeUnique(cycleSignatureText(cycle));
    fingerprint.lexicalTokens.assign(lexical.begin(), lexical.end());
    const auto tags = deriveConceptTags(cycle);
    fingerprint.conceptTags.assign(tags.begin(), tags.end());
    fingerprint.tensionsCount = cycle.tensions.size();
    fingerprint.expansionsCount = cycle.expansions.size();
    fingerprint.reconfigurationsCount = cycle.reconfigurations.size();
    return fingerprint;
}

double jaccardSimilarity(const std::set<std::string>& a, const std::set<std::string>& b) {
    if (a.empty() || b.empty()) return 0.0;
    std::size_t intersection = 0;
    for (const auto& token : a) {
        if (b.contains(token)) ++intersection;
    }
    const std::size_t uni = a.size() + b.size() - intersection;
    if (uni == 0) return 0.0;
    return static_cast<double>(intersection) / static_cast<double>(uni);
}

double structuralSimilarity(const ConstructionCycle& lhs, const ConstructionCycle& rhs) {
    const auto normalizedDelta = [](std::size_t a, std::size_t b) {
        const double maxv = static_cast<double>(std::max<std::size_t>(1, std::max(a, b)));
        return 1.0 - (static_cast<double>(std::max(a, b) - std::min(a, b)) / maxv);
    };

    const double tensions = normalizedDelta(lhs.tensions.size(), rhs.tensions.size());
    const double expansions = normalizedDelta(lhs.expansions.size(), rhs.expansions.size());
    const double reconfigurations = normalizedDelta(lhs.reconfigurations.size(), rhs.reconfigurations.size());
    return (tensions + expansions + reconfigurations) / 3.0;
}

double combinedSimilarity(const ConstructionCycle& query, const ConceptMemoryEntry& entry) {
    const double lexical = jaccardSimilarity(
        tokenizeUnique(cycleSignatureText(query)),
        std::set<std::string>(entry.fingerprint.lexicalTokens.begin(), entry.fingerprint.lexicalTokens.end()));
    const double conceptual = jaccardSimilarity(
        deriveConceptTags(query),
        std::set<std::string>(entry.fingerprint.conceptTags.begin(), entry.fingerprint.conceptTags.end()));
    const double structural = structuralSimilarity(query, entry.cycle);
    return lexical * 0.5 + conceptual * 0.35 + structural * 0.15;
}

std::string buildRationale(const ConstructionCycle& query,
                           const ConceptMemoryEntry& entry,
                           double score) {
    std::ostringstream out;
    out << "similaridade combinada=" << score;
    if (query.tensions.size() == entry.cycle.tensions.size()) {
        out << "; tensoes em volume semelhante";
    }
    if (!query.expansions.empty() && !entry.cycle.expansions.empty()) {
        out << "; ambos possuem expansao reflexiva";
    }
    const double conceptual = jaccardSimilarity(
        deriveConceptTags(query),
        std::set<std::string>(entry.fingerprint.conceptTags.begin(), entry.fingerprint.conceptTags.end()));
    if (conceptual > 0.0) {
        out << "; proximidade conceitual=" << conceptual;
    }
    return out.str();
}

std::string joinLines(const std::vector<std::string>& values) {
    std::ostringstream out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out << "\n";
        out << values[i];
    }
    return out.str();
}

std::vector<std::string> splitLinesField(const std::string& value) {
    std::vector<std::string> parts;
    std::istringstream in(value);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) parts.push_back(line);
    }
    return parts;
}

std::string escapeField(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);
    for (char ch : value) {
        switch (ch) {
            case '\\': out += "\\\\"; break;
            case '\t': out += "\\t"; break;
            case '\n': out += "\\n"; break;
            case '\r': break;
            default: out.push_back(ch); break;
        }
    }
    return out;
}

std::string unescapeField(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[i + 1];
            if (next == 't') {
                out.push_back('\t');
                ++i;
                continue;
            }
            if (next == 'n') {
                out.push_back('\n');
                ++i;
                continue;
            }
            if (next == '\\') {
                out.push_back('\\');
                ++i;
                continue;
            }
        }
        out.push_back(value[i]);
    }
    return out;
}

std::vector<std::string> splitTabFields(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '\t') {
            fields.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(line[i]);
    }
    fields.push_back(current);
    return fields;
}

} // namespace

void ConceptMemory::store(ConceptMemoryEntry entry) {
    if (entry.fingerprint.lexicalTokens.empty() && entry.fingerprint.conceptTags.empty()) {
        entry.fingerprint = buildFingerprint(entry.cycle);
    }
    for (auto& existing : m_entries) {
        if (existing.scenarioId == entry.scenarioId) {
            existing = std::move(entry);
            return;
        }
    }
    m_entries.push_back(std::move(entry));
}

bool ConceptMemory::loadFromFile(const fs::path& path) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        const auto fields = splitTabFields(line);
        if (fields.size() < 12) continue;

        ConstructionCycle cycle;
        cycle.scenarioId = unescapeField(fields[0]);
        cycle.intuition.statement = unescapeField(fields[3]);
        cycle.intuition.intent = unescapeField(fields[4]);
        cycle.stabilized.title = unescapeField(fields[1]);
        cycle.stabilized.formulation = unescapeField(fields[5]);
        cycle.stabilized.defense = unescapeField(fields[6]);

        for (const auto& item : splitLinesField(unescapeField(fields[7]))) {
            cycle.tensions.push_back({"persisted", item});
        }
        for (const auto& item : splitLinesField(unescapeField(fields[8]))) {
            cycle.expansions.push_back({"persisted", item});
        }
        for (const auto& item : splitLinesField(unescapeField(fields[9]))) {
            cycle.reconfigurations.push_back({"persisted", item});
        }

        ConceptFingerprint fingerprint;
        fingerprint.lexicalTokens = splitLinesField(unescapeField(fields[10]));
        fingerprint.conceptTags = splitLinesField(unescapeField(fields[11]));
        fingerprint.tensionsCount = cycle.tensions.size();
        fingerprint.expansionsCount = cycle.expansions.size();
        fingerprint.reconfigurationsCount = cycle.reconfigurations.size();

        store({
            cycle.scenarioId,
            unescapeField(fields[1]),
            unescapeField(fields[2]),
            std::move(cycle),
            std::move(fingerprint),
            {}
        });
    }
    return true;
}

bool ConceptMemory::saveToFile(const fs::path& path) const {
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);

    std::ofstream out(path);
    if (!out.is_open()) return false;

    for (const auto& entry : m_entries) {
        std::vector<std::string> tensions;
        for (const auto& item : entry.cycle.tensions) tensions.push_back(item.description);
        std::vector<std::string> expansions;
        for (const auto& item : entry.cycle.expansions) expansions.push_back(item.implication);
        std::vector<std::string> reconfigurations;
        for (const auto& item : entry.cycle.reconfigurations) reconfigurations.push_back(item.effect);

        out << escapeField(entry.scenarioId) << '\t'
            << escapeField(entry.title) << '\t'
            << escapeField(entry.sourceKind) << '\t'
            << escapeField(entry.cycle.intuition.statement) << '\t'
            << escapeField(entry.cycle.intuition.intent) << '\t'
            << escapeField(entry.cycle.stabilized.formulation) << '\t'
            << escapeField(entry.cycle.stabilized.defense) << '\t'
            << escapeField(joinLines(tensions)) << '\t'
            << escapeField(joinLines(expansions)) << '\t'
            << escapeField(joinLines(reconfigurations)) << '\t'
            << escapeField(joinLines(entry.fingerprint.lexicalTokens)) << '\t'
            << escapeField(joinLines(entry.fingerprint.conceptTags)) << '\n';
    }
    return out.good();
}

const ConceptMemoryEntry* ConceptMemory::findByScenarioId(const std::string& scenarioId) const {
    for (const auto& entry : m_entries) {
        if (entry.scenarioId == scenarioId) return &entry;
    }
    return nullptr;
}

std::vector<MemoryMatch> ConceptMemory::findSimilar(const ConstructionCycle& query, std::size_t limit) const {
    std::vector<MemoryMatch> matches;
    for (const auto& entry : m_entries) {
        if (entry.scenarioId == query.scenarioId) continue;

        const double score = combinedSimilarity(query, entry);
        if (score <= 0.0) continue;

        matches.push_back({
            entry.scenarioId,
            entry.title,
            score,
            buildRationale(query, entry, score)
        });
    }

    std::sort(matches.begin(), matches.end(), [](const MemoryMatch& lhs, const MemoryMatch& rhs) {
        if (lhs.score != rhs.score) return lhs.score > rhs.score;
        return lhs.scenarioId < rhs.scenarioId;
    });
    if (matches.size() > limit) matches.resize(limit);
    return matches;
}

ConceptMemory seedBuiltInConceptMemory() {
    ConceptMemory memory;
    for (const auto& scenario : listBuiltInScenarios()) {
        ConstructionCycle cycle = createConceptualConstructionDemo(scenario.id);
        memory.store({scenario.id, cycle.stabilized.title, "built-in", std::move(cycle), {}, {}});
    }
    return memory;
}

fs::path defaultConceptMemoryPath(const fs::path& baseDir) {
    return baseDir / ".cce" / "concept_memory.tsv";
}

ConceptMemory loadDefaultConceptMemory(const fs::path& baseDir) {
    ConceptMemory memory = seedBuiltInConceptMemory();
    const bool loaded = memory.loadFromFile(defaultConceptMemoryPath(baseDir));
    (void) loaded;
    return memory;
}

bool persistCycleToDefaultConceptMemory(const ConstructionCycle& cycle,
                                        std::string sourceKind,
                                        const fs::path& baseDir) {
    ConceptMemory memory = loadDefaultConceptMemory(baseDir);
    memory.store({
        cycle.scenarioId,
        cycle.stabilized.title,
        std::move(sourceKind),
        cycle,
        {},
        {}
    });
    return memory.saveToFile(defaultConceptMemoryPath(baseDir));
}

} // namespace cce
