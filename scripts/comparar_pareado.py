# Compara BFS e IDS apenas nos puzzles que os dois resolveram. Uso: python scripts/comparar_pareado.py results/bench.csv
import csv, sys, statistics as st
for arq in sys.argv[1:]:
    rows = [r for r in csv.DictReader(open(arq)) if r["bfs_resolveu"] == "1" and r["ids_resolveu"] == "1"]
    n = len(rows)
    be = [int(r["bfs_estados"]) for r in rows]; ie = [int(r["ids_estados_total"]) for r in rows]
    bt = [float(r["bfs_ms"]) for r in rows]; it = [float(r["ids_ms"]) for r in rows]
    bn = [int(r["bfs_nos_pico"]) for r in rows]; inn = [int(r["ids_nos_pico"]) for r in rows]
    raz = [i/b for i, b in zip(ie, be)]
    print(arq, "| puzzles resolvidos pelos dois:", n)
    print("  estados  BFS: média %.0f mediana %.0f máx %d | IDS: média %.0f mediana %.0f máx %d" % (st.mean(be), st.median(be), max(be), st.mean(ie), st.median(ie), max(ie)))
    print("  tempo ms BFS: média %.2f mediana %.3f máx %.1f | IDS: média %.2f mediana %.3f máx %.1f" % (st.mean(bt), st.median(bt), max(bt), st.mean(it), st.median(it), max(it)))
    print("  nós pico BFS máx %d (%.1f MiB) | IDS máx %d (%.1f MiB)" % (max(bn), max(bn)*64/2**20, max(inn), max(inn)*64/2**20))
    print("  razão estados IDS/BFS: mediana %.1f, mín %.1f, máx %.1f" % (st.median(raz), min(raz), max(raz)))
    print("  caminho médio (igual nos dois): %.2f" % st.mean(int(r["bfs_caminho"]) for r in rows))
