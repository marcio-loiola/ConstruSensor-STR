import json
from math import gcd

# Declaração de variáveis globais
global n
global hp
global tarefas

def ler_dados():
    global n
    global tarefas

    # Solicita ao usuário o número de tarefas
    n = int(input("\n\n\t\tDigite o número de tarefas:"))

    # Inicializa o dicionário 'tarefas'
    tarefas = {}

    # Armazena os dados em um dicionário
    for i in range(n):
        tarefas["TAREFA_%d" % i] = {"início": [], "fim": []}

    tarefas["TAREFA_IDLE"] = {"início": [], "fim": []}

    # Solicita ao usuário os períodos e WCETs das tarefas
    for i in range(n):
        tarefas[i] = {}
        print("\n\n\n Digite o período da tarefa T", i+1, ":")
        p = input()
        tarefas[i]["Período"] = int(p)
        print("Digite o WCET da tarefa T", i, ":")
        w = input()
        tarefas[i]["WCET"] = int(w)

    # Escreve o dicionário em um arquivo JSON
    with open('tarefas.json', 'w') as outfile:
        json.dump(tarefas, outfile, indent=4)

def hiperperíodo():
    global n
    global tarefas

    temp = []

    # Calcula o hiperperíodo
    for i in range(n):
        temp.append(tarefas[i]["Período"])
    HP = temp[0]
    for i in temp[1:]:
        HP = HP * i // gcd(HP, i)
    print("\n Hiperperíodo:", HP)
    return HP

def escalonabilidade():
    global n
    global tarefas

    T = []
    C = []
    U = []

    # Calcula o fator de utilização
    for i in range(n):
        T.append(int(tarefas[i]["Período"]))
        C.append(int(tarefas[i]["WCET"]))
        u = int(C[i]) / int(T[i])
        U.append(u)

    fator_utilizacao = sum(U)

    # Verifica a condição de escalonabilidade
    if fator_utilizacao <= 1:
        print("\n\tFator de utilização:", fator_utilizacao, "tarefas subutilizadas")

        sched_util = n * (2 ** (1 / n) - 1)
        print("Checando condição:", sched_util)

        count = 0
        T.sort()
        for i in range(len(T)):
            if T[i] % T[0] == 0:
                count = count + 1

        # Verifica a condição de escalonabilidade pelo protocolo RM
        if fator_utilizacao <= sched_util or count == len(T):
            print("\n\tTarefas são escalonáveis pelo protocolo RM!")
            return True
        else:
            print("\n\tTarefas não são escalonáveis pelo protocolo RM!")
            return False
    print("\n\tTarefas sobrecarregadas!")
    print("\n\tFator de utilização > 1")
    return False

def estimatePriority(RealTime_task):
    global hp
    global tarefas

    tempPeriod = hp
    P = -1  # Retorna -1 para tarefas ociosas
    for i in RealTime_task.keys():
        if (RealTime_task[i]["WCET"] != 0):
            if (tempPeriod > RealTime_task[i]["Período"] or tempPeriod > tarefas[i]["Período"]):
                tempPeriod = tarefas[i]["Período"]  # Checa a prioridade de cada tarefa com base no período
                P = i
    return P

def main():
    global hp
    ler_dados()
    hp = hiperperíodo()
    escalonabilidade()
    estimatePriority()

if __name__ == "__main__":
    main()
