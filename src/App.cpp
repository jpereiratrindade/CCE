#include "cce/App.hpp"
#include "cce/ConceptMemory.hpp"
#include "cce/SemanticAnalyzer.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace cce {
namespace {

constexpr std::string_view kDefaultScenarioId = "knowledge-reality";

ScenarioDescriptor findScenarioDescriptor(std::string_view scenarioId) {
    for (const auto& scenario : listBuiltInScenarios()) {
        if (scenario.id == scenarioId) return scenario;
    }
    return {
        std::string(scenarioId.empty() ? kDefaultScenarioId : scenarioId),
        "Custom Scenario",
        "Cenario sem descriptor explicito."
    };
}

void ensureDirectory(const fs::path& path) {
    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec) {
        throw std::runtime_error("failed to create directory: " + path.string());
    }
}

void writeTextFile(const fs::path& path, const std::string& content) {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open output file: " + path.string());
    }
    out << content;
    if (!out.good()) {
        throw std::runtime_error("failed to write output file: " + path.string());
    }
}

std::string trim(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string slugify(const std::string& value) {
    std::string out;
    bool lastDash = false;
    for (char ch : value) {
        const bool isAlphaNum =
            (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
        if (isAlphaNum) {
            out.push_back(static_cast<char>((ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch));
            lastDash = false;
        } else if (!lastDash && !out.empty()) {
            out.push_back('-');
            lastDash = true;
        }
    }
    while (!out.empty() && out.back() == '-') out.pop_back();
    return out.empty() ? "ingested-document" : out;
}

std::string readTextFile(const fs::path& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("failed to open input file: " + path.string());
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

std::map<std::string, std::vector<std::string>> parseMarkdownSections(const std::string& content) {
    std::map<std::string, std::vector<std::string>> sections;
    std::string currentSection = "preamble";
    for (const auto& rawLine : splitLines(content)) {
        const std::string line = trim(rawLine);
        if (line.rfind("### ", 0) == 0) {
            currentSection = trim(line.substr(4));
            continue;
        }
        if (line.rfind("## ", 0) == 0 || line.rfind("# ", 0) == 0) {
            continue;
        }
        sections[currentSection].push_back(rawLine);
    }
    return sections;
}

std::vector<std::string> collectBulletItems(const std::vector<std::string>& lines) {
    std::vector<std::string> items;
    for (const auto& rawLine : lines) {
        const std::string line = trim(rawLine);
        if (line.rfind("- ", 0) == 0) items.push_back(trim(line.substr(2)));
    }
    return items;
}

std::string extractFirstParagraph(const std::vector<std::string>& lines) {
    std::ostringstream out;
    bool started = false;
    for (const auto& rawLine : lines) {
        const std::string line = trim(rawLine);
        if (line.empty()) {
            if (started) break;
            continue;
        }
        if (line.rfind("- ", 0) == 0) continue;
        if (started) out << ' ';
        out << line;
        started = true;
    }
    return out.str();
}

std::string extractQuotedSingleLine(const std::vector<std::string>& lines) {
    for (const auto& rawLine : lines) {
        const std::string line = trim(rawLine);
        if (!line.empty()) return line;
    }
    return {};
}

std::string extractDocumentTitle(const std::string& content, const fs::path& inputPath) {
    for (const auto& rawLine : splitLines(content)) {
        const std::string line = trim(rawLine);
        if (line.rfind("## ", 0) == 0) return trim(line.substr(3));
    }
    return trim(inputPath.stem().string());
}

std::string renderCycleArtifact(const ConstructionCycle& cycle, const ScenarioDescriptor& scenario) {
    std::ostringstream out;
    out << "# Conceptual Construction Cycle\n\n";
    out << "## 1. Genesis of the Problem\n";
    out << "- scenario_id: " << scenario.id << "\n";
    out << "- scenario_label: " << scenario.label << "\n";
    out << "- intuition_statement: " << cycle.intuition.statement << "\n";
    out << "- intuition_intent: " << cycle.intuition.intent << "\n";

    out << "\n## 2. Tension Engine\n";
    for (const auto& tension : cycle.tensions) {
        out << "- " << tension.label << ": " << tension.description << "\n";
    }

    out << "\n## 3. Constraint Layer\n";
    for (const auto& constraint : cycle.constraints) {
        out << "- " << constraint.rule << ": " << constraint.assessment
            << " [" << (constraint.satisfied ? "ok" : "violated") << "]\n";
    }

    out << "\n## 4. Reflexive Expansion\n";
    for (const auto& note : cycle.expansions) {
        out << "- " << note.perspective << ": " << note.implication << "\n";
    }

    out << "\n## 5. Reconfiguration\n";
    for (const auto& step : cycle.reconfigurations) {
        out << "- " << step.shift << ": " << step.effect << "\n";
    }

    out << "\n## 6. Stabilization\n";
    out << "- stabilized_title: " << cycle.stabilized.title << "\n";
    out << "- stabilized_formulation: " << cycle.stabilized.formulation << "\n";
    out << "- defense: " << cycle.stabilized.defense << "\n";

    out << "\n## 7. Semantic Analysis\n";
    out << "- intuition_stabilization_overlap: " << cycle.semantic.intuitionStabilizationOverlap << "\n";
    out << "- teleology_risk: " << cycle.semantic.teleologyRisk << "\n";
    out << "- normativity_risk: " << cycle.semantic.normativityRisk << "\n";
    out << "- linearity_risk: " << cycle.semantic.linearityRisk << "\n";
    out << "- weak_stabilization: " << (cycle.semantic.weakStabilization ? "yes" : "no") << "\n";
    out << "- implicit_tensions: " << cycle.semantic.implicitTensions.size() << "\n";
    for (const auto& implicit : cycle.semantic.implicitTensions) {
        out << "  - implicit_tension: " << implicit << "\n";
    }
    for (const auto& finding : cycle.semantic.semanticFindings) {
        out << "  - finding: " << finding << "\n";
    }
    for (const auto& suggestion : cycle.semantic.revisionSuggestions) {
        out << "  - revision_suggestion: " << suggestion << "\n";
    }
    out << "- memory_note: consult report output for ranked concept memory matches\n";

    return out.str();
}

std::string renderTraceArtifact(const ConstructionCycle& cycle) {
    std::ostringstream out;
    out << "# Construction Trace\n\n";
    for (const auto& event : cycle.trace) {
        out << "- " << event.stage << ": " << event.observation << "\n";
    }
    return out.str();
}

std::size_t countConstraintViolations(const ConstructionCycle& cycle) {
    std::size_t violations = 0;
    for (const auto& constraint : cycle.constraints) {
        if (!constraint.satisfied) ++violations;
    }
    return violations;
}

ConstructionReport buildReportFromCycle(const ConstructionCycle& cycle, std::vector<MemoryMatch> memoryMatches) {
    const ScenarioDescriptor scenario = findScenarioDescriptor(cycle.scenarioId);

    ConstructionReport report;
    report.scenarioId = cycle.scenarioId;
    report.scenarioLabel = scenario.label == "Custom Scenario" ? cycle.stabilized.title : scenario.label;
    report.initialStatement = cycle.intuition.statement;
    report.stabilizedTitle = cycle.stabilized.title;
    report.stabilizedFormulation = cycle.stabilized.formulation;
    report.tensionsCount = cycle.tensions.size();
    report.constraintViolations = countConstraintViolations(cycle);
    report.traceCount = cycle.trace.size();
    report.passedExplicitTension = !cycle.tensions.empty();
    report.passedConstraintLayer = report.constraintViolations == 0;
    report.reflexivityPresent = !cycle.expansions.empty();
    report.intuitionStabilizationOverlap = cycle.semantic.intuitionStabilizationOverlap;
    report.teleologyRisk = cycle.semantic.teleologyRisk;
    report.normativityRisk = cycle.semantic.normativityRisk;
    report.linearityRisk = cycle.semantic.linearityRisk;
    report.weakStabilization = cycle.semantic.weakStabilization;
    report.implicitTensionsCount = cycle.semantic.implicitTensions.size();
    report.semanticFindings = cycle.semantic.semanticFindings;
    report.implicitTensions = cycle.semantic.implicitTensions;
    report.revisionSuggestions = cycle.semantic.revisionSuggestions;
    report.memoryMatches = std::move(memoryMatches);
    report.decision = report.passedExplicitTension && report.passedConstraintLayer && report.reflexivityPresent
                          ? "stabilize"
                          : "iterate";
    return report;
}

void enrichCycleWithMemory(ConstructionCycle& cycle, const std::vector<MemoryMatch>& matches) {
    if (matches.empty()) {
        cycle.trace.push_back({"concept-memory", "Nenhuma construcao proxima foi recuperada na memoria local."});
        return;
    }

    std::ostringstream out;
    out << "Recuperadas " << matches.size() << " construcao(oes) proximas: ";
    for (std::size_t i = 0; i < matches.size(); ++i) {
        if (i > 0) out << "; ";
        out << matches[i].scenarioId << " score=" << matches[i].score;
    }
    cycle.trace.push_back({"concept-memory", out.str()});
}

ConstructionCycle buildKnowledgeRealityScenario() {
    ConstructionCycle cycle;
    cycle.scenarioId = "knowledge-reality";
    cycle.intuition = {
        "Conhecimento transforma realidade.",
        "Preservar a intuicao de que o conhecimento importa sem cair em causalidade linear."
    };
    cycle.tensions = {
        {
            "generalizacao",
            "A afirmacao pressupoe que qualquer conhecimento produz transformacao observavel."
        },
        {
            "teleologia",
            "A formulacao sugere uma direcao necessaria e positiva da transformacao."
        },
        {
            "instrumentalismo",
            "Conhecimento aparece apenas como ferramenta e perde sua espessura historica e social."
        }
    };
    cycle.constraints = {
        {
            "evitar teleologia implicita",
            "A formulacao final precisa abandonar a ideia de destino automatico do conhecimento.",
            true
        },
        {
            "evitar causalidade linear simplista",
            "A relacao entre conhecimento e realidade deve ser mediada por praticas, instituicoes e conflito.",
            true
        },
        {
            "evitar normatividade implicita",
            "A saida nao deve pressupor que toda transformacao e desejavel.",
            true
        }
    };
    cycle.expansions = {
        {
            "pressupostos",
            "O enunciado inicial depende de uma nocao nao explicitada de realidade socialmente configurada."
        },
        {
            "implicacoes",
            "Se conhecimento participa da configuracao do real, entao seus efeitos sao disputados e situados."
        },
        {
            "limites",
            "Nem todo conhecimento se traduz em mudanca; mediacoes materiais e politicas podem bloquea-lo."
        }
    };
    cycle.reconfigurations = {
        {
            "deslocar foco",
            "Trocar uma logica de impacto direto por uma logica de participacao em processos de configuracao."
        },
        {
            "redefinir termos",
            "Conhecimento deixa de ser agente unico e passa a ser operador entre praticas e interpretacoes."
        },
        {
            "alterar estrutura logica",
            "Substituir a forma sujeito-verbo-objeto por uma formulacao relacional e condicional."
        }
    };
    cycle.stabilized = {
        "Conhecimento como operador de configuracao",
        "O conhecimento nao transforma a realidade de modo direto ou necessario; ele participa da reconfiguracao do real quando se articula com praticas, instituicoes e disputas de sentido.",
        "A formulacao preserva a intuicao inicial, mas torna explicitas as mediacoes e recusa teleologia."
    };
    cycle.trace = {
        {"intuition", "A intuicao inicial foi registrada sem correcoes prematuras."},
        {"tension", "A frase foi tensionada contra generalizacao, teleologia e instrumentalismo."},
        {"constraint", "As restricoes bloquearam uma saida causal e normativamente otimista."},
        {"expansion", "Foram introduzidos pressupostos, implicacoes e limites em chave reflexiva."},
        {"reconfiguration", "O enunciado foi reconstruido com logica relacional e mediada."},
        {"stabilization", "A formula final ficou defensavel sem perder diferenca conceitual."}
    };
    return cycle;
}

ConstructionCycle buildEducationScenario() {
    ConstructionCycle cycle;
    cycle.scenarioId = "education-formation";
    cycle.intuition = {
        "Formacao humanistica produz pessoas melhores.",
        "Salvar a importancia formativa das Humanidades sem moralismo simplista."
    };
    cycle.tensions = {
        {
            "moralismo",
            "A frase identifica automaticamente educacao humanistica com melhora moral."
        },
        {
            "homogeneizacao",
            "Ignora trajetorias, contextos e apropriacoes divergentes da experiencia formativa."
        },
        {
            "efeito garantido",
            "Supoe que a formacao gera um resultado previsivel e uniforme."
        }
    };
    cycle.constraints = {
        {
            "evitar teleologia implicita",
            "A saida nao pode apresentar a formacao como caminho necessario para um tipo ideal de pessoa.",
            true
        },
        {
            "evitar causalidade linear simplista",
            "A relacao entre experiencia formativa e transformacao subjetiva precisa aparecer como aberta.",
            true
        },
        {
            "evitar normatividade implicita",
            "A linguagem final nao deve impor um padrao unico de 'melhor'.",
            true
        }
    };
    cycle.expansions = {
        {
            "pressupostos",
            "A intuicao opera com um ideal de pessoa melhor que precisa ser explicitado e problematizado."
        },
        {
            "implicacoes",
            "A formacao humanistica pode ampliar repertorios de interpretacao e julgamento sem determinar conduta."
        },
        {
            "limites",
            "Experiencias formativas tambem podem ser neutralizadas, recusadas ou instrumentalizadas."
        }
    };
    cycle.reconfigurations = {
        {
            "mudar foco",
            "Sair da promessa de melhoramento e entrar na ampliacao de capacidade interpretativa."
        },
        {
            "redefinir o resultado",
            "Trocar 'pessoas melhores' por sujeitos mais capazes de examinar conflitos e pressupostos."
        }
    };
    cycle.stabilized = {
        "Formacao humanistica como ampliacao reflexiva",
        "A formacao humanistica nao produz automaticamente pessoas melhores; ela pode ampliar a capacidade de interpretar conflitos, revisar pressupostos e responder de modo menos imediato ao mundo.",
        "A saida conserva valor formativo, mas evita moralismo e determinismo."
    };
    cycle.trace = {
        {"intuition", "A intuicao foi preservada como semente e nao como tese pronta."},
        {"tension", "O motor de tensao expôs moralismo, homogeneizacao e efeito garantido."},
        {"constraint", "As restricoes mantiveram a saida livre de teleologia e normatividade implicita."},
        {"expansion", "O plano reflexivo introduziu pressupostos e limites da experiencia formativa."},
        {"reconfiguration", "O eixo passou de melhoramento moral para ampliacao reflexiva."},
        {"stabilization", "A formula final ficou conceitualmente mais robusta e menos trivial."}
    };
    return cycle;
}

ConstructionCycle buildSocioecologicalScenario() {
    ConstructionCycle cycle;
    cycle.scenarioId = "socioecological-reflexivity";
    cycle.intuition = {
        "O conhecimento possui potencial para criar condicoes que permitam as pessoas transformar suas realidades.",
        "Preservar a intuicao de abertura do possivel sem cair em instrumentalismo, teleologia ou prescricao normativa."
    };
    cycle.tensions = {
        {
            "instrumentalismo",
            "A formula inicial trata o conhecimento como recurso funcional para produzir mudanca, sem qualificar seus modos."
        },
        {
            "teleologia",
            "A relacao entre conhecimento e transformacao parece direta, linear e implicitamente positiva."
        },
        {
            "normatividade difusa",
            "A formulacao sugere emancipacao como resultado esperado sem explicitar condicoes, limites e mediacoes."
        },
        {
            "cegueira sistemica",
            "Fica ausente a observacao de que certos regimes de conhecimento operam dentro dos limites dos sistemas que sustentam."
        }
    };
    cycle.constraints = {
        {
            "evitar teleologia implicita",
            "A formula final nao pode afirmar que conhecimento conduz necessariamente a transformacao social.",
            true
        },
        {
            "evitar causalidade linear simplista",
            "A passagem entre conhecimento e mudanca precisa ser mediada por regimes epistemicos, interpretacao e acao situada.",
            true
        },
        {
            "evitar normatividade implicita",
            "A saida nao deve prometer solucao ou emancipacao garantida.",
            true
        }
    };
    cycle.expansions = {
        {
            "pressupostos epistemologicos",
            "Nos sistemas socioecologicos predomina frequentemente um conhecimento instrumentalizado, orientado pela eficiencia e associado aos meios de producao."
        },
        {
            "limites estruturais",
            "Esse regime tende a operar dentro dos proprios limites dos sistemas que sustenta, restringindo a critica de seus fundamentos e efeitos."
        },
        {
            "operacao das Humanidades",
            "As Humanidades introduzem reflexividade critica ao incidir sobre pressupostos, limites e condicoes de producao do conhecimento."
        },
        {
            "efeito epistemologico",
            "Ao tornar explicito o que estava naturalizado, a reflexividade desestabiliza as praticas vigentes e amplia as condicoes de inteligibilidade."
        },
        {
            "abertura do possivel",
            "Esse movimento nao entrega solucao direta, mas cria condicoes para a emergencia de configuracoes de acao antes impensaveis."
        }
    };
    cycle.reconfigurations = {
        {
            "deslocar o centro do argumento",
            "Sair do conhecimento em geral e entrar na distincao entre modos de conhecimento e seus regimes de operacao."
        },
        {
            "substituir transformacao direta por possibilidade condicionada",
            "A mudanca deixa de aparecer como efeito garantido e passa a depender da reconfiguracao dos modos de compreender a realidade."
        },
        {
            "introduzir nivel meta-epistemologico",
            "A relevancia das Humanidades e formulada como capacidade de produzir reflexividade critica, nao como fornecimento imediato de solucao."
        },
        {
            "redefinir o resultado",
            "O argumento termina na ampliacao das condicoes do possivel, nao em promessa de transformacao assegurada."
        }
    };
    cycle.stabilized = {
        "Reflexividade critica em sistemas socioecologicos",
        "Nos sistemas socioecologicos, formas instrumentalizadas de conhecimento tendem a operar dentro dos limites dos sistemas que sustentam. A incorporacao de perspectivas das Humanidades introduz reflexividade critica, explicita pressupostos, limites e implicacoes, reconfigura as condicoes de inteligibilidade da realidade e amplia as condicoes sob as quais novas configuracoes de acao podem emergir.",
        "A formulacao final abandona causalidade direta e mantem apenas uma logica de possibilidade condicionada: a reflexividade critica nao garante transformacao, mas amplia as condicoes em que ela pode tornar-se possivel."
    };
    cycle.trace = {
        {"intuition", "A intuicao inicial foi registrada como potencial de transformacao sem validacao apressada."},
        {"tension", "O argumento foi tensionado contra instrumentalismo, teleologia, normatividade difusa e cegueira sistemica."},
        {"constraint", "As restricoes impediram qualquer formula final com promessa causal direta ou emancipacao garantida."},
        {"expansion", "A expansao reflexiva introduziu sistemas socioecologicos, conhecimento instrumentalizado, Humanidades e naturalizacao."},
        {"reconfiguration", "O eixo mudou de transformacao direta da realidade para reconfiguracao das condicoes de inteligibilidade."},
        {"stabilization", "A tese foi consolidada como ampliacao do possivel por reflexividade critica, e nao como solucao prescritiva."}
    };
    return cycle;
}

} // namespace

std::string buildEngineStatusMessage() {
    return "CCE Conceptual Construction Engine pronto";
}

std::vector<ScenarioDescriptor> listBuiltInScenarios() {
    return {
        {
            "knowledge-reality",
            "Knowledge and Reality",
            "Parte da intuicao 'conhecimento transforma realidade' e a reconstrói como formulacao mediada."
        },
        {
            "education-formation",
            "Education and Formation",
            "Reformula a intuicao de melhoramento humano via formacao humanistica em chave reflexiva."
        },
        {
            "socioecological-reflexivity",
            "Socioecological Reflexivity",
            "Materializa o argumento sobre conhecimento instrumentalizado, Humanidades e ampliacao das condicoes do possivel."
        }
    };
}

ConstructionCycle createConceptualConstructionDemo(std::string_view scenarioId) {
    if (scenarioId == "socioecological-reflexivity") {
        return buildSocioecologicalScenario();
    }
    if (scenarioId == "education-formation") {
        return buildEducationScenario();
    }
    return buildKnowledgeRealityScenario();
}

ConstructionCycle ingestConceptualConstructionDocument(const fs::path& inputPath) {
    const std::string content = readTextFile(inputPath);
    const auto sections = parseMarkdownSections(content);

    const std::string title = extractDocumentTitle(content, inputPath);
    ConstructionCycle cycle;
    cycle.scenarioId = slugify(inputPath.stem().string());
    cycle.intuition = {
        extractFirstParagraph(sections.contains("1. Intuicao Inicial") ? sections.at("1. Intuicao Inicial")
                                                                       : std::vector<std::string>{}),
        extractFirstParagraph(sections.contains("3. Hipotese de Construcao") ? sections.at("3. Hipotese de Construcao")
                                                                             : std::vector<std::string>{})
    };

    if (cycle.intuition.statement.empty()) {
        cycle.intuition.statement = extractFirstParagraph(sections.contains("preamble") ? sections.at("preamble")
                                                                                        : std::vector<std::string>{});
    }

    const auto problemItems = collectBulletItems(
        sections.contains("2. Problema Ontologico") ? sections.at("2. Problema Ontologico")
                                                    : std::vector<std::string>{});
    for (const auto& item : problemItems) {
        cycle.tensions.push_back({"problema ontologico", item});
    }

    const auto firstTensionItems = collectBulletItems(
        sections.contains("5. Primeira Tensao") ? sections.at("5. Primeira Tensao")
                                                : std::vector<std::string>{});
    for (const auto& item : firstTensionItems) {
        cycle.tensions.push_back({"primeira tensao", item});
    }

    const auto secondTensionItems = collectBulletItems(
        sections.contains("6. Segunda Tensao") ? sections.at("6. Segunda Tensao")
                                               : std::vector<std::string>{});
    for (const auto& item : secondTensionItems) {
        cycle.tensions.push_back({"segunda tensao", item});
    }

    cycle.constraints = {
        {
            "evitar teleologia implicita",
            "A formulacao ingerida nao deve reduzir resiliencia a retorno garantido ou resultado automatico.",
            !cycle.tensions.empty()
        },
        {
            "evitar causalidade linear simplista",
            "A formulacao final precisa manter mediacao entre coerencia, reconfiguracao e perturbacao.",
            true
        },
        {
            "preservar emergencia relacional",
            "A saida final deve manter resiliencia como dimensao emergente, nao como atributo isolado.",
            true
        }
    };

    const auto dimensionalItems = collectBulletItems(
        sections.contains("4. Campo Dimensional") ? sections.at("4. Campo Dimensional")
                                                  : std::vector<std::string>{});
    for (const auto& item : dimensionalItems) {
        cycle.expansions.push_back({"campo dimensional", item});
    }

    const std::string dimensionalParagraph = extractFirstParagraph(
        sections.contains("4. Campo Dimensional") ? sections.at("4. Campo Dimensional")
                                                  : std::vector<std::string>{});
    if (!dimensionalParagraph.empty()) {
        cycle.expansions.push_back({"campo dimensional", dimensionalParagraph});
    }

    const auto implicationItems = collectBulletItems(
        sections.contains("9. Implicacoes Ontologicas") ? sections.at("9. Implicacoes Ontologicas")
                                                        : std::vector<std::string>{});
    for (const auto& item : implicationItems) {
        cycle.expansions.push_back({"implicacoes ontologicas", item});
    }

    const auto reconfigurationItems = collectBulletItems(
        sections.contains("7. Deslocamento Conceitual") ? sections.at("7. Deslocamento Conceitual")
                                                        : std::vector<std::string>{});
    for (const auto& item : reconfigurationItems) {
        cycle.reconfigurations.push_back({"deslocamento conceitual", item});
    }

    const std::string reconfigurationParagraph = extractFirstParagraph(
        sections.contains("7. Deslocamento Conceitual") ? sections.at("7. Deslocamento Conceitual")
                                                        : std::vector<std::string>{});
    if (!reconfigurationParagraph.empty()) {
        cycle.reconfigurations.push_back({"efeito de reconfiguracao", reconfigurationParagraph});
    }

    const std::string strongerFormulation = extractFirstParagraph(
        sections.contains("10. Formulacao Mais Forte") ? sections.at("10. Formulacao Mais Forte")
                                                       : std::vector<std::string>{});
    const std::string closingLine = extractQuotedSingleLine(
        sections.contains("11. Fecho") ? sections.at("11. Fecho")
                                       : std::vector<std::string>{});
    const auto closingItems = collectBulletItems(
        sections.contains("11. Fecho") ? sections.at("11. Fecho")
                                       : std::vector<std::string>{});

    std::ostringstream defense;
    if (!closingLine.empty()) defense << closingLine;
    if (!closingItems.empty()) {
        if (defense.tellp() > 0) defense << " Preserva: ";
        for (std::size_t i = 0; i < closingItems.size(); ++i) {
            if (i > 0) defense << ", ";
            defense << closingItems[i];
        }
        defense << ".";
    }

    cycle.stabilized = {
        title.empty() ? "Ingested Conceptual Construction" : title,
        !strongerFormulation.empty() ? strongerFormulation
                                     : extractFirstParagraph(
                                           sections.contains("8. Formulacao Provisoria")
                                               ? sections.at("8. Formulacao Provisoria")
                                               : std::vector<std::string>{}),
        defense.str()
    };

    if (cycle.stabilized.formulation.empty()) {
        cycle.stabilized.formulation = cycle.intuition.statement;
    }
    if (cycle.stabilized.defense.empty()) {
        cycle.stabilized.defense = "Documento ingerido e estabilizado a partir da estrutura argumentativa identificada.";
    }

    cycle.trace = {
        {"ingest", "Documento Markdown carregado de " + inputPath.string() + "."},
        {"intuition", "A intuicao inicial foi extraida da secao 'Intuicao Inicial'."},
        {"tension", "Tensoes foram inferidas a partir de problema ontologico e secoes de tensao explicita."},
        {"constraint", "Restricoes padrao do CCE foram aplicadas ao documento ingerido."},
        {"expansion", "Campo dimensional e implicacoes ontologicas foram incorporados como expansao reflexiva."},
        {"reconfiguration", "Deslocamentos conceituais foram convertidos em passos de reconfiguracao."},
        {"stabilization", "A formulacao mais forte do documento foi promovida a estabilizacao."}
    };

    return cycle;
}

ConstructionReport runConceptualConstructionDemo(std::string_view scenarioId) {
    ConstructionCycle cycle = createConceptualConstructionDemo(
        scenarioId.empty() ? kDefaultScenarioId : scenarioId);
    const SemanticSignal semantic = analyzeCycleSemantics(cycle);
    enrichCycleWithSemantics(cycle, semantic);
    ConceptMemory memory = loadDefaultConceptMemory(fs::current_path());
    const auto matches = memory.findSimilar(cycle);
    enrichCycleWithMemory(cycle, matches);
    return buildReportFromCycle(cycle, matches);
}

ConstructionReport runConceptualConstructionDocument(const fs::path& inputPath) {
    ConstructionCycle cycle = ingestConceptualConstructionDocument(inputPath);
    const SemanticSignal semantic = analyzeCycleSemantics(cycle);
    enrichCycleWithSemantics(cycle, semantic);
    ConceptMemory memory = loadDefaultConceptMemory(fs::current_path());
    const auto matches = memory.findSimilar(cycle);
    enrichCycleWithMemory(cycle, matches);
    return buildReportFromCycle(cycle, matches);
}

std::string renderConstructionReport(const ConstructionReport& report) {
    std::ostringstream out;
    out << buildEngineStatusMessage() << "\n";
    out << "Scenario: " << report.scenarioLabel << " (" << report.scenarioId << ")\n";
    out << "Initial intuition: " << report.initialStatement << "\n";
    out << "Stabilized title: " << report.stabilizedTitle << "\n";
    out << "Stabilized formulation: " << report.stabilizedFormulation << "\n";
    out << "Method path: intuition -> tension -> constraint -> reflexive expansion -> reconfiguration -> stabilization\n";
    out << "Tensions: " << report.tensionsCount << "\n";
    out << "Implicit tensions: " << report.implicitTensionsCount << "\n";
    out << "Constraint violations: " << report.constraintViolations << "\n";
    out << "Trace events: " << report.traceCount << "\n";
    out << "Intuition/stabilization overlap: " << report.intuitionStabilizationOverlap << "\n";
    out << "Teleology risk: " << report.teleologyRisk << "\n";
    out << "Normativity risk: " << report.normativityRisk << "\n";
    out << "Linearity risk: " << report.linearityRisk << "\n";
    out << "Weak stabilization: " << (report.weakStabilization ? "yes" : "no") << "\n";
    for (const auto& implicit : report.implicitTensions) {
        out << "Implicit tension detail: " << implicit << "\n";
    }
    for (const auto& finding : report.semanticFindings) {
        out << "Semantic finding: " << finding << "\n";
    }
    for (const auto& suggestion : report.revisionSuggestions) {
        out << "Revision suggestion: " << suggestion << "\n";
    }
    for (const auto& match : report.memoryMatches) {
        out << "Memory match: " << match.title << " (" << match.scenarioId << ") score="
            << match.score << " [" << match.rationale << "]\n";
    }
    out << "Decision: " << report.decision << "\n";
    return out.str();
}

CycleArtifacts recordConceptualConstructionCycle(const fs::path& outputDir, std::string_view scenarioId) {
    ConstructionCycle cycle = createConceptualConstructionDemo(
        scenarioId.empty() ? kDefaultScenarioId : scenarioId);
    const SemanticSignal semantic = analyzeCycleSemantics(cycle);
    enrichCycleWithSemantics(cycle, semantic);
    ConceptMemory memory = loadDefaultConceptMemory(fs::current_path());
    const auto matches = memory.findSimilar(cycle);
    enrichCycleWithMemory(cycle, matches);
    const ScenarioDescriptor scenario = findScenarioDescriptor(cycle.scenarioId);
    const ConstructionReport report = buildReportFromCycle(cycle, matches);

    ensureDirectory(outputDir);

    const fs::path cyclePath = outputDir / (cycle.scenarioId + std::string("-cycle.md"));
    const fs::path tracePath = outputDir / (cycle.scenarioId + std::string("-trace.md"));
    const fs::path reportPath = outputDir / (cycle.scenarioId + std::string("-report.md"));

    writeTextFile(cyclePath, renderCycleArtifact(cycle, scenario));
    writeTextFile(tracePath, renderTraceArtifact(cycle));
    writeTextFile(reportPath, renderConstructionReport(report));

    return {cyclePath, tracePath, reportPath};
}

CycleArtifacts recordConceptualConstructionDocument(const fs::path& outputDir, const fs::path& inputPath) {
    ConstructionCycle cycle = ingestConceptualConstructionDocument(inputPath);
    const SemanticSignal semantic = analyzeCycleSemantics(cycle);
    enrichCycleWithSemantics(cycle, semantic);
    ConceptMemory memory = loadDefaultConceptMemory(fs::current_path());
    const auto matches = memory.findSimilar(cycle);
    enrichCycleWithMemory(cycle, matches);
    const ScenarioDescriptor scenario = {
        cycle.scenarioId,
        cycle.stabilized.title,
        "Documento ingerido de " + inputPath.string()
    };
    const ConstructionReport report = buildReportFromCycle(cycle, matches);

    ensureDirectory(outputDir);

    const fs::path cyclePath = outputDir / (cycle.scenarioId + std::string("-cycle.md"));
    const fs::path tracePath = outputDir / (cycle.scenarioId + std::string("-trace.md"));
    const fs::path reportPath = outputDir / (cycle.scenarioId + std::string("-report.md"));

    writeTextFile(cyclePath, renderCycleArtifact(cycle, scenario));
    writeTextFile(tracePath, renderTraceArtifact(cycle));
    writeTextFile(reportPath, renderConstructionReport(report));

    return {cyclePath, tracePath, reportPath};
}

} // namespace cce
