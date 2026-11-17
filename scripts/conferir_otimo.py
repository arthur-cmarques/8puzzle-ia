# Confere, com uma BFS independente (com conjunto de visitados), se os caminhos de results/bench*.csv são ótimos.
# Uso: python scripts/conferir_otimo.py results/bench.csv
import csv, sys
from collections import deque
alvo = "123456780"
def bfs(ini):
    vis = {ini: 0}; q = deque([ini])
    while q:
        s = q.popleft()
        if s == alvo: return vis[s], len(vis)
        z = s.index("0"); r, c = divmod(z, 3)
        for dr, dc in ((-1,0),(1,0),(0,-1),(0,1)):
            nr, nc = r+dr, c+dc
            if 0 <= nr < 3 and 0 <= nc < 3:
                l = list(s); n = nr*3+nc
                l[z], l[n] = l[n], l[z]; t = "".join(l)
                if t not in vis: vis[t] = vis[s]+1; q.append(t)
rows = list(csv.DictReader(open(sys.argv[1])))
ruim = 0
for r in rows:
    d, _ = bfs(r["estado_inicial"])
    for nome in ("bfs", "ids"):
        if r[nome+"_resolveu"] == "1" and int(r[nome+"_caminho"]) != d:
            ruim += 1; print("DIVERGE", r["puzzle"], nome, r[nome+"_caminho"], "ótimo", d)
    if r["bfs_resolveu"] == "0": print("não resolvido pela BFS: puzzle", r["puzzle"], "distância ótima real =", d)
print("conferidos:", len(rows), "divergências:", ruim, "| profundidades ótimas:", sorted(bfs(r["estado_inicial"])[0] for r in rows))
