[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnObjectBasedOnDistance : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Measure distance from what - use getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "It will select only objects that are at least x amount of meters away")]
	int m_iMinDistance;

	[Attribute(desc: "You can also set max distance to setup the hard limit of the max distance - but be aware that there might be a situation where it would not spawn anything.")]
	int m_iMaxDistance;

	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "List of objects that are to be compared")]
	ref array<string> 	m_aListOfObjects;

	[Attribute(defvalue: SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION.ToString(), uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType))]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	[Attribute(defvalue: "0", UIWidgets.ComboBox, desc: "Spawn all objects, only random one or random multiple ones?", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkSpawnChildrenType))]
	SCR_EScenarioFrameworkSpawnChildrenType m_SpawnObjects;

	[Attribute(defvalue: "100", desc: "If the RANDOM_MULTIPLE option is selected, what's the percentage? ", UIWidgets.Graph, "0 100 1")]
	int m_iRandomPercent;

	//------------------------------------------------------------------------------------------------
	void SpawnRandomObject(notnull array<string> aObjectsNames)
	{
		IEntity object = GetGame().GetWorld().FindEntityByName(aObjectsNames.GetRandomElement());
		if (!object)
		{
			Print(string.Format("ScenarioFramework Action: Can't spawn object set in slot %1. Slot doesn't exist", object), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", object), LogLevel.ERROR);
			return;
		}

		if (layer.GetActivationType() != m_eActivationType)
		{
			Print(string.Format("Can't spawn object %1 - the slot has %2 activation type set instead of %3", object.GetName(), layer.GetActivationType(), m_eActivationType), LogLevel.ERROR);
			return;
		}

		layer.Init(null, m_eActivationType);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnRandomMultipleObjects(notnull array<string> aObjectsNames)
	{
		array<SCR_ScenarioFrameworkLayerBase> aChildren = {};
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;
		SCR_ScenarioFrameworkLayerBase cachedLayer;
		foreach (string objectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(objectName);
			if (!object)
			{
				Print(string.Format("ScenarioFramework Action: Can't spawn object set in slot %1. Slot doesn't exist", objectName), LogLevel.ERROR);
				continue;
			}

			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("ScenarioFramework Action: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", objectName), LogLevel.ERROR);
				continue;
			}

			if (layer.GetActivationType() != m_eActivationType)
			{
				Print(string.Format("Can't spawn object %1 - the slot has %2 activation type set instead of %3", object.GetName(), layer.GetActivationType(), m_eActivationType), LogLevel.ERROR);
				continue;
			}

			if (!cachedLayer)
				cachedLayer = layer;

			if (!aChildren.Contains(layer))
				aChildren.Insert(layer);
		}

		if (aChildren.IsEmpty())
			return;

		if (m_SpawnObjects == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_BASED_ON_PLAYERS_COUNT)
			m_iRandomPercent = Math.Ceil(cachedLayer.GetPlayersCount() / cachedLayer.GetMaxPlayersForGameMode() * 100);

		int randomMultipleNumber = Math.Round(aObjectsNames.Count() * 0.01 * m_iRandomPercent);
		SCR_ScenarioFrameworkLayerBase child;
		for (int i = 1; i <= randomMultipleNumber; i++)
		{
			if (aChildren.IsEmpty())
				break;

			child = aChildren.GetRandomElement();
			child.Init(null, m_eActivationType);
			child.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
			aChildren.RemoveItem(child);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entityFrom;
		if (!m_Getter)
		{
			if (object)
			{
				entityFrom = object;
			}
			else
			{
				Print(string.Format("ScenarioFramework Action: The object the distance is calculated from is missing!"), LogLevel.ERROR);
				return;
			}
		}

		array<IEntity> aEntities = {};

		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> arrayOfEntitiesWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!arrayOfEntitiesWrapper)
				{
					Print(string.Format("ScenarioFramework Action: Issue with Array Getter detected for Action %1.", this), LogLevel.ERROR);
					return;
				}

				aEntities = arrayOfEntitiesWrapper.GetValue();
				if (!aEntities)
				{
					Print(string.Format("ScenarioFramework Action: Issue with retrieved array detected for Action %1.", this), LogLevel.ERROR);
					return;
				}

				if (!entityFrom && entityWrapper)
					entityFrom = entityWrapper.GetValue();
			}
		}

		bool entitiesAreEmpty = aEntities.IsEmpty();

		IEntity entityInList;
		SCR_ScenarioFrameworkLayerBase selectedLayer;
		if (!entityFrom && entitiesAreEmpty)
		{
			Print(string.Format("ScenarioFramework Action: Getter returned null object. Random object spawned instead."), LogLevel.WARNING);
			array<string> aRandomObjectToSpawn = {};
			aRandomObjectToSpawn.Insert(m_aListOfObjects[m_aListOfObjects.GetRandomIndex()]);

			entityInList = GetGame().GetWorld().FindEntityByName(aRandomObjectToSpawn[0]);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Object %1 doesn't exist", aRandomObjectToSpawn[0]), LogLevel.ERROR);
				return;
			}

			SpawnObjects(aRandomObjectToSpawn, m_eActivationType);
			return;
		}

		array<string> aObjectsNames = {};

		foreach (string objectName : m_aListOfObjects)
		{
			entityInList = GetGame().GetWorld().FindEntityByName(objectName);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Object %1 doesn't exist", objectName), LogLevel.ERROR);
				continue;
			}

			if (entitiesAreEmpty)
			{
				float fActualDistance = Math.AbsFloat(vector.Distance(entityFrom.GetOrigin(), entityInList.GetOrigin()));

				if (fActualDistance <= m_iMaxDistance && fActualDistance >= m_iMinDistance)
					aObjectsNames.Insert(objectName)
			}
			else
			{
				bool entityInRange;
				foreach (IEntity targetEntity : aEntities)
				{
					float fActualDistance = Math.AbsFloat(vector.Distance(targetEntity.GetOrigin(), entityInList.GetOrigin()));

					if (fActualDistance <= m_iMaxDistance && fActualDistance >= m_iMinDistance)
					{
						entityInRange = true;
					}
					else
					{
						entityInRange = false;
						break;
					}
				}

				if (entityInRange)
					aObjectsNames.Insert(objectName)
			}
		}

		if (m_SpawnObjects == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			SpawnRandomObject(aObjectsNames);
			return;
		}

		if (m_SpawnObjects == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			foreach (string objectName : aObjectsNames)
			{
				entityInList = GetGame().GetWorld().FindEntityByName(objectName);
				selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(entityInList.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (selectedLayer)
				{
					if (selectedLayer.GetActivationType() != m_eActivationType)
					{
						Print(string.Format("Can't spawn object %1 - the slot has %2 activation type set instead of %3", selectedLayer.GetName(), selectedLayer.GetActivationType(), m_eActivationType), LogLevel.ERROR);
						continue;
					}

					SCR_ScenarioFrameworkArea area = selectedLayer.GetParentArea();
					if (!area)
						continue;

					area.SetAreaSelected(true);
					SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(selectedLayer);
					if (layerTask)
						selectedLayer.GetParentArea().SetLayerTask(layerTask);

					selectedLayer.Init(area, m_eActivationType);
					selectedLayer.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
				}
				else
				{
					Print(string.Format("ScenarioFramework Action: Can't spawn slot %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", selectedLayer.GetName()), LogLevel.ERROR);
				}
			}

			return;
		}

		SpawnRandomMultipleObjects(aObjectsNames);
	}
}