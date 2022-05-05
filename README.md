DPS: A Framework for Deterministic Parallel SAT Solvers
====

DPS is a framework to make a deterministic parallel SAT solver easily. It originates from ManyGlucose that is a reproducible efficient Parallel SAT Solver. ManyGlucose got the 3rd place in parallel track of SAT 2020 competition even with deterministic behavior.

## How to build?
- Release version
    ```bash
    $ ./build r
    $ ./Release/DPS-X.Y.Z SAT-instance.cnf
    ```
- Debug version
    ```bash
    $ ./build d
    $ ./Debug/DPS-X.Y.Z SAT-instance.cnf
    ```


