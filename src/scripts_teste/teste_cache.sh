#!/bin/bash

SERVER_BIN=bin/dserver
CLIENT_BIN=bin/dclient
DOC_DIR=docs
CACHE_SIZES=(1 10 100 1000)
KEYS=("doc150" "doc120" "doc200" "doc300")
RESULTS="resultados_teste/resultados_cache.txt"

mkdir -p resultados_teste
mkdir -p pipes

echo "Teste Comparativo de Cache (sem e com cache)" > "$RESULTS"

for SIZE in "${CACHE_SIZES[@]}"; do
    echo "Testando com cache_size = $SIZE..."
    echo -e "\n--- Cache Size = $SIZE ---" >> "$RESULTS"

    $SERVER_BIN "$DOC_DIR" "$SIZE" &
    SERVER_PID=$!
    sleep 1

    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "Servidor terminou inesperadamente (cache_size = $SIZE)" >> "$RESULTS"
        continue
    fi

    for KEY in "${KEYS[@]}"; do
        # Primeira chamada: sem cache (cold)
        START=$(date +%s%N)
        $CLIENT_BIN -c "$KEY" > /dev/null
        END=$(date +%s%N)
        COLD=$(( (END - START)/1000000 ))

        # Segunda chamada: com cache (hot)
        START=$(date +%s%N)
        $CLIENT_BIN -c "$KEY" > /dev/null
        END=$(date +%s%N)
        HOT=$(( (END - START)/1000000 ))

        echo "Consulta $KEY: sem cache ${COLD} ms, com cache ${HOT} ms" >> "$RESULTS"
    done

    kill "$SERVER_PID"
    sleep 1
done

echo -e "\nResultados guardados em $RESULTS"
