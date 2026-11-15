# Emergency Relief Logistics Hub

A beginner-oriented C++17 console project for tracking emergency supplies and evaluating relief-vehicle routes after an earthquake. It demonstrates the CIC-211 assignment mechanisms in a small example that can be read from `main()` outward.

## Build and run

With g++ on Linux or macOS:

```sh
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o relief-logistics-hub
./relief-logistics-hub
```

With MinGW g++ on Windows:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o relief-logistics-hub.exe
.\relief-logistics-hub.exe
```

The program writes binary incident records to `incident_log.bin` in its current working directory. That generated log is ignored by Git.

## What to look at

- `ReliefOps::Inventory::ReliefCargo` owns a dynamic array and implements a deep copy, destructor, and stream operators.
- `DistributionDepot` demonstrates encapsulated inventory, reference access, and `+=` / `-=` restocking and dispatch.
- `ReliefVehicle` is an abstract base class. Ground, aerial, amphibious, and hybrid vehicles override route evaluation.
- `AmphibiousMedicalUnit` inherits through two virtual base paths, so it contains one shared `ReliefVehicle` base.
- `PriorityDispatchQueue<T>`, STL containers, `std::sort`, and a custom functor sort and queue urgent needs.
- `saveIncident()` writes a new binary log through a temporary file and replaces the old file after a successful write; `loadIncidents()` reads it back.

The example uses simple illustrative route multipliers and sample quantities. They are not operational disaster-response advice.

## Assignment mapping

The full unit-by-unit explanation, design notes, and run-through are in [`docs/TECHNICAL_REPORT.md`](docs/TECHNICAL_REPORT.md).

