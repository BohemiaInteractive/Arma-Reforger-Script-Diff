/*
===========================================
Do not modify, this script is generated
===========================================
*/

class CompartmentDoorInfo: BaseCompartmentDoor
{
	proto external bool UseTeleportForGetIn();
	proto external bool UseTeleportForGetOut();
	proto external PointInfo GetEntryPointInfo();
	proto external PointInfo GetAIEntryPointInfo();
	proto external PointInfo GetAIJumpPointInfo();
	proto external PointInfo GetExitPointInfo();
	proto external bool HasExitPointInfoDefined();
	proto external ECharacterStanceChange GetStanceChangeOnExit();
}
