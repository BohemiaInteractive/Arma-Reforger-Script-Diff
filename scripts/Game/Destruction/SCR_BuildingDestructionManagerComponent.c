[EntityEditorProps(category: "GameScripted/Destruction", description: "Building destruction manager, stores data for destroyed buildings.")]
class SCR_BuildingDestructionManagerComponentClass : ScriptComponentClass
{
}

class SCR_BuildingDestructionData
{
	int m_iNextFreeIndex = -1;
	vector m_vTargetOrigin;
	vector m_vStartAngles;
	vector m_vStartMatrix[4];
	vector m_vTargetAngles;
	int m_iRotatedTimes;
	int m_iRotationTime;
	int m_iRotationMultiplier = -1;
	int m_iChecksThisFrame;
	int m_iPauseTime;
	float m_fRotationSpeedMultiplier = 1;
	int m_iRotationStart;
	float m_fSpeedMultiplier = 1;
	ref RandomGenerator m_RandomGenerator = new RandomGenerator();
	ref array<IEntity> m_aQueriedEntities;
	ref set<int> m_aExecutedEffectIndices;
	ref array<IEntity> m_aExcludeList;
	ref array<ref Tuple2<vector, vector>> m_aNavmeshAreas;
	ref array<bool> m_aRedoRoads;
	ref SCR_BuildingDestructionCameraShakeProgress m_CameraShake = InitCameraShake();
	SCR_AudioSource m_AudioSource;
	float m_fBuildingVolume;
	float m_fSizeMultiplier;
	
	//------------------------------------------------------------------------------------------------
	//! Sets variables to default values when freed to be reused by another object on demand
	void Reset()
	{
		m_vTargetOrigin = vector.Zero;
		m_vStartAngles = vector.Zero;
		Math3D.MatrixIdentity4(m_vStartMatrix);
		m_vTargetAngles = vector.Zero;
		m_iRotatedTimes = 0;
		m_iRotationTime = 0;
		m_iRotationMultiplier = -1;
		m_iChecksThisFrame = 0;
		m_iPauseTime = 0;
		m_fRotationSpeedMultiplier = 1;
		m_iRotationStart = 0;
		m_fSpeedMultiplier = 1;
		m_RandomGenerator = new RandomGenerator();
		m_aQueriedEntities = null;
		m_aExecutedEffectIndices = null;
		m_aExcludeList = null;
		m_aNavmeshAreas = null;
		m_aRedoRoads = null;
		m_CameraShake = InitCameraShake();
		m_AudioSource = null;
		m_fBuildingVolume = 0;
	}

	static SCR_BuildingDestructionCameraShakeProgress InitCameraShake()
	{
		// No camera shake for headless
		if (System.IsConsoleApp())
			return null;
		
		return new SCR_BuildingDestructionCameraShakeProgress();
	}
}

class SCR_BuildingDestructionManagerComponent : ScriptComponent
{			
	[Attribute()]
	protected ref SCR_BuildingDestructionConfig m_BuildingDestructionConfig;
	
	private int m_iFirstFreeData = -1;
	private ref array<ref SCR_BuildingDestructionData> m_aBuildingDestructionData = {};
	
	protected ref array<SCR_DestructibleBuildingComponent> m_aDestroyedBuildings = {};
	protected static int s_iHighestId;
	protected static ref map<int, SCR_DestructibleBuildingComponent> s_mBuildingIds = new map<int, SCR_DestructibleBuildingComponent>();
	
	//------------------------------------------------------------------------------------------------
	//! Returns data stored at provided index
	//! \param[in,out] index
	notnull SCR_BuildingDestructionData GetData(inout int index)
	{
		if (index == -1)
			index = AllocateData();
		
		return m_aBuildingDestructionData[index];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Allocates data for the objects storing it here
	//! \return index of the stored data
	private int AllocateData()
	{
		if (m_iFirstFreeData == -1)
			return m_aBuildingDestructionData.Insert(new SCR_BuildingDestructionData());
		
		int returnIndex = m_iFirstFreeData;
		SCR_BuildingDestructionData data = m_aBuildingDestructionData[returnIndex];
		m_iFirstFreeData = data.m_iNextFreeIndex;
		data.m_iNextFreeIndex = -1;
		return returnIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Public to be used by objects storing data here
	//! \param[in] index
	void FreeData(int index)
	{
		// If you got a VME here, please report it only if you didn't touch the array/index
		// Otherwise no guarantees
		m_aBuildingDestructionData[index].Reset();
		m_aBuildingDestructionData[index].m_iNextFreeIndex = m_iFirstFreeData;
		m_iFirstFreeData = index;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return array of typenames to be excluded from entity query
	array<typename> GetExcludedQueryTypes()
	{
		array<typename> outArray = {};
		if (!m_BuildingDestructionConfig)
			return outArray;
		else 
			outArray.Copy(m_BuildingDestructionConfig.m_aExcludedEntityQueryTypenames);

		return outArray;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BuildingDestructionManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		GetGame().RegisterBuildingDestructionManager(this);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BuildingDestructionManagerComponent()
	{
		GetGame().UnregisterBuildingDestructionManager(this);
	}
}
