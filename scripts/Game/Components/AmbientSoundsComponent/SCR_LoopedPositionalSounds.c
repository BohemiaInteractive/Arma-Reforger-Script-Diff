//! Handles looped sounds such as leaves rustles, or crickets played on entities close to camera
[BaseContainerProps(configRoot: true)]
class SCR_LoopedPositionalSounds : SCR_AmbientSoundsEffect
{
	private const int MINIMUM_MOVE_DISTANCE_SQ = 2;
	private const int INVALID = -1;	
	private const int LOOP_SOUND_HEIGHT_LIMIT = 25;
	private const int LOOP_SOUND_COUNT = 9;
    private const float BOUNDING_BOX_CROP_FACTOR = 0.3;
    private const float BOUNDING_BOX_CROP_FACTOR_HEIGHT = 0.9;
	private const int BUSH_LOOP_SOUND_COUNT_LIMIT = 4;
	private const string ENTITY_SIZE_SIGNAL_NAME = "EntitySize";
	private const string SEED_SIGNAL_NAME = "Seed";

	[Attribute("0.3", desc: "Ratio of bushes with insect sounds vs without")]
	private float m_fBushCricketThreshold;

	private int m_iEntitySizeSignalIdx;
	private int m_iSeedSignalIdx;
	
	//! Camera position where looped sounds were processed the last time	
	private vector m_vCameraPosLooped;

	//! All closest entities arround camera				
	private ref array<IEntity> m_aClosestEntityID = {};

	//! Entities with plaing looped sound	
	private ref array<IEntity> m_aLoopedSoundID = {};

	//! Playing sound audio handles
	private ref array<AudioHandle> m_aLoopedSoundAudioHandle = {};
		
	//------------------------------------------------------------------------------------------------
	// Called by SCR_AmbientSoundComponent in UpdateSoundJob()
	override void Update(float worldTime, vector cameraPos)
	{
		if (vector.DistanceSqXZ(m_vCameraPosLooped, cameraPos) < MINIMUM_MOVE_DISTANCE_SQ)
			return;
		
		m_vCameraPosLooped = cameraPos;
		
		m_aClosestEntityID.Clear();
					
		//Get closest entities array
		UpdateClosesEntitieyArray();
		
		// Stop sounds that are no longer among closest entities
		StopLoopedSounds();
		
		// Play sounds on entities, that become closest				
		PlayLoopedSounds(cameraPos);
	}

	//------------------------------------------------------------------------------------------------
	private void StopLoopedSounds()
	{		
		for (int i = m_aLoopedSoundID.Count() - 1; i >= 0; i--)
		{				
			int index = m_aClosestEntityID.Find(m_aLoopedSoundID[i]);
			
			if (index == INVALID)
			{
				m_aLoopedSoundID.Remove(i);
				m_AmbientSoundsComponent.Terminate(m_aLoopedSoundAudioHandle[i]);
				m_aLoopedSoundAudioHandle.Remove(i);			
			}		
		}
	}	
	
	//------------------------------------------------------------------------------------------------			
	private void PlayLoopedSounds(vector cameraPos)
	{
		foreach(IEntity entity: m_aClosestEntityID)
		{
			if (entity == null)
				continue;
			
			int index = m_aLoopedSoundID.Find(entity);
			
			if (index != INVALID)
				continue;
			
			float foliageHight;
			ETreeSoundTypes treeSoundType				
			GetTreeProperties(entity, treeSoundType, foliageHight);
				
			// Get event name
			string eventName;
								
			if (treeSoundType == ETreeSoundTypes.Leafy)
			{					
				eventName = GetTreeSoundEventName(foliageHight, treeSoundType);
			}
			else
			{
				vector v = entity.GetOrigin();
				int seed = v[0] * v[2];
				RandomGenerator ranGen = SCR_Math.GetMathRandomGenerator();
				ranGen.SetSeed(seed);

				if (ranGen.RandFloat01() > m_fBushCricketThreshold)
				{
					float sampleSelector = Math.AbsInt(seed % 100) * 0.01;
					m_LocalSignalsManager.SetSignalValue(m_iSeedSignalIdx, sampleSelector);
					eventName = SCR_SoundEvent.SOUND_BUSH_CRICKETS_LP;
				}
				else
				{
					eventName = SCR_SoundEvent.SOUND_BUSH_LP;
				}
			}
											
			// Get sound position
			vector mat[4];
			Math3D.MatrixIdentity4(mat);
				
			// Get world bounding box
			vector mins, maxs;			
			entity.GetWorldBounds(mins, maxs);									
										
			// Get height	
			float BVHeight = (maxs[1] - mins[1] - foliageHight) * BOUNDING_BOX_CROP_FACTOR_HEIGHT;
							
			// Get average width
			float widthX = (maxs[0] - mins[0]) * 0.5;
			float widthZ = (maxs[2] - mins[2]) * 0.5;
			float BVRadius = 0.5 * (widthX + widthZ) * BOUNDING_BOX_CROP_FACTOR;
								
			// Get center					
			mat[3][0] = mins[0] + widthX;
			mat[3][1] = mins[1] + foliageHight + 0.5 * BVHeight;
			mat[3][2] = mins[2] + widthZ;
					
			// Play sound					
			AudioHandle audioHandle = m_AmbientSoundsComponent.SoundEventTransform(eventName, mat);
			if (!m_AmbientSoundsComponent.IsHandleValid(audioHandle))
				continue;	
			
			// Set BV parameters	
			AudioSystem.SetBoundingVolumeParams(audioHandle, AudioSystem.BV_Cylinder, BVRadius, BVHeight, 0);
				
			// Store references
			m_aLoopedSoundID.Insert(entity);
			m_aLoopedSoundAudioHandle.Insert(audioHandle);									
		}
	}
		
	//------------------------------------------------------------------------------------------------
	private void UpdateClosesEntitieyArray()
	{
		m_AmbientSoundsComponent.GetClosestEntities(EQueryType.TreeBush, BUSH_LOOP_SOUND_COUNT_LIMIT, m_aClosestEntityID);
		m_AmbientSoundsComponent.GetClosestEntities(EQueryType.TreeLeafy, LOOP_SOUND_COUNT, m_aClosestEntityID);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetTreeProperties(IEntity entity, out ETreeSoundTypes soundType, out float foliageHeight)
	{
		// Get tree entity
		Tree tree = Tree.Cast(entity);
		if (!tree)
			return;
		
		// Get prefab data
		TreeClass treeClass = TreeClass.Cast(tree.GetPrefabData());
		if (!treeClass)
			return;
		
		foliageHeight = treeClass.m_iFoliageHeight * entity.GetScale();	
		soundType = treeClass.SoundType;
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetTreeSoundEventName(float foliageHeight, ETreeSoundTypes treeSoundType)
	{
			if (foliageHeight < 3)
				return SCR_SoundEvent.SOUND_LEAFYTREE_SMALL_LP;
			else if (foliageHeight < 5)
				return SCR_SoundEvent.SOUND_LEAFYTREE_MEDIUM_LP;
			else if (foliageHeight < 7)
				return SCR_SoundEvent.SOUND_LEAFYTREE_LARGE_LP;
			else
				return SCR_SoundEvent.SOUND_LEAFYTREE_VERYLARGE_LP;	
	}
	
	//------------------------------------------------------------------------------------------------	
	// Called by SCR_AmbientSoundComponent in EOnPostInit()
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, signalsManagerComponent);

		m_iEntitySizeSignalIdx = signalsManagerComponent.AddOrFindSignal(ENTITY_SIZE_SIGNAL_NAME);
		m_iSeedSignalIdx = signalsManagerComponent.AddOrFindSignal(SEED_SIGNAL_NAME);
	}
}
