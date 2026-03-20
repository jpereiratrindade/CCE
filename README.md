# CCE

Conceptual Construction Engine.

O `CCE` e um motor de construcao conceitual reflexiva. Ele transforma intuicoes iniciais em formulacoes conceituais mais robustas por meio de um percurso obrigatorio de tensao, restricao, expansao reflexiva, reconfiguracao e estabilizacao, sempre com `trace` explicito da construcao.

Nesta fase, o `CCE` e um projeto separado do `CRE`, embora tenha sido iniciado a partir da base de engenharia do `LabGestao`.

## O Que Ja Funciona
- cenarios conceituais embutidos
- ingestao de documentos Markdown
- analise semantica heuristica
- memoria conceitual local com persistencia
- geracao de artefatos `cycle`, `trace` e `report`
- testes automatizados e build CMake

## Nucleo do Metodo
- `Intuition Seed`: entrada inicial da intuicao
- `Tension Engine`: quebra generalizacao, teleologia e instrumentalismo
- `Constraint Layer`: impede teleologia implicita, causalidade linear simplista e normatividade oculta
- `Reflexive Expansion`: explicita pressupostos, implicacoes e limites
- `Reconfiguration Module`: reorganiza foco, termos e estrutura logica
- `Stabilization Layer`: produz uma formulacao final defensavel
- `Trace Layer`: registra a trajetoria inteira de construcao

Fluxo canonico:
- `Intuition -> Tension -> Constraint -> Reflexive Expansion -> Reconfiguration -> Stabilization`
- `Trace` acompanha todas as etapas

Invariantes iniciais:
- todo conceito deve passar por tensao explicita
- nenhuma saida final pode manter teleologia implicita
- nenhuma saida final pode manter causalidade linear simplista
- toda saida precisa registrar trace
- reflexividade e obrigatoria

## Inicio Rapido
```bash
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Demonstracao basica:

```bash
./build/cce
./build/cce demo --scenario socioecological-reflexivity
```

Ingestao de documento:

```bash
./build/cce ingest --input examples/resilience_ontological_test_doc.md
./build/cce ingest-record --input examples/resilience_ontological_test_doc.md --output ./output
```

Memoria conceitual:

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

## Cenarios Embutidos
- `knowledge-reality`
- `education-formation`
- `socioecological-reflexivity`

Caso canonico atual:
- `socioecological-reflexivity` materializa o argumento sobre conhecimento instrumentalizado, Humanidades, reflexividade critica, reconfiguracao das condicoes de inteligibilidade e ampliacao do campo do possivel

## Artefatos
Os artefatos gerados registram:
- ciclo conceitual consolidado
- trace da construcao
- relatorio final de estabilizacao

A memoria conceitual local fica em:
- `.cce/concept_memory.tsv`

## Estrutura
- `src/`: implementacao
- `include/`: headers publicos
- `tests/`: testes automatizados
- `examples/`: documentos de teste
- `adr/`: decisoes arquiteturais
- `ddd/`: modelagem de dominio
- `policies/`: regras operacionais
- `mcp/`: contratos executaveis para ferramentas e agentes
- `prompts/`: contexto estruturado
- `scripts/`: automacoes locais
- `.github/workflows/`: CI

## Separacao em Relacao ao CRE
Nesta fase:
- o `CCE` nao depende de codigo do `CRE`
- o `CCE` nao participa do build do `CRE`
- o `CCE` opera como projeto epistemico separado

Acoplamento futuro possivel:
- `CCE` produz enquadramentos conceituais e formulacoes estabilizadas
- `CRE` ou outro sistema pode testar implicacoes desses enquadramentos em ciclos experimentais
