#include <algorithm>
#include <cmath>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

namespace ReliefOps {
namespace Inventory {

class ReliefCargo {
    string name_;
    int* quantities_;
    size_t size_;
public:
    ReliefCargo(string name = "", const int* values = nullptr, size_t count = 0)
        : name_(move(name)), quantities_(count ? new int[count] : nullptr), size_(count) {
        for (size_t i = 0; i < size_; ++i) quantities_[i] = values[i];
    }
    ReliefCargo(const ReliefCargo& other)
        : ReliefCargo(other.name_, other.quantities_, other.size_) {}
    ReliefCargo& operator=(ReliefCargo other) { swap(other); return *this; }
    ~ReliefCargo() { delete[] quantities_; }
    void swap(ReliefCargo& other) noexcept {
        swap(name_, other.name_); swap(quantities_, other.quantities_); swap(size_, other.size_);
    }
    const string& name() const { return name_; }
    size_t size() const { return size_; }
    int quantity(size_t i) const { return i < size_ ? quantities_[i] : 0; }
    bool operator==(const ReliefCargo& rhs) const { return name_ == rhs.name_; }
    friend ostream& operator<<(ostream& out, const ReliefCargo& c) {
        out << c.name_ << ' ' << c.size_;
        for (size_t i = 0; i < c.size_; ++i) out << ' ' << c.quantities_[i];
        return out;
    }
    friend istream& operator>>(istream& in, ReliefCargo& c) {
        string name; size_t n;
        if (in >> name >> n) {
            int* values = n ? new int[n] : nullptr;
            for (size_t i = 0; i < n; ++i) in >> values[i];
            if (in) { ReliefCargo temp(name, values, n); c = temp; }
            delete[] values;
        }
        return in;
    }
};

class DistributionDepot {
    unordered_map<string, int> stock_;
public:
    DistributionDepot& operator+=(const ReliefCargo& cargo) {
        for (size_t i = 0; i < cargo.size(); ++i) stock_[cargo.name()] += cargo.quantity(i);
        return *this;
    }
    DistributionDepot& operator-=(const ReliefCargo& cargo) {
        for (size_t i = 0; i < cargo.size(); ++i) {
            int& available = stock_[cargo.name()];
            if (available < cargo.quantity(i)) throw runtime_error("Insufficient depot stock");
            available -= cargo.quantity(i);
        }
        return *this;
    }
    int stock(const string& item) const {
        auto it = stock_.find(item); return it == stock_.end() ? 0 : it->second;
    }
    int& stockRef(const string& item) { return stock_[item]; }
};

} // namespace Inventory

namespace Logistics {

inline double distanceKm(double x1, double y1, double x2, double y2) {
    const double dx = x2 - x1, dy = y2 - y1;
    return sqrt(dx * dx + dy * dy); // simple teaching estimate
}

class IncidentCommander;
class FuelDepletionException : public exception {
public: const char* what() const noexcept override { return "Vehicle fuel is below dispatch minimum"; }
};
class DamagedCorridorException : public exception {
public: const char* what() const noexcept override { return "Route corridor is marked damaged"; }
};
class CargoOverweightException : public exception {
public: const char* what() const noexcept override { return "Cargo exceeds vehicle payload capacity"; }
};

class GPSBeacon {
    double x_, y_;
public:
    GPSBeacon(double x = 0, double y = 0) : x_(x), y_(y) {}
    void set(double x, double y) { x_ = x; y_ = y; }
    double distanceTo(double x, double y) const { return distanceKm(x_, y_, x, y); }
};
class EmergencyLifeSupportUnit {
    bool ready_;
public:
    explicit EmergencyLifeSupportUnit(bool ready = true) : ready_(ready) {}
    bool ready() const { return ready_; }
};

class ReliefVehicle {
    string id_;
    double capacity_, fuel_;
    static int operationalVehicles_;
    static double dispatchedTonnage_;
protected:
    GPSBeacon gps_;
    EmergencyLifeSupportUnit lifeSupport_;
public:
    ReliefVehicle(string id, double capacity, double fuel)
        : id_(move(id)), capacity_(capacity), fuel_(fuel) { ++operationalVehicles_; }
    virtual ~ReliefVehicle() { --operationalVehicles_; }
    ReliefVehicle(const ReliefVehicle&) = delete;
    ReliefVehicle& operator=(const ReliefVehicle&) = delete;
    const string& id() const { return id_; }
    double capacity() const { return capacity_; }
    static int operationalVehicles() { return operationalVehicles_; }
    static double dispatchedTonnage() { return dispatchedTonnage_; }
    virtual string type() const = 0;
    virtual double evaluateRoute(double distance) const = 0;
    double dispatch(double payload, double distance, bool corridorOpen) const {
        if (!corridorOpen) throw DamagedCorridorException();
        if (payload > capacity_) throw CargoOverweightException();
        if (fuel_ <= distance * 0.1) throw FuelDepletionException();
        dispatchedTonnage_ += payload / 1000.0;
        return evaluateRoute(distance);
    }
    friend class IncidentCommander;
};
using TransportVehicle = ReliefVehicle; // assignment term for the same abstract vehicle model
int ReliefVehicle::operationalVehicles_ = 0;
double ReliefVehicle::dispatchedTonnage_ = 0.0;

class GroundConvoy : virtual public ReliefVehicle {
public:
    GroundConvoy(string id, double capacity, double fuel) : ReliefVehicle(id, capacity, fuel) {}
    string type() const override { return "Ground convoy"; }
    double evaluateRoute(double km) const override { return km * 1.2; }
};
class AerialTransport : virtual public ReliefVehicle {
public:
    AerialTransport(string id, double capacity, double fuel) : ReliefVehicle(id, capacity, fuel) {}
    string type() const override { return "Aerial transport"; }
    double evaluateRoute(double km) const override { return km * 0.8; }
};
class AmphibiousCraft : virtual public ReliefVehicle {
public:
    AmphibiousCraft(string id, double capacity, double fuel) : ReliefVehicle(id, capacity, fuel) {}
    string type() const override { return "Amphibious craft"; }
    double evaluateRoute(double km) const override { return km * 1.5; }
};
class AmphibiousMedicalUnit : public GroundConvoy, public AmphibiousCraft {
public:
    AmphibiousMedicalUnit(string id, double capacity, double fuel)
        : ReliefVehicle(id, capacity, fuel), GroundConvoy(id, capacity, fuel), AmphibiousCraft(id, capacity, fuel) {}
    string type() const override { return "Amphibious medical unit"; }
    double evaluateRoute(double km) const override { return km * 1.1; }
};

class IncidentCommander {
public:
    void emergencyRefuel(ReliefVehicle& vehicle, double amount) const {
        vehicle.fuel_ += amount; // friend access for an authorized emergency override
    }
};

// The second overload represents priority-based triage allocation.
inline bool allocateSupply(Inventory::DistributionDepot& depot, const Inventory::ReliefCargo& cargo) {
    depot -= cargo; return true;
}
inline bool allocateSupply(Inventory::DistributionDepot& depot, const Inventory::ReliefCargo& cargo, int triagePriority) {
    if (triagePriority < 1) return false;
    depot -= cargo; return true;
}

template <typename T>
class PriorityDispatchQueue {
    priority_queue<T> queue_;
public:
    void push(const T& item) { queue_.push(item); }
    bool empty() const { return queue_.empty(); }
    T pop() { T item = queue_.top(); queue_.pop(); return item; }
};
struct UrgentNeed {
    string item; int priority;
    bool operator<(const UrgentNeed& rhs) const { return priority < rhs.priority; }
};
struct UrgencyFirst {
    bool operator()(const UrgentNeed& a, const UrgentNeed& b) const { return a.priority > b.priority; }
};
ostream& manifestTable(ostream& out) { return out << "ITEM                 QTY\n------------------------\n"; }

struct IncidentRecord { char vehicleId[32]; double distanceKm; double travelHours; };
bool saveIncident(const IncidentRecord& record, const string& path) {
    const string tempPath = path + ".tmp";
    fstream out(tempPath, ios::binary | ios::out | ios::trunc);
    ifstream old(path, ios::binary);
    if (old) {
        old.seekg(0, ios::end); const auto end = old.tellg(); old.seekg(0, ios::beg);
        vector<char> previous(static_cast<size_t>(end));
        if (!previous.empty()) old.read(previous.data(), static_cast<streamsize>(previous.size()));
        out.seekp(0, ios::beg);
        if (!previous.empty()) out.write(previous.data(), static_cast<streamsize>(previous.size()));
    }
    out.seekp(0, ios::end);
    out.write(reinterpret_cast<const char*>(&record), sizeof(record)); out.flush();
    if (!out) { out.close(); remove(tempPath.c_str()); return false; }
    out.close(); old.close();
#ifdef _WIN32
    if (!MoveFileExA(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(tempPath.c_str()); return false;
    }
#else
    return rename(tempPath.c_str(), path.c_str()) == 0;
#endif
    return true;
}
vector<IncidentRecord> loadIncidents(const string& path) {
    ifstream in(path, ios::binary);
    vector<IncidentRecord> records;
    IncidentRecord record{};
    while (in.read(reinterpret_cast<char*>(&record), sizeof(record))) records.push_back(record);
    return records;
}

} // namespace Logistics
} // namespace ReliefOps

int main() {
    using namespace ReliefOps;
    Inventory::DistributionDepot depot;
    const int initial[] = {20};
    Inventory::ReliefCargo water("Water kits", initial, 1);
    depot += water;
    int& waterStock = depot.stockRef("Water kits"); // return-by-reference update
    waterStock += 2;
    Logistics::allocateSupply(depot, water, 10); // priority triage overload

    vector<Logistics::ReliefVehicle*> fleet;
    fleet.push_back(new Logistics::GroundConvoy("TRK-01", 1000, 80));
    fleet.push_back(new Logistics::AmphibiousMedicalUnit("HYB-01", 500, 45));

    Logistics::PriorityDispatchQueue<Logistics::UrgentNeed> needs;
    needs.push({"Trauma kits", 10}); needs.push({"Water kits", 6});
    vector<Logistics::UrgentNeed> sortedNeeds{{"Water kits", 6}, {"Trauma kits", 10}};
    sort(sortedNeeds.begin(), sortedNeeds.end(), Logistics::UrgencyFirst());
    cout << "Highest sorted need: " << sortedNeeds.front().item << '\n';
    cout << "Highest queued need: " << needs.pop().item << '\n';

    cout << Logistics::manifestTable << "Water kits          " << depot.stock("Water kits") << '\n';
    const Logistics::IncidentCommander commander;
    for (auto* vehicle : fleet) {
        try {
            double hours = vehicle->dispatch(100, 30, true);
            cout << vehicle->id() << " (" << vehicle->type() << ") route estimate: " << hours << " hours\n";
            Logistics::IncidentRecord record{};
            strncpy(record.vehicleId, vehicle->id().c_str(), sizeof(record.vehicleId) - 1);
            record.distanceKm = 30; record.travelHours = hours;
            Logistics::saveIncident(record, "incident_log.bin");
        } catch (const exception& e) {
            commander.emergencyRefuel(*vehicle, 20);
            cerr << e.what() << "\n";
        }
    }
    try {
        fleet[1]->dispatch(600, 30, true); // intentionally exceeds the hybrid's capacity
    } catch (const Logistics::CargoOverweightException& e) {
        cout << "Rejected dispatch: " << e.what() << '\n';
    }
    cout << "Total dispatched: " << Logistics::ReliefVehicle::dispatchedTonnage() << " tonnes\n";
    cout << "Incidents saved: " << Logistics::loadIncidents("incident_log.bin").size() << '\n';
    for (auto* vehicle : fleet) delete vehicle; // invokes the virtual base destructor
    return 0;
}

