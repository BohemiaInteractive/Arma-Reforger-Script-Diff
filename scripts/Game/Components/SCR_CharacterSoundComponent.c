[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_CharacterSoundComponentClass : CharacterSoundComponentClass
{
	[Attribute(uiwidget: UIWidgets.Object, category: "Bone Speed Signals")]
	protected ref array<ref SCR_BoneSpeedSignalData> m_aBoneSpeedSignalData;
	
	array<ref SCR_BoneSpeedSignalData> GetBoneSpeedSignalData()
	{
		return m_aBoneSpeedSignalData;
	}
}

class SCR_CharacterSoundComponent : CharacterSoundComponent
{	
	protected SignalsManagerComponent m_SignalsManagerComponent;
		
	protected ref array<int> m_aSignalIdx;
	protected ref array<int> m_aTrackedBoneIdx;
	protected ref array<int> m_aPivotBoneIdx;
				
	protected ref array<vector> m_aBonePositionLast;
			
	//------------------------------------------------------------------------------------------------
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{	
		if (timeSlice == 0)
			return;
		
		const float perSecondFactor = 1 / timeSlice;
						
		foreach (int index, int trackedBoneIdx : m_aTrackedBoneIdx)
		{
			const vector bonePosVector = GetBonePosVector(owner, index, trackedBoneIdx);
			float distance;			
			const int pivotBoneIndex = m_aPivotBoneIdx[index];
			
			if (pivotBoneIndex == -1)
			{
				distance = vector.Distance(bonePosVector, m_aBonePositionLast[index]);
				
				m_aBonePositionLast[index] = bonePosVector;		
			}
			else
			{			
				const float pivotBoneDistance = vector.Distance(bonePosVector, GetBonePosVector(owner, index, pivotBoneIndex));
				
				const vector lastDistanceVector = m_aBonePositionLast.Get(index);
				float lastPivotBoneDistance = lastDistanceVector[0];
				
				distance = Math.AbsFloat(lastPivotBoneDistance - pivotBoneDistance);	
								
				m_aBonePositionLast.Set(index, {pivotBoneDistance, 0, 0});
			}
			
			const float boneSpeed = distance * perSecondFactor;
			
			m_SignalsManagerComponent.SetSignalValue(m_aSignalIdx[index], boneSpeed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetBonePosVector(IEntity owner, int index, int boneIdx)
	{
		vector mat[4];
		owner.GetAnimation().GetBoneMatrix(boneIdx, mat);
		return mat[3];
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));		
		const SCR_CharacterSoundComponentClass componentData = GetComponentData();
		
		if (!m_SignalsManagerComponent || !componentData)
		{
			SetScriptedMethodsCall(false);
			return;
		}

		// Prepare arrays
		m_aSignalIdx = {};
		m_aTrackedBoneIdx = {};
		m_aPivotBoneIdx = {};
		m_aBonePositionLast = {};
		
		const array<ref SCR_BoneSpeedSignalData> prefabFoleyData = componentData.GetBoneSpeedSignalData();		
		const Animation currentAnim = owner.GetAnimation();
							
		foreach (int index, SCR_BoneSpeedSignalData foleyData : prefabFoleyData)
		{
			const string signalName = foleyData.GetSignalName();
			
			if (signalName.IsEmpty())
				continue;
						
			const int boneIndex = currentAnim.GetBoneIndex(foleyData.GetTrackedBoneName());
			if (boneIndex != -1)
			{
				const string pivotBone = foleyData.GetPivotBoneName();
				
				int pivotBoneIndex;
				
				if (pivotBone.IsEmpty())
				{
					pivotBoneIndex = -1;
				}
				else
				{
					pivotBoneIndex = owner.GetAnimation().GetBoneIndex(pivotBone);
					if (pivotBoneIndex == -1)
						Print(string.Format("'SCR_CharacterSoundComponent': PivotBone name with index position '%1' does not exist. The tracked bone will not be tracked in relation to it.", index), LogLevel.WARNING);
				}
				
				m_aTrackedBoneIdx.Insert(boneIndex);
				m_aPivotBoneIdx.Insert(pivotBoneIndex);
				
				m_aSignalIdx.Insert(m_SignalsManagerComponent.AddOrFindSignal(signalName));
			}
			else
			{
				Print(string.Format("'SCR_CharacterSoundComponent': Tracked bone name with index position '%1' does not exist on entity and can not be tracked.", index), LogLevel.WARNING);
			}
		}
		
		m_aBonePositionLast.Resize(m_aTrackedBoneIdx.Count()); 		
	}
		
	//------------------------------------------------------------------------------------------------
	SCR_CharacterSoundComponentClass GetComponentData()
	{
		SCR_CharacterSoundComponentClass componentData = SCR_CharacterSoundComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return null;
		
		return componentData;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CharacterSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	
	}
}