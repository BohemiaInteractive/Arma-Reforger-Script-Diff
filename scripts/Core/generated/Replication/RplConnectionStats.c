/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Replication
\{
*/

sealed class RplConnectionStats: Managed
{
	private void RplConnectionStats();

	/*!
	Gets round-trip time (ping) of the connection. Round-trip time says how much
	time it takes for a packet to travel over network from starting point to its
	destination and then back again to the starting point.
	\return     Round-trip time in milliseconds.
	*/
	proto external float GetRoundTripTimeInMs();
	/*!
	Gets ratio of packets sent that did not reach the destination. Ratio is value
	ranging from 0 (no packets are getting lost) to 1 (all packets are getting lost).
	\return     Packet loss ratio in range 0 to 1.
	*/
	proto external float GetPacketLoss();
}

/*!
\}
*/
