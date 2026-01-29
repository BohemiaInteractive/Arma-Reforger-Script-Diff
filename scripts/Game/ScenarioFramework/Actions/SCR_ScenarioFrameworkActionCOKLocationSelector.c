[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCOKLocationSelector : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", desc: "Selection of Areas")]
	ref array<string> m_aSelectionOfAreas;

	[Attribute(defvalue: "Exfil_Area", desc: "Exfil Area Name")]
	string m_sExfilAreaName;

	[Attribute(defvalue: "10", desc: "Marker Search Distance", params: "0 inf 0.01")]
	float m_fMarkerSearchDistance;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_aSelectionOfAreas || m_aSelectionOfAreas.Count() < 2)
			return;

		//Prepare starting location
		const string startingLocation = m_aSelectionOfAreas.GetRandomElement();
		m_aSelectionOfAreas.RemoveItem(startingLocation);
		SpawnObjects({startingLocation}, SCR_ScenarioFrameworkEActivationType.ON_INIT);

		//Prepare exfil location
		const string endingLocation = m_aSelectionOfAreas.GetRandomElement();
		IEntity endLocationEnt = GetGame().GetWorld().FindEntityByName(endingLocation);
		if (endLocationEnt)
		{
			MoveExfil(endLocationEnt);
			m_aSelectionOfAreas.RemoveItem(endingLocation);
		}
		
		//Spawn rest of PB markers
		SpawnMarkers();
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnMarkers()
	{
		IEntity ent;
		SCR_ScenarioFrameworkSlotMarker slotMarker;
		foreach(string selection : m_aSelectionOfAreas)
		{
			ent = GetGame().GetWorld().FindEntityByName(selection);
			if (!ent)
				continue;
			
			ent = ent.GetChildren();
			while (ent)
			{
				slotMarker = SCR_ScenarioFrameworkSlotMarker.Cast(ent.FindComponent(SCR_ScenarioFrameworkSlotMarker));
				if (slotMarker)
				{
					SpawnObjects({slotMarker.GetName()}, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
					break;
				}
				
				ent = ent.GetSibling();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MoveExfil(notnull IEntity ent)
	{
		SCR_ScenarioFrameworkSlotMarker existingMarkerSlot;
		IEntity child = ent.GetChildren();
		while (child)
		{
			existingMarkerSlot = SCR_ScenarioFrameworkSlotMarker.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotMarker));
			if (existingMarkerSlot)
				break;
			
			child = child.GetSibling();
		}

		if (!existingMarkerSlot)
			return;
		
		SCR_MapMarkerBase exfilMarker;
		IEntity exfilArea = GetGame().GetWorld().FindEntityByName(m_sExfilAreaName);
		child = exfilArea.GetChildren();
		
		// Place exfil logic area at the area marker position
		exfilArea.SetOrigin(ent.GetOrigin());
		exfilArea.Update();
		
		while (child)
		{
			SCR_ScenarioFrameworkSlotMarker exfilSlotMarker = SCR_ScenarioFrameworkSlotMarker.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotMarker));
			if (exfilSlotMarker)
			{
				SpawnObjects({exfilSlotMarker.GetOwner().GetName()}, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
				const SCR_MapMarkerBase marker = exfilSlotMarker.GetMapMarker();
				if (marker)
				{
					exfilMarker = marker;
					break;
				}
			}

			child = child.GetSibling();
		}

		if (!exfilMarker)
			return;

		// Copy random exfil location name onto exfil marker
		SCR_ScenarioFrameworkMarkerCustom mapMarkerType = SCR_ScenarioFrameworkMarkerCustom.Cast(existingMarkerSlot.GetMapMarkerType());
		if (mapMarkerType)
			exfilMarker.SetCustomText(mapMarkerType.m_sMapMarkerText);
	}
}
