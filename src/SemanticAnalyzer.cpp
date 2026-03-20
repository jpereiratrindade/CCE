#include "cce/SemanticAnalyzer.hpp"

#include <algorithm>
#include <set>
#include <sstream>

namespace cce {
namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        if (ch >= 'A' && ch <= 'Z') return static_cast<char>(ch - 'A' + 'a');
        return static_cast<char>(ch);
    });
    return value;
}

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

double overlapScore(const std::string& lhs, const std::string& rhs) {
    const auto a = tokenizeUnique(lhs);
    const auto b = tokenizeUnique(rhs);
    if (a.empty() || b.empty()) return 0.0;

    std::size_t intersection = 0;
    for (const auto& token : a) {
        if (b.contains(token)) ++intersection;
    }
    return static_cast<double>(intersection) / static_cast<double>(std::min(a.size(), b.size()));
}

double containsWeightedPatterns(const std::string& text, const std::vector<std::string>& patterns) {
    if (patterns.empty()) return 0.0;
    const std::string lower = toLower(text);
    std::size_t hits = 0;
    for (const auto& pattern : patterns) {
        if (lower.find(pattern) != std::string::npos) ++hits;
    }
    return static_cast<double>(hits) / static_cast<double>(patterns.size());
}

std::string joinCycleText(const ConstructionCycle& cycle) {
    std::ostringstream out;
    out << cycle.intuition.statement << ' ' << cycle.intuition.intent << ' ';
    out << cycle.stabilized.title << ' ' << cycle.stabilized.formulation << ' ' << cycle.stabilized.defense << ' ';
    for (const auto& tension : cycle.tensions) out << tension.label << ' ' << tension.description << ' ';
    for (const auto& note : cycle.expansions) out << note.perspective << ' ' << note.implication << ' ';
    for (const auto& step : cycle.reconfigurations) out << step.shift << ' ' << step.effect << ' ';
    return out.str();
}

bool hasExplicitTensionLabel(const ConstructionCycle& cycle, const std::string& needle) {
    const std::string normalized = toLower(needle);
    for (const auto& tension : cycle.tensions) {
        if (toLower(tension.label).find(normalized) != std::string::npos) return true;
        if (toLower(tension.description).find(normalized) != std::string::npos) return true;
    }
    return false;
}

} // namespace

SemanticSignal analyzeCycleSemantics(const ConstructionCycle& cycle) {
    SemanticSignal signal;
    signal.intuitionStabilizationOverlap =
        overlapScore(cycle.intuition.statement, cycle.stabilized.formulation);

    const std::string cycleText = joinCycleText(cycle);
    signal.teleologyRisk = containsWeightedPatterns(
        cycleText,
        {"necessariamente", "garante", "destino", "automaticamente", "conduz diretamente"});
    signal.normativityRisk = containsWeightedPatterns(
        cycleText,
        {"deve", "melhor", "correto", "ideal", "desejavel"});
    signal.linearityRisk = containsWeightedPatterns(
        cycleText,
        {"diretamente", "linear", "causa", "resultado automatico", "efeito garantido"});

    {
        std::ostringstream finding;
        finding << "Coerencia intuicao-estabilizacao em nivel "
                << (signal.intuitionStabilizationOverlap >= 0.5 ? "alto" :
                    signal.intuitionStabilizationOverlap >= 0.2 ? "moderado" : "baixo")
                << " (overlap=" << signal.intuitionStabilizationOverlap << ").";
        signal.semanticFindings.push_back(finding.str());
    }

    if (!hasExplicitTensionLabel(cycle, "teleologia") && signal.teleologyRisk >= 0.2) {
        signal.implicitTensions.push_back("teleologia implicita detectada semanticamente");
        signal.semanticFindings.push_back(
            "A semantica sugere teleologia implicita porque ha linguagem de resultado necessario ou automatico.");
        signal.revisionSuggestions.push_back(
            "Revisar a formulacao para substituir linguagem de necessidade ou garantia por mediacao, contingencia ou condicao.");
    }
    if (!hasExplicitTensionLabel(cycle, "normatividade") && signal.normativityRisk >= 0.2) {
        signal.implicitTensions.push_back("normatividade implicita detectada semanticamente");
        signal.semanticFindings.push_back(
            "A semantica sugere normatividade implicita porque aparecem termos de valor ou prescricao na formulacao.");
        signal.revisionSuggestions.push_back(
            "Explicitar melhor em que sentido a avaliacao de valor esta sendo usada ou substituir termos normativos por descricoes mais analiticas.");
    }
    if (!hasExplicitTensionLabel(cycle, "causalidade") && signal.linearityRisk >= 0.2) {
        signal.implicitTensions.push_back("causalidade linear implicita detectada semanticamente");
        signal.semanticFindings.push_back(
            "A semantica sugere causalidade linear implicita porque aparecem marcadores de relacao direta entre causa e resultado.");
        signal.revisionSuggestions.push_back(
            "Introduzir mediacoes, condicoes ou relacoes contextuais para evitar leitura causal direta demais.");
    }
    if (signal.intuitionStabilizationOverlap < 0.15) {
        signal.implicitTensions.push_back("baixo acoplamento semantico entre intuicao inicial e estabilizacao");
        signal.semanticFindings.push_back(
            "A formulacao final parece distante da intuicao inicial, indicando risco de deslocamento conceitual excessivo.");
        signal.revisionSuggestions.push_back(
            "Reaproximar a formulacao final dos termos nucleares da intuicao inicial sem perder o ganho conceitual.");
    }

    signal.weakStabilization =
        signal.intuitionStabilizationOverlap < 0.10 || cycle.stabilized.formulation.size() < 80;
    if (signal.weakStabilization) {
        signal.semanticFindings.push_back(
            "A estabilizacao parece fraca: a formulacao final esta curta demais ou semanticamente distante da intuicao.");
        signal.revisionSuggestions.push_back(
            "Fortalecer a estabilizacao com uma formulacao mais densa, defensavel e menos eliptica.");
    } else {
        signal.semanticFindings.push_back(
            "A estabilizacao parece robusta para esta fase: ha formulacao final suficiente e conexao semantica com a intuicao.");
    }
    return signal;
}

void enrichCycleWithSemantics(ConstructionCycle& cycle, const SemanticSignal& signal) {
    cycle.semantic = signal;

    for (const auto& implicit : signal.implicitTensions) {
        cycle.tensions.push_back({"semantic implicit", implicit});
    }

    std::ostringstream semanticSummary;
    semanticSummary << "Overlap intuicao-estabilizacao=" << signal.intuitionStabilizationOverlap
                    << ", teleologia=" << signal.teleologyRisk
                    << ", normatividade=" << signal.normativityRisk
                    << ", linearidade=" << signal.linearityRisk;
    cycle.trace.push_back({"semantic-analysis", semanticSummary.str()});

    if (signal.weakStabilization) {
        cycle.trace.push_back({"semantic-review", "A estabilizacao parece fraca ou semanticamente distante da intuicao inicial."});
    }
}

} // namespace cce
