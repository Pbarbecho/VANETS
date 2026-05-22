// beacon.cc – Neighbour Node Table (NNT) implementation for Veins VANET routing
// Modified: Pablo Barbecho – Universidad de Cuenca

#include "veins/modules/application/traci/beacon.h"

#include <iomanip>
#include <string>

using namespace std;
using namespace veins;

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

BeaconList::BeaconList()
    : head(nullptr), curr(nullptr), temp(nullptr)
{
}

BeaconList::~BeaconList()
{
    // Walk the list and free every node to avoid memory leaks.
    curr = head;
    while (curr != nullptr) {
        beaconPtr toDelete = curr;
        curr = curr->next;
        delete toDelete;
    }
}

// ---------------------------------------------------------------------------
// AddBeacon – append a new entry at the tail of the NNT
// ---------------------------------------------------------------------------

void BeaconList::AddBeacon(std::string b_typeNode, int b_idMsg, int b_idVehicle,
                           double b_time, Coord b_SenderCoord,
                           double b_dsr, double b_dsd, double b_drd,
                           double b_Nv, double b_wdist, double b_abe,
                           LAddress::L2Type b_NH_Address,
                           double b_NH_GS, double b_NH_Dst_to_Dest)
{
    beaconPtr n  = new beacon;
    n->next      = nullptr;
    n->typeNode  = b_typeNode;
    n->idMsg     = b_idMsg;
    n->idVehicle = b_idVehicle;
    n->time      = b_time;
    n->SenderCoord   = b_SenderCoord;
    n->dsr       = b_dsr;
    n->dsd       = b_dsd;
    n->drd       = b_drd;
    n->Nv        = b_Nv;
    n->wdist     = b_wdist;
    n->abe       = b_abe;
    n->NH_Address     = b_NH_Address;
    n->NH_GS          = b_NH_GS;
    n->NH_Dst_to_Dest = b_NH_Dst_to_Dest;

    if (head != nullptr) {
        // Traverse to the last node and link the new one.
        curr = head;
        while (curr->next != nullptr) { curr = curr->next; }
        curr->next = n;
    }
    else {
        head = n;
    }
}

// ---------------------------------------------------------------------------
// UpdateBeacon – overwrite all fields of the entry identified by b_idVehicle
// ---------------------------------------------------------------------------

void BeaconList::UpdateBeacon(std::string b_typeNode, int b_idMsg, int b_idVehicle,
                              double b_time, Coord b_SenderCoord,
                              double b_dsr, double b_dsd, double b_drd,
                              double b_Nv, double b_wdist, double b_abe,
                              LAddress::L2Type b_NH_Address,
                              double b_NH_GS, double b_NH_Dst_to_Dest)
{
    // Traverse without allocating – we are updating an existing node.
    beaconPtr n = head;
    while (n != nullptr) {
        if (n->idVehicle == b_idVehicle) {
            n->typeNode       = b_typeNode;
            n->idMsg          = b_idMsg;
            n->time           = b_time;
            n->SenderCoord    = b_SenderCoord;
            n->dsr            = b_dsr;
            n->dsd            = b_dsd;
            n->drd            = b_drd;
            n->Nv             = b_Nv;
            n->wdist          = b_wdist;
            n->abe            = b_abe;
            n->NH_Address     = b_NH_Address;
            n->NH_GS          = b_NH_GS;
            n->NH_Dst_to_Dest = b_NH_Dst_to_Dest;
            return;
        }
        n = n->next;
    }
    // If idVehicle was not found the call is a no-op (caller should use
    // SearchBeaconNodeID() before deciding between Add vs. Update).
}

// ---------------------------------------------------------------------------
// DeleteBeacon – remove the entry with the given vehicle ID
// ---------------------------------------------------------------------------

void BeaconList::DeleteBeacon(int delb_idVehicle)
{
    if (head == nullptr) return;

    // Special case: the matching entry is the head.
    if (head->idVehicle == delb_idVehicle) {
        beaconPtr toDelete = head;
        head = head->next;
        delete toDelete;
        return;
    }

    // General case: find the predecessor of the entry to delete.
    temp = head;
    curr = head->next;
    while (curr != nullptr && curr->idVehicle != delb_idVehicle) {
        temp = curr;
        curr = curr->next;
    }

    if (curr != nullptr) {
        temp->next = curr->next;
        delete curr;
    }
    // If curr == nullptr the ID was not found – no-op.
}

// ---------------------------------------------------------------------------
// PurgeBeacons – remove all entries older than b_ttl seconds
// ---------------------------------------------------------------------------

void BeaconList::PurgeBeacons(double b_ttl)
{
    double timeExpired = simTime().dbl() - b_ttl;

    // Remove expired entries from the head of the list.
    while (head != nullptr && head->time < timeExpired) {
        beaconPtr toDelete = head;
        head = head->next;
        delete toDelete;   // use delete (not free) for C++ objects
    }

    // Remove expired entries from the remainder of the list.
    curr = head;
    while (curr != nullptr && curr->next != nullptr) {
        if (curr->next->time < timeExpired) {
            beaconPtr toDelete = curr->next;
            curr->next = toDelete->next;
            delete toDelete;
        }
        else {
            curr = curr->next;
        }
    }
}

// ---------------------------------------------------------------------------
// SortBeacons – bubble-sort in descending order of composite score (wdist+Nv)
// ---------------------------------------------------------------------------

void BeaconList::SortBeacons()
{
    bool changeMade;
    do {
        changeMade = false;
        curr = head;
        while (curr != nullptr && curr->next != nullptr) {
            temp = curr;
            curr = curr->next;

            // Composite score: equal weight for weighted distance and density.
            double GS_temp = (temp->wdist + temp->Nv) * 0.5;
            double GS_curr = (curr->wdist + curr->Nv) * 0.5;

            if (GS_curr > GS_temp) {
                // Swap all payload fields (not the next pointers).
                swap(temp->typeNode,       curr->typeNode);
                swap(temp->idMsg,          curr->idMsg);
                swap(temp->idVehicle,      curr->idVehicle);
                swap(temp->time,           curr->time);
                swap(temp->SenderCoord,    curr->SenderCoord);
                swap(temp->dsr,            curr->dsr);
                swap(temp->dsd,            curr->dsd);
                swap(temp->drd,            curr->drd);
                swap(temp->Nv,             curr->Nv);
                swap(temp->wdist,          curr->wdist);
                swap(temp->abe,            curr->abe);
                swap(temp->NH_Address,     curr->NH_Address);
                swap(temp->NH_GS,          curr->NH_GS);
                swap(temp->NH_Dst_to_Dest, curr->NH_Dst_to_Dest);
                changeMade = true;
            }
        }
    } while (changeMade);
}

// ---------------------------------------------------------------------------
// PrintBeacons – dump the NNT to the OMNeT++ EV log
// ---------------------------------------------------------------------------

void BeaconList::PrintBeacons(int CurrNodeAddress, string IN_OUT)
{
    curr = head;
    EV << string(160, '-') << endl;
    EV << setw(70) << IN_OUT << "  -->  Vehicle Neighbour Table  CurrNodeAddress=" << CurrNodeAddress << endl;
    EV << string(160, '-') << endl;
    EV << "NodeType"
       << setw(15) << "TreeMsg_ID"
       << setw(15) << "MAC_Address"
       << setw(15) << "NH_Address"
       << setw(13) << "NH_GS"
       << setw(15) << "NH_DstToDest"
       << setw(15) << "dsr"
       << setw(20) << "dsd(|D-RSU|)"
       << setw(15) << "drd"
       << setw(16) << "Density"
       << setw(15) << "wdist"
       << setw(15) << "ABE"
       << setw(15) << "ArrivalTime"
       << endl;

    while (curr != nullptr) {
        EV << setw(5)  << curr->typeNode
           << setw(15) << curr->idMsg
           << setw(15) << curr->idVehicle
           << setw(15) << curr->NH_Address
           << setw(15) << curr->NH_GS
           << setw(15) << curr->NH_Dst_to_Dest
           << setw(20) << curr->dsr
           << setw(15) << curr->dsd
           << setw(20) << curr->drd
           << setw(15) << curr->Nv
           << setw(15) << curr->wdist
           << setw(15) << curr->abe
           << setw(15) << curr->time
           << endl;
        curr = curr->next;
    }
    EV << string(160, '-') << endl;
}

// ---------------------------------------------------------------------------
// Top-of-table accessors (call SortBeacons() before using these)
// ---------------------------------------------------------------------------

int BeaconList::TopTable()
{
    return (head != nullptr) ? head->idVehicle : 0;
}

double BeaconList::TopTableNv()
{
    return (head != nullptr) ? head->Nv : 0.0;
}

Coord BeaconList::TopTableNghCoord()
{
    return (head != nullptr) ? head->SenderCoord : Coord();
}

double BeaconList::TopTable_U_dist()
{
    return (head != nullptr) ? head->wdist : 0.0;
}

double BeaconList::TopTabledisToDestination()
{
    return (head != nullptr) ? head->dsd : 0.0;
}

double BeaconList::topTableDtoS()
{
    return (head != nullptr) ? head->dsr : 0.0;
}

double BeaconList::TopTableGS()
{
    return (head != nullptr) ? head->NH_GS : 0.0;
}

// ---------------------------------------------------------------------------
// CounterBeacons – number of entries (used for local density computation)
// ---------------------------------------------------------------------------

int BeaconList::CounterBeacons()
{
    int counter = 0;
    curr = head;
    while (curr != nullptr) {
        ++counter;
        curr = curr->next;
    }
    return counter;
}

// ---------------------------------------------------------------------------
// Search utilities
// ---------------------------------------------------------------------------

bool BeaconList::SearchBeaconNodeID(int NodeID)
{
    curr = head;
    while (curr != nullptr) {
        if (curr->idVehicle == NodeID) return true;
        curr = curr->next;
    }
    return false;
}

bool BeaconList::SearchBeaconTreeID(int treeID)
{
    curr = head;
    while (curr != nullptr) {
        if (curr->idMsg == treeID) return true;
        curr = curr->next;
    }
    return false;
}

Coord BeaconList::SearchBeaconCoord(int NodeID)
{
    curr = head;
    while (curr != nullptr) {
        if (curr->idVehicle == NodeID) return curr->SenderCoord;
        curr = curr->next;
    }
    return Coord(); // not found – return default-constructed Coord
}

int BeaconList::SearchClstNODE(Coord DstNodeCoord)
{
    int    closestID  = 0;
    double minDist    = 4000.0; // initial threshold [m]

    curr = head;
    while (curr != nullptr) {
        if (curr->typeNode != "rsu") {
            double d = curr->SenderCoord.distance(DstNodeCoord);
            if (d < minDist) {
                minDist   = d;
                closestID = curr->idVehicle;
            }
        }
        curr = curr->next;
    }
    return closestID; // 0 if no vehicle neighbour is within threshold
}

int BeaconList::SearchBeaconRSU()
{
    curr = head;
    while (curr != nullptr) {
        if (curr->typeNode == "rsu") return curr->idVehicle;
        curr = curr->next;
    }
    return 0; // no RSU found
}

bool BeaconList::Search_RSU_NNT()
{
    curr = head;
    while (curr != nullptr) {
        if (curr->typeNode == "rsu") return true;
        curr = curr->next;
    }
    return false;
}
