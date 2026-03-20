# ADR-AGENT-004 - Context Management

## Status
Aceito

## Contexto
O agente não preserva intenção com confiabilidade sem contexto explícito.

## Decisão
Contexto deve ser explícito, versionado e reintroduzido por tarefa.

## Estrategia
- Uso de prompts estruturados
- Reinício controlado de contexto
- Modos operacionais explícitos: explore, safe-change, restricted
