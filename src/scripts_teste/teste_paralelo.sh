#!/bin/bash

SERVER_BIN=bin/dserver
CLIENT_BIN=bin/dclient
DOC_DIR=docs
RESULTS="resultados_teste/resultados_paralelo.txt"

MAX_PROCS=(1 2 4 8)
KEYWORDS=("adventure" "science" "history" "magic")

mkdir -p resultados_teste
mkdir -p pipes

echo "Teste de Paralelismo (sem cache)" > "$RESULTS"

for N in "${MAX_PROCS[@]}"; do
    echo -e "\n--- Testando com $N processos ---" >> "$RESULTS"
    echo "Executando testes com $N processos..."

    for WORD in "${KEYWORDS[@]}"; do
        echo "Palavra-chave: $WORD" >> "$RESULTS"

        $SERVER_BIN "$DOC_DIR" 1 &  
        SERVER_PID=$!
        sleep 1

        START=$(date +%s%N)
        $CLIENT_BIN -s "$WORD" "$N" > /dev/null
        END=$(date +%s%N)

        ELAPSED=$(( (END - START)/1000000 ))
        echo "$N processos, '$WORD': ${ELAPSED} ms" >> "$RESULTS"

        kill "$SERVER_PID"
        wait "$SERVER_PID" 2>/dev/null
        sleep 1
    done
done

echo -e "\nResultados guardados em $RESULTS"
