# ns3-rocev2

Simulatore custom per il protocollo **RoCEv2 (RDMA over Converged Ethernet v2)**, basato su [ns-3](https://www.nsnam.org/).  
Il modulo consente la simulazione di flussi RDMA su percorsi multipli con supporto per misorder e soluzioni alternative ai moduli hardware già esistenti.

---

## Struttura

```
ns3-rocev2/
├── model/                
├── examples/             
├── CMakeLists.txt        
└── README.md
```

---

## Requisiti

- `ns-3-dev` (installato localmente fuori da questa repo)
- `CMake ≥ 3.10`
- `Ninja`
- Compiler C++20 (Clang/LLVM, GCC ≥ 10)

---

## Setup

1. Clona `ns-3-dev` accanto al modulo:
   ```bash
   git clone https://gitlab.com/nsnam/ns-3-dev.git
   ```

2. Costruisci NS-3 con supporto per librerie condivise:
   ```bash
   cd ns-3-dev
   ./ns3 configure --enable-examples
   ./ns3 build
   ```

---

## Build del modulo

```bash
cd ns3-rocev2
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug
```
---

## Esecuzione

```bash
./cmake-build-debug/rocev2-skeleton
```

L'output viene scritto nel terminale

---

## Note

- **Il modulo non include NS-3**: NS-3 è esterno e va mantenuto separato.
- Testato su **macOS ARM64 (Apple Silicon)**.

---
