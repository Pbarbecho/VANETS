//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Pablo Barbecho – Universidad de Cuenca
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include <string>
#include <iomanip>

#include "veins/veins.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/base/utils/SimpleAddress.h"

/**
 * @file beacon.h
 * @brief Neighbour Node Table (NNT) for VANET routing in Veins.
 *
 * BeaconList implements a singly-linked list that stores one entry per
 * neighbouring node.  Each entry (beacon) holds the routing metrics
 * received in the last periodic beacon broadcast from that neighbour:
 * position, distances, density, available bandwidth, and next-hop info.
 *
 * Typical usage inside an application layer:
 * @code
 *   BeaconList nnt;                          // one table per vehicle/RSU
 *
 *   // On beacon reception:
 *   if (!nnt.SearchBeaconNodeID(senderId))
 *       nnt.AddBeacon(...);
 *   else
 *       nnt.UpdateBeacon(...);
 *   nnt.PurgeBeacons(2.0);   // remove entries older than 2 s
 *   nnt.SortBeacons();       // rank by composite score
 *
 *   // Forwarding decision:
 *   int nextHop = nnt.TopTable();
 * @endcode
 */
class BeaconList {

private:
    /**
     * @brief One entry of the Neighbour Node Table.
     *
     * Fields
     * ------
     * - typeNode  : "rsu" for Road-Side Units, any other string for vehicles.
     * - idMsg     : tree/message ID of the last beacon received.
     * - idVehicle : MAC/node ID of the neighbour (key used for lookup).
     * - time      : simulation time [s] when this entry was last updated.
     * - SenderCoord : 2-D position of the neighbour at the time of the beacon.
     * - dsr       : distance [m] from the SOURCE of the original packet to this
     *               neighbour (Dist Source–Relay).
     * - dsd       : distance [m] from this neighbour to the DESTINATION (RSU)
     *               (Dist Sender–Destination).
     * - drd       : distance [m] from the current node to the destination
     *               (Dist Relay–Destination).
     * - Nv        : local vehicle density reported by the neighbour [normalised].
     * - wdist     : weighted distance score used by SortBeacons().
     * - abe       : available bandwidth estimate [bps] reported by the neighbour.
     * - NH_Address: L2 address of the neighbour's own selected next-hop.
     * - NH_GS     : global score of the neighbour's next-hop candidate.
     * - NH_Dst_to_Dest: distance [m] of the neighbour's next-hop to destination.
     */
    typedef struct beacon {
        std::string          typeNode;
        int                  idMsg      = 0;
        int                  idVehicle  = 0;
        double               time       = 0.0;
        veins::Coord         SenderCoord;
        double               dsr        = 0.0; ///< Dist Source–Relay
        double               dsd        = 0.0; ///< Dist Sender–Destination (to RSU)
        double               drd        = 0.0; ///< Dist Relay–Destination
        double               Nv         = 0.0; ///< neighbour density
        double               wdist      = 0.0; ///< weighted distance score
        double               abe        = 0.0; ///< available bandwidth estimate [bps]
        veins::LAddress::L2Type NH_Address    = 0;
        double               NH_GS      = 1.0; ///< next-hop global score
        double               NH_Dst_to_Dest = 0.0;
        beacon*              next       = nullptr;
    }* beaconPtr;

    beaconPtr head; ///< first entry of the linked list
    beaconPtr curr; ///< cursor used during traversal (not thread-safe)
    beaconPtr temp; ///< auxiliary pointer used during traversal

public:
    /** Initialises an empty Neighbour Node Table. */
    BeaconList();

    /** Destroys the table and releases all dynamically allocated entries. */
    ~BeaconList();

    // -----------------------------------------------------------------------
    // Mutators
    // -----------------------------------------------------------------------

    /**
     * @brief Appends a new entry at the tail of the NNT.
     *
     * Call this when SearchBeaconNodeID() returns false for the sender.
     *
     * @param b_typeNode      "rsu" or vehicle type string.
     * @param b_idMsg         Message/tree identifier.
     * @param b_idVehicle     Sender MAC/node ID (used as primary key).
     * @param b_time          Current simulation time [s].
     * @param b_SenderCoord   Sender position.
     * @param b_dsr           Distance source–relay [m].
     * @param b_dsd           Distance sender–destination [m].
     * @param b_drd           Distance relay–destination [m].
     * @param b_Nv            Neighbour density (normalised).
     * @param b_wdist         Weighted distance score.
     * @param b_abe           Available bandwidth estimate [bps].
     * @param b_NH_Address    Next-hop L2 address of the neighbour.
     * @param b_NH_GS         Global score of the neighbour's next-hop.
     * @param b_NH_Dst_to_Dest Distance of neighbour's next-hop to destination [m].
     */
    void AddBeacon(std::string b_typeNode, int b_idMsg, int b_idVehicle,
                   double b_time, veins::Coord b_SenderCoord,
                   double b_dsr, double b_dsd, double b_drd,
                   double b_Nv, double b_wdist, double b_abe,
                   veins::LAddress::L2Type b_NH_Address,
                   double b_NH_GS, double b_NH_Dst_to_Dest);

    /**
     * @brief Updates an existing entry identified by b_idVehicle.
     *
     * Call this when SearchBeaconNodeID() returns true for the sender.
     * All fields of the matching entry are overwritten with the new values.
     * If no entry with b_idVehicle exists the call is a no-op.
     */
    void UpdateBeacon(std::string b_typeNode, int b_idMsg, int b_idVehicle,
                      double b_time, veins::Coord b_SenderCoord,
                      double b_dsr, double b_dsd, double b_drd,
                      double b_Nv, double b_wdist, double b_abe,
                      veins::LAddress::L2Type b_NH_Address,
                      double b_NH_GS, double b_NH_Dst_to_Dest);

    /**
     * @brief Removes the entry with the given vehicle ID from the NNT.
     * @param b_idVehicle Node ID to remove.
     */
    void DeleteBeacon(int b_idVehicle);

    /**
     * @brief Removes all entries whose timestamp is older than
     *        (simTime() − b_ttl).
     * @param b_ttl Time-to-live threshold [s].  Typical value: 2.0 s.
     */
    void PurgeBeacons(double b_ttl);

    /**
     * @brief Sorts the NNT in descending order of a composite score
     *        (wdist + Nv) so that TopTable() returns the best candidate.
     *
     * Uses bubble sort — acceptable for the small tables typical in VANETs
     * (< 30 entries per vehicle at common inter-beacon intervals).
     */
    void SortBeacons();

    // -----------------------------------------------------------------------
    // Queries – top-of-table accessors (call SortBeacons() first)
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the vehicle ID of the best forwarding candidate
     *        (head of the sorted NNT), or 0 if the table is empty.
     */
    int TopTable();

    /**
     * @brief Returns the density (Nv) of the best forwarding candidate,
     *        or 0 if the table is empty.
     */
    double TopTableNv();

    /**
     * @brief Returns the position of the best forwarding candidate,
     *        or a default-constructed Coord if the table is empty.
     */
    veins::Coord TopTableNghCoord();

    /**
     * @brief Returns the weighted distance score (wdist) of the best
     *        forwarding candidate, or 0 if the table is empty.
     */
    double TopTable_U_dist();

    /**
     * @brief Returns the distance-to-destination (dsd) of the best
     *        forwarding candidate, or 0 if the table is empty.
     */
    double TopTabledisToDestination();

    /**
     * @brief Returns the distance source–relay (dsr) of the best
     *        forwarding candidate, or 0 if the table is empty.
     */
    double topTableDtoS();

    /**
     * @brief Returns the next-hop global score (NH_GS) of the best
     *        forwarding candidate, or 0 if the table is empty.
     */
    double TopTableGS();

    // -----------------------------------------------------------------------
    // Search utilities
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the number of entries currently in the NNT.
     *        Used to compute local vehicle density.
     */
    int CounterBeacons();

    /**
     * @brief Returns true if a node with the given vehicle ID is in the NNT.
     * @param NodeID Vehicle/node ID to search.
     */
    bool SearchBeaconNodeID(int NodeID);

    /**
     * @brief Returns true if a beacon with the given message/tree ID exists.
     * @param treeID Message identifier to search.
     */
    bool SearchBeaconTreeID(int treeID);

    /**
     * @brief Returns the position of the node with the given vehicle ID,
     *        or a default-constructed Coord if not found.
     * @param NodeID Vehicle/node ID to search.
     */
    veins::Coord SearchBeaconCoord(int NodeID);

    /**
     * @brief Finds the neighbour (excluding RSUs) closest to DstNodeCoord.
     * @param DstNodeCoord Target position (e.g. destination RSU).
     * @return Vehicle ID of the closest neighbour, or 0 if none exists.
     */
    int SearchClstNODE(veins::Coord DstNodeCoord);

    /**
     * @brief Returns the vehicle ID of the first RSU found in the NNT,
     *        or 0 if no RSU is present.
     */
    int SearchBeaconRSU();

    /**
     * @brief Returns true if at least one RSU entry exists in the NNT.
     */
    bool Search_RSU_NNT();

    // -----------------------------------------------------------------------
    // Debug
    // -----------------------------------------------------------------------

    /**
     * @brief Prints the full NNT to the OMNeT++ EV log.
     * @param CurrNodeAddress MAC address of the local node (for context).
     * @param IN_OUT          Label string (e.g. "IN" / "OUT") for the log.
     */
    void PrintBeacons(int CurrNodeAddress, std::string IN_OUT);
};
