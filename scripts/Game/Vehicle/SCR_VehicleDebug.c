[EntityEditorProps(category: "GameCode/Vehicles", description: "Debug tool that places a player character in a specified vehicle on game start in WE.", color: "0 255 255 255")]
class SCR_VehicleDebugClass : GenericEntityClass
{
}

class SCR_VehicleDebug : GenericEntity
{
	[Attribute(desc: "Name of vehicle entity a player starts in")]
	string m_VehicleName;
	[Attribute("0", UIWidgets.ComboBox, "Compartment type to be used (if available)", "", ParamEnumArray.FromEnum(ECompartmentType))]
	ECompartmentType m_CompartmentType;
	[Attribute("0", UIWidgets.CheckBox, "Engine will be started (if possible)")]
	bool m_StartEngine;
	[Attribute("", UIWidgets.ResourceNamePicker, "Overrides surface material of wheel/track casts", params: "gamemat")]
	ResourceName m_SurfaceMaterialOverride;
	
	void SCR_VehicleDebug(IEntitySource src, IEntity parent)
	{
		GetGame().GetCallqueue().CallLater(DelayedInit, 1);
	}
	
	void DelayedInit()
	{
		if (!m_SurfaceMaterialOverride.IsEmpty())		
			VehicleBaseSimulation.SetSurfaceOverride(m_SurfaceMaterialOverride);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
	}
	
	void OnPlayerSpawned(int playerId, IEntity playerEntity)
	{
		IEntity vehicleEntity = GetGame().GetWorld().FindEntityByName(m_VehicleName);
		GetInVehicle(playerEntity, vehicleEntity);
	}
	
	void GetInVehicle(IEntity player, IEntity vehicle)
	{
		if (!vehicle)
			return;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		array<BaseCompartmentSlot> slots = {};
		compartmentManager.GetFreeCompartmentsOfType(slots, m_CompartmentType);
		if (slots.IsEmpty())
			return;
		
		BaseCompartmentSlot slot = slots[0];
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(player.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
		
		compartmentAccess.GetInVehicle(vehicle, slot, true, -1, ECloseDoorAfterActions.INVALID, true);
		
		if (!m_StartEngine)
			return;
		
		BaseVehicleControllerComponent controller = BaseVehicleControllerComponent.Cast(vehicle.FindComponent(BaseVehicleControllerComponent));
		if (!controller)
			return;
		
		controller.ForceStartEngine();
	}
}
