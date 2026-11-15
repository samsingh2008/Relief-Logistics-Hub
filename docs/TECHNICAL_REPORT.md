# Technical Report — Emergency Relief Logistics Hub

**Course:** Object-Oriented Programming Using C++ (CIC-211)  
**Student:** Samriddhi Singh  
**Group:** C21  
**Project option:** Option 2 — Post-Earthquake Multi-Agency Emergency Relief Logistics Hub  
**Language standard:** C++17

## 1. Project overview

This console program models a small relief depot and a fleet of emergency vehicles. The depot stores sample water-kit stock. Ground and amphibious medical vehicles evaluate a route, check payload, fuel, and corridor status, and write successful trips to a binary incident log. The example keeps its data and route rules intentionally small so the class relationships and C++ mechanisms are straightforward to review.

## 2. Requirements and design

| Assignment unit | Mechanism | Where it appears |
|---|---|---|
| I | Namespaces | `ReliefOps::Inventory` and `ReliefOps::Logistics` separate stock from transport code. |
| I | Inline calculations and function overloading | `distanceKm()` is inline; the two `allocateSupply()` overloads represent ordinary and triage allocation. |
| I | References | `DistributionDepot::operator+=` / `operator-=` take cargo by reference; `stockRef()` returns a stock count by reference. |
| I | Friend access | `IncidentCommander` is a friend of `ReliefVehicle` and can apply an emergency refuel without a public fuel setter. |
| II | Encapsulation and const correctness | Fields are private; read-only accessors are `const`; inputs that are not modified are const references. |
| II | Dynamic memory and deep copying | `ReliefCargo` allocates its quantity array, copies it, assigns safely, and deletes it in its destructor. |
| II | Static analytics | `ReliefVehicle` tracks active instances and dispatched payload tonnage across the fleet. |
| II | Operator overloading | `ReliefCargo` defines `==`, `<<`, and `>>`; `DistributionDepot` defines `+=` and `-=`. |
| III | Abstract base and runtime polymorphism | `ReliefVehicle` has pure virtual route/type functions; a `vector<ReliefVehicle*>` invokes each override. Its virtual destructor supports safe base-pointer deletion. |
| III | Virtual inheritance | Ground, aerial, and amphibious branches virtually inherit `ReliefVehicle`; the amphibious medical hybrid combines ground and amphibious branches. |
| III | Composition | Each vehicle object contains a `GPSBeacon` and an `EmergencyLifeSupportUnit` through its base vehicle state. |
| IV | Exceptions | Fuel, cargo, and corridor exceptions derive from `std::exception`; dispatch reports them through `what()`. |
| IV | Binary persistence | Fixed-size `IncidentRecord` values are appended to a binary file via a temporary file; the old log is copied with `seekg()` and the new record is appended with `seekp()`. Failed writes discard the temporary file. |
| IV | STL, templates, algorithm, functor, manipulator | The project uses `unordered_map`, `vector`, `priority_queue`, `PriorityDispatchQueue<T>`, `sort`, `UrgencyFirst`, and `manifestTable`. |

## 3. Program flow

1. `main()` creates a depot and adds sample water-kit stock.
2. It constructs a ground convoy and an amphibious medical hybrid, then stores their base-class addresses in a vector.
3. It creates urgent supply needs and sorts them by priority with `UrgencyFirst`.
4. Each vehicle evaluates a sample route through virtual dispatch. Dispatch checks route safety, payload, and fuel and updates the shared dispatched-tonnage statistic.
5. A successful trip is saved in `incident_log.bin`; afterward the program reads the records back and reports the count.
6. If dispatch validation throws, the catch block prints the custom diagnostic and the commander applies a sample emergency refuel.

## 4. Build and execution

Compile with a C++17 compiler:

```text
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o relief-logistics-hub
```

Run `relief-logistics-hub` (or `relief-logistics-hub.exe` on Windows). The program creates `incident_log.bin` in its current working directory. The illustrative route coefficients and test data are classroom examples; they are not real routing or emergency-response calculations.

## 5. Learning notes and limitations

A good review order is `main()`, `ReliefCargo`, `DistributionDepot`, the vehicle hierarchy, and finally persistence and the template queue. The source uses `using namespace std;` to match the provided beginner-style sample and keep standard-library names short. The heap-allocated cargo array is small to make copy construction and destruction visible; production code would normally use `std::vector<int>`. The sample does not provide a full dispatch user interface, real maps, fuel models, or agency authentication. A full deployment would require domain validation, stronger persistence and recovery tests, and safety review.

## 6. Verification status

The source was updated to use `using namespace std;` after the previous successful GitHub Actions run. The updated files are being uploaded now; this section will be replaced with the result of the new C++17 build and run.

