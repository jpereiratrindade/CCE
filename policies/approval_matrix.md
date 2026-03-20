# Approval Matrix

## Regras por tipo de mudanca
- docs: aprovação humana simples
- config: aprovação humana simples com diff revisado
- código fora de contexto crítico: aprovação humana simples e testes
- código em contexto crítico: aprovação humana obrigatória com política required_if_core_module
- ação destrutiva: aprovação humana obrigatória com política required_if_destructive

## Evidencias minimas
- task_id
- owner
- tool_name
- approval_policy
- tests_executed
- final_status
