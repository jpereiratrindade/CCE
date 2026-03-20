# CCE

Conceptual Construction Engine.

O `CCE` é um motor de construção conceitual reflexiva. Ele transforma intuições iniciais em formulações conceituais mais robustas por meio de um percurso obrigatório de tensão, restrição, expansão reflexiva, reconfiguração e estabilização, sempre com `trace` explícito da construção.

## O Que Ja Funciona
- cenários conceituais embutidos
- ingestão de documentos Markdown
- análise semântica heurística
- memória conceitual local com persistência
- geração de artefatos `cycle`, `trace` e `report`
- testes automatizados e build CMake

## Núcleo do Método
- `Intuition Seed`: entrada inicial da intuição
- `Tension Engine`: quebra generalização, teleologia e instrumentalismo
- `Constraint Layer`: impede teleologia implícita, causalidade linear simplista e normatividade oculta
- `Reflexive Expansion`: explicita pressupostos, implicações e limites
- `Reconfiguration Module`: reorganiza foco, termos e estrutura lógica
- `Stabilization Layer`: produz uma formulação final defensável
- `Trace Layer`: registra a trajetória inteira de construção

Fluxo canônico:
- `Intuition -> Tension -> Constraint -> Reflexive Expansion -> Reconfiguration -> Stabilization`
- `Trace` acompanha todas as etapas

Invariantes iniciais:
- todo conceito deve passar por tensão explícita
- nenhuma saída final pode manter teleologia implícita
- nenhuma saída final pode manter causalidade linear simplista
- toda saída precisa registrar trace
- reflexividade é obrigatória

## Início Rápido
```bash
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Demonstração básica:

```bash
./build/cce
./build/cce demo --scenario socioecological-reflexivity
```

Ingestão de documento:

```bash
./build/cce ingest --input examples/resilience_ontological_test_doc.md
./build/cce ingest-record --input examples/resilience_ontological_test_doc.md --output ./output
```

Memória conceitual:

```bash
./build/cce memory-inspect
./build/cce memory-query --scenario resilience-ontological-test-doc
```

## CLI
- `./build/cce`
- `./build/cce demo --scenario <id>`
- `./build/cce list-scenarios`
- `./build/cce record-cycle --scenario <id> --output <dir>`
- `./build/cce ingest --input <arquivo.md>`
- `./build/cce ingest-record --input <arquivo.md> --output <dir>`
- `./build/cce memory-inspect`
- `./build/cce memory-query --scenario <id> --limit <n>`
- `./build/cce memory-query --input <arquivo.md> --limit <n>`

## Cenários Embutidos
- `knowledge-reality`
- `education-formation`
- `socioecological-reflexivity`

Caso canônico atual:
- `socioecological-reflexivity` materializa o argumento sobre conhecimento instrumentalizado, Humanidades, reflexividade crítica, reconfiguração das condições de inteligibilidade e ampliação do campo do possível

## Artefatos
Os artefatos gerados registram:
- ciclo conceitual consolidado
- trace da construção
- relatório final de estabilização

A memória conceitual local fica em:
- `.cce/concept_memory.tsv`

## Estrutura
- `src/`: implementação
- `include/`: headers públicos
- `tests/`: testes automatizados
- `examples/`: documentos de teste
- `adr/`: decisões arquiteturais
- `ddd/`: modelagem de dominio
- `policies/`: regras operacionais
- `mcp/`: contratos executáveis para ferramentas e agentes
- `prompts/`: contexto estruturado
- `scripts/`: automações locais
- `.github/workflows/`: CI
