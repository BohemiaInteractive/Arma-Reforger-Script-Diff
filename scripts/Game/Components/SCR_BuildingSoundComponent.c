enum SCR_ESpawnType
{
	BOUNDING_BOX,
	POINTS
}

[ComponentEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.")]
class SCR_BuildingSoundComponentClass : SoundComponentClass
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Spawning type", "", enumType: SCR_ESpawnType, category: "Spawn Setup")]
	SCR_ESpawnType m_eSpawnType;
	
	[Attribute("0", UIWidgets.Slider, "Minimum distance from selected point for sound to be played [m]", "0 60 1", category: "Spawn Setup")]
	int m_iMinimumDistance;
	
	[Attribute("8", UIWidgets.Slider, "Time interval when maximum WindSpeed [s]", "0 60 1", category: "Spawn Setup")]
	int m_iTimeIntervalMin;
	
	[Attribute("20", UIWidgets.Slider, "Time interval when minimum WindSpeed [s]", "0 60 1", category: "Spawn Setup")]
	int m_iTimeIntervalMax;
				
	[Attribute("0 0 0", UIWidgets.Coords, "Mins OOB Point", category: "Spawn Setup")]
	vector m_vMins;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Maxs OOB Point", category: "Spawn Setup")]
	vector m_vMaxs;
	
	[Attribute(category: "Spawn Setup")]
	ref array<ref PointInfo> m_aSpawnPoints;
	
	[Attribute("0", UIWidgets.Auto, "Update Interior signal", category: "Spawn Setup")]
	bool m_bInteriorSignal;
	
	void SCR_BuildingSoundComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		// Make sure m_vMins is smaller than m_vMaxs.
		// This is necessary because these values are used as inputs for Math.RandomFloat(a,b) that requires that a < b.
		bool invalid = false;
		for (int i=0; i<3; ++i)
		{
			// Handle wrong inputs
			if (m_vMins[i] > m_vMaxs[i])
			{
				m_vMaxs[i] = m_vMins[i] + 0.1;
				invalid = true;
			}
		}
		
		// Display the warning in the edit mode. We don't want the play mode spammed.
		// The idea is that the author catches their mistake in the log.
		if (invalid && SCR_Global.IsEditMode())
		{
			if (prefabSource)
				Print(string.Format("Requirement not met: Mins <= Maxs on %1 %2", this, prefabSource.GetName()), LogLevel.WARNING);
			else
				Print("Requirement not met: Mins <= Maxs on " + this, LogLevel.WARNING);
		}
	}
	
	private float CalculateLocalOffset(int i)
	{
		if (m_vMins[i]==m_vMaxs[i])
			return m_vMins[i];
		
		return Math.RandomFloat(m_vMins[i], m_vMaxs[i]);
	}
	
	vector CalculateOBBLocalOffset()
	{
		vector v;
		v[0] = CalculateLocalOffset(0);
		v[1] = CalculateLocalOffset(1);
		v[2] = CalculateLocalOffset(2);
		return v;
	}
	
	vector CalculatePointsLocalOffset()
	{		
		vector mat[4];
		m_aSpawnPoints.GetRandomElement().GetLocalTransform(mat);		
		
		return mat[3];
	}
}

class SCR_BuildingSoundComponent : SoundComponent
{
	private static const float RANDOM_PERCENTAGE_MIN = 0.7;
	private static const float RANDOM_PERCENTAGE_MAX = 1.3;
		
	protected float m_fTriggerInterval;

	//------------------------------------------------------------------------------------------------				
	private void TriggeredSoundHandler(IEntity owner, float timeSlice)
	{
		// Get prefab data
		SCR_BuildingSoundComponentClass prefabData = SCR_BuildingSoundComponentClass.Cast(GetComponentData(owner));
		
		// Get remetition time
		GetTimeInterval(prefabData);
		
		// Get sound position
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = GetSoundPosition(prefabData);
		
		// Check distance	
		if (prefabData.m_iMinimumDistance > 0)
		{
			vector cameraMat[4];
			owner.GetWorld().GetCurrentCamera(cameraMat);
			
			// Dont play sound if sound position is too close to listener
			if (vector.Distance(mat[3], cameraMat[3]) < prefabData.m_iMinimumDistance)
			{
				return;
			}
		}
	
		//Set Interior signal
		if (prefabData.m_bInteriorSignal)
		{
			SetInteriorSignal(owner);
		}

		// Play sound
		SoundEventTransform(SCR_SoundEvent.SOUND_CREAK, mat);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetInteriorSignal(IEntity owner)
	{						
		bool interior;
				
		float gInterior = AudioSystem.GetVariableValue("GInterior", "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf");
		
		if (gInterior == 1)
		{
			vector mat[4];
			owner.GetWorld().GetCurrentCamera(mat);
						
			interior = IsInWorldBounds(mat[3], owner);
		}
	
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));		
		if (signalsManagerComponent)
			signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal("Interior"), interior);		
	}
	
	//------------------------------------------------------------------------------------------------
	private bool IsInWorldBounds(vector point, IEntity entity)
	{
		vector mins = vector.Zero;
		vector maxs = vector.Zero;
		
		entity.GetWorldBounds(mins, maxs);
			
		return !(point[0] < mins[0] || point[0] > maxs[0] || point[1] < mins[1] || point[1] > maxs[1] || point[2] < mins[2] || point[2] > maxs[2]);
	}
	
	//------------------------------------------------------------------------------------------------
	private void GetTimeInterval(SCR_BuildingSoundComponentClass prefabData)
	{
		GameSignalsManager signals = GetGame().GetSignalsManager();
		float windSpeed = signals.GetSignalValue(signals.AddOrFindSignal("WindSpeed"));
		float timeInterval = Interpolate(windSpeed, SCR_AmbientSoundsComponent.WINDSPEED_MIN, SCR_AmbientSoundsComponent.WINDSPEED_MAX, prefabData.m_iTimeIntervalMax, prefabData.m_iTimeIntervalMin);
			
		m_fTriggerInterval = Math.RandomFloat(timeInterval * RANDOM_PERCENTAGE_MIN, timeInterval * RANDOM_PERCENTAGE_MAX);
	}
	
	//------------------------------------------------------------------------------------------------
	private vector GetSoundPosition(SCR_BuildingSoundComponentClass prefabData)
	{		
		vector v;
		
		if (prefabData.m_eSpawnType == SCR_ESpawnType.BOUNDING_BOX)
		{
			v = prefabData.CalculateOBBLocalOffset();
		}
		else
		{
			v = prefabData.CalculatePointsLocalOffset();
		}
		
		// From Local To World Space
		return GetOwner().CoordToParent(v);
	}

	//------------------------------------------------------------------------------------------------
	private float Interpolate(float in, float Xmin, float Xmax, float Ymin, float Ymax)
	{
		if (in <= Xmin)
			return Ymin;
			
		if (in >= Xmax)
			return Ymax;
			
		return ((Ymin * (Xmax - in) + Ymax * (in - Xmin)) / (Xmax - Xmin));		
	}
		
	//------------------------------------------------------------------------------------------------		
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		super.UpdateSoundJob(owner, timeSlice);
		
		m_fTriggerInterval -= timeSlice;
	
		if (m_fTriggerInterval > 0)
			return;
		
		TriggeredSoundHandler(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdateSoundJobBegin(IEntity owner)
	{
		GetTimeInterval(SCR_BuildingSoundComponentClass.Cast(GetComponentData(owner)));
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_BuildingSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SCR_BuildingSoundComponentClass prefabData = SCR_BuildingSoundComponentClass.Cast(GetComponentData(GetOwner()));
		
		if (prefabData.m_eSpawnType == SCR_ESpawnType.BOUNDING_BOX)
		{
			if (prefabData.m_vMins == vector.Zero && prefabData.m_vMaxs == vector.Zero)
			{
				SetScriptedMethodsCall(false);
				return;
			}	
		}
		else if (prefabData.m_eSpawnType == SCR_ESpawnType.POINTS)
		{
			if (!prefabData.m_aSpawnPoints)
			{
				SetScriptedMethodsCall(false);
				return;
			}
			
			if (prefabData.m_aSpawnPoints.IsEmpty())
			{
				SetScriptedMethodsCall(false);
				return;
			}
		}
		
		SetScriptedMethodsCall(true);	
	}
}