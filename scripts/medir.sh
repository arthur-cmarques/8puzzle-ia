#!/bin/sh
# Compila e roda a medição (BFS x aprofundamento iterativo).
# Uso: scripts/medir.sh [quantidade=30] [semente_base=1] [max_nos=5000000] [sufixo]
# Saída: results/bench[_sufixo].csv e results/resumo[_sufixo].txt
set -e
cd "$(dirname "$0")/.."
QTD="${1:-30}"; SEMENTE="${2:-1}"; MAXNOS="${3:-5000000}"; SUF="${4:+_$4}"
mkdir -p build results
gcc -O2 -Wall -Wextra -o build/8puzzle_com_IA src/8puzzle_com_IA.c
./build/8puzzle_com_IA --bench "$QTD" "$SEMENTE" "$MAXNOS" > "results/bench$SUF.csv" 2> "results/resumo$SUF.txt"
{
  echo
  echo "Ambiente:"
  gcc --version | head -n 1
  uname -sm
  grep -m 1 'model name' /proc/cpuinfo 2>/dev/null || true
} >> "results/resumo$SUF.txt"
echo "Gerado: results/bench$SUF.csv e results/resumo$SUF.txt"
