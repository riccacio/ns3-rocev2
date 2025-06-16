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
- `Ninja` (consigliato per CLion)
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

> Se usi CLion: imposta `ns-3-dev` come progetto esterno e configura il generatore `Ninja`.

---

## Esecuzione

```bash
./cmake-build-debug/rocev2-skeleton
```

L'output viene scritto nel file:

```bash
rocev2.log
```

---

## Note

- **Il modulo non include NS-3**: NS-3 è esterno e va mantenuto separato.
- Testato su **macOS ARM64 (Apple Silicon)** e **Linux Ubuntu**.

---

## Licenza

Distribuito sotto licenza MIT:

```
MIT License

Copyright (c) 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions...
```
