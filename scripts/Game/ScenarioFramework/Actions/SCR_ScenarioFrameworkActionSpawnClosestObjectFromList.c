[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnClosestObjectFromList : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "", desc: "The closest one from the list will be spawned")]
	ref array<string> m_aListOfObjects;
	
	[Attribute(defvalue: SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION.ToString(), uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType))]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!m_Getter)
		{
			if (object)
			{
				entity = object;
			}
			else
			{
			Print(string.Format("ScenarioFramework Action: The object the distance is calculated from is missing!"), LogLevel.ERROR);
			return;
		}
		}

		if (!entity)
		{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			{
				Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
				return;
			}

			entity = entityWrapper.GetValue();
		}

		IEntity entityInList;
		SCR_ScenarioFrameworkLayerBase selectedLayer;
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Getter returned null object. Random object spawned instead."), LogLevel.WARNING);
			array<string> aRandomObjectToSpawn = {};
			aRandomObjectToSpawn.Insert(m_aListOfObjects[m_aListOfObjects.GetRandomIndex()]);

			entityInList = GetGame().GetWorld().FindEntityByName(aRandomObjectToSpawn[0]);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				return;
			}

			SpawnObjects(aRandomObjectToSpawn, m_eActivationType);
			return;
		}

		IEntity closestEntity;
		float fDistance = float.MAX;
		foreach (string sObjectName : m_aListOfObjects)
		{
			entityInList = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Object %1 doesn't exist", sObjectName), LogLevel.ERROR);
				continue;
			}
			
			selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(entityInList.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!selectedLayer)
				continue;

			float fActualDistance = Math.AbsFloat(vector.Distance(entity.GetOrigin(), entityInList.GetOrigin()));

			if (fActualDistance < fDistance)
			{
				closestEntity = entityInList;
				fDistance = fActualDistance;
			}
		}

		if (!closestEntity)
			return;

		selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(closestEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));

		if (selectedLayer)
		{
			selectedLayer.Init(null, m_eActivationType);
			selectedLayer.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
		}
		else
		{
			Print(string.Format("ScenarioFramework Action: Can't spawn slot %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", closestEntity.GetName()), LogLevel.ERROR);
		}
	}
}