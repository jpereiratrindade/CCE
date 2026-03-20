# CCE

Conceptual Construction Engine.

O `CCE` e um projeto separado do `CRE`, criado sobre a base de engenharia do `LabGestao` e agora orientado ao metodo de construcao conceitual.

Seu objetivo nao e apenas organizar ideias, mas transformar intuicoes iniciais em formulacoes conceituais robustas por meio de um fluxo obrigatorio de tensao, restricao, expansao reflexiva, reconfiguracao e estabilizacao com trace explicito.

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

## Bootstrap
- Configurar build:
  - `cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- Compilar:
  - `cmake --build build`
- Testar:
  - `ctest --test-dir build --output-on-failure`
- Quality local:
  - `./scripts/run_quality.sh`
- Validar governança:
  - `python3 scripts/validate_tool_contracts.py`
  - `python3 scripts/validate_governance_repo.py`
- Instalar hooks:
  - `./scripts/dev/install-git-hooks.sh`

## CLI Inicial
- Rodar a demonstracao textual:
  - `./build/cce`
  - `./build/cce demo`
  - `./build/cce demo --scenario education-formation`
  - `./build/cce demo --scenario socioecological-reflexivity`
- Listar cenarios embutidos:
  - `./build/cce list-scenarios`
- Registrar um ciclo conceitual em Markdown:
  - `./build/cce record-cycle`
  - `./build/cce record-cycle --output ./output`
  - `./build/cce record-cycle --scenario knowledge-reality`

Cenarios embutidos:
- `knowledge-reality`
- `education-formation`
- `socioecological-reflexivity`

Os artefatos gerados registram:
- ciclo conceitual consolidado
- trace da construcao
- relatorio final de estabilizacao

Caso canonico atual:
- `socioecological-reflexivity` materializa o argumento sobre conhecimento instrumentalizado, Humanidades, reflexividade critica, reconfiguracao das condicoes de inteligibilidade e ampliacao do campo do possivel

## Estrutura
- `src/`: implementação
- `include/`: headers públicos
- `tests/`: testes automatizados
- `adr/`: decisoes arquiteturais
- `ddd/`: modelagem de dominio
- `policies/`: regras operacionais
- `mcp/`: contratos executáveis para ferramentas/agentes
- `prompts/`: contexto estruturado
- `scripts/`: automações locais
- `.github/workflows/`: CI

## Separacao em Relacao ao CRE
Nesta fase:
- o `CCE` nao depende de codigo do `CRE`
- o `CCE` nao participa do build do `CRE`
- o `CCE` opera como projeto epistemico separado

Acoplamento futuro possivel:
- `CCE` produz enquadramentos conceituais e formulacoes estabilizadas
- `CRE` ou outro sistema pode testar implicacoes desses enquadramentos em ciclos experimentais
