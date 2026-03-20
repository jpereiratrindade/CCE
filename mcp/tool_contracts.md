# Tool Contracts

Define contratos executáveis de interação entre agente e ferramentas.

## Campos obrigatórios
- contract_version
- tool_name
- tool
- description
- allowed_modes
- risk_level
- policy_refs
- human_approval
- preconditions
- required_evidence
- forbidden_actions
- input_schema
- output_schema

## Regras operacionais
- Ferramentas sem contrato válido não podem ser usadas em modo executável
- Ferramentas de risco high ou critical exigem aprovação humana
- Toda ferramenta deve apontar para políticas que justificam seu uso
- Toda ferramenta deve declarar evidências mínimas de auditoria

## Modo operacional
- explore: leitura, análise e planejamento
- safe-change: alteração controlada com validação
- restricted: alteração apenas sob políticas mais rígidas e aprovação explícita
