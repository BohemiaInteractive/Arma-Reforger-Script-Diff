[EntityEditorProps(category: "GameScripted/Buildings", description: "The main entity for a destructible building.", visible: false, dynamicBox: true)]
class SCR_DestructibleBuildingEntityClass: BuildingClass
{
	/*[Attribute("", UIWidgets.ResourceNamePicker, "The building configuration object to use for this building", "conf", category: "Destruction Building")]
	protected ResourceName m_sBuildingConfig;*/
	[Attribute("1", UIWidgets.EditBox, "Minimum damage above which the building switches to damaged state from the undamaged state", "0 1000000 0.5", category: "Destruction Building")]
	protected float m_fMinUndamagedThreshold;
	
#ifdef WORKBENCH
	[Attribute("0", UIWidgets.CheckBox, "Check to update various data in the config (must be defined)", category: "EDITOR: Destruction Building")]
	bool m_bUpdateConfigData;
	
	//------------------------------------------------------------------------------------------------
	bool UpdateConfigData()
	{
		return m_bUpdateConfigData;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	/*ResourceName GetBuildingConfig()
	{
		return m_sBuildingConfig;
	}*/
	
	//------------------------------------------------------------------------------------------------
	float GetMinUndamagedThreshold()
	{
		return m_fMinUndamagedThreshold;
	}
	
};

enum BuildingDamageState
{
	UNDAMAGED,
	DAMAGED,
	DESTROYED
};

//! Building switches between undamaged/damaged/destroyed states based on damage to regions and destruction
//! If a region is only partially damaged, its damage is stored in a map (m_RegionDamage) that stores index and damage
//! If a region is destroyed, it is stored in a bitmask array (m_RegionMask)
//! If the entire building is destroyed, only a single bool is set (m_bDestroyed)
//! This ensures a minimal amount of data is stored per building to handle destruction
//------------------------------------------------------------------------------------------------
class SCR_DestructibleBuildingEntity : Building
{
	[Attribute("", UIWidgets.ResourceNamePicker, "The building configuration object to use for this building", "conf", category: "Destruction Building")]
	protected ResourceName m_BuildingConfig;

	#ifdef ENABLE_BUILDING_DESTRUCTION
		#ifdef WORKBENCH
			[Attribute("0", UIWidgets.CheckBox, "Check to update various data in the config (must be defined)", category: "EDITOR: Destruction Building")]
			bool UpdateConfigData;
			
			//-----------------------------------------------------------------------
			override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
			{
				WorldEditorAPI api = _WB_GetEditorAPI();
				if (!api || api.UndoOrRedoIsRestoring())
					return false;
				
				if (key != "UpdateConfigData")
					return true;
				
				IEntitySource entSrc = src.ToEntitySource();
				entSrc.ClearVariable("UpdateConfigData");
				BaseContainerTools.WriteToInstance(this, entSrc);
				
				if (m_BuildingConfig == string.Empty)
				{
					Print("SCR_DestructibleBuildingEntity: Trying to update data in config, but config not defined!");
					return true;
				}
				
				m_BuildingSetup = SCR_BuildingConfigManagerEntity.GetBuildingSetupFromConfig(m_BuildingConfig);
				if (!m_BuildingSetup)
				{
					Print("SCR_DestructibleBuildingEntity: Trying to update data in config, but config not loaded!");
					return true;
				}
				
				Print("SCR_DestructibleBuildingEntity: Updating data in config...");
				
				m_BuildingSetup.FillDamagedModels();
				
				Resource configResource = BaseContainerTools.LoadContainer(m_BuildingConfig);
				BaseResourceObject configSource = configResource.GetResource();
				BaseContainer configBase = configSource.ToBaseContainer();
				configBase.Set("m_DamagedRegionModels", m_BuildingSetup.m_DamagedRegionModels);
				configBase.Set("m_DamagedRegionIntersectDebrisModels", m_BuildingSetup.m_DamagedRegionIntersectDebrisModels);
				if (BaseContainerTools.SaveContainer(configBase, m_BuildingConfig))
					Print("SCR_DestructibleBuildingEntity: Success! Config data updated and stored.");
				else
					Print("SCR_DestructibleBuildingEntity: Failed! Config data was not successfully stored!!");
				
				m_BuildingSetup = SCR_BuildingConfigManagerEntity.ReloadConfig(m_BuildingConfig);
				
				return true;
			}
		#endif
		
		SCR_BuildingSetup m_BuildingSetup = null;
	
		// Uncomment to enable building destruction debugging
		//#define BUILDING_DESTRUCTION_DEBUG
		
		// Uncomment to enable temporary debug shapes
		//#define BUILDING_DEBUG_TEMP
				
		//! Array of bitmasks containing whether each region is destroyed
		protected ref SCR_BitMaskArray m_RegionMask = null;
		
		//! A map holding the damage of regions
		protected ref map<int, float> m_RegionDamage = null;				
			
		ref array<SCR_BuildingRegionEntity> m_RegionEntities = null;
		
		// List to hold queried props list
		static ref array<IEntity> s_aQueryPropsList = new array<IEntity>();
		
		// Static reference to queried building region
		static SCR_BuildingRegionEntity s_QueryBuildingRegion = null;
		
		#ifdef BUILDING_DEBUG_TEMP
			// TODO: Remove once box trace bug is fixed
			static ref array<ref Shape> debugShapes = new array<ref Shape>();
		#endif
		
		//! Whether the building is destroyed
		protected bool m_bDestroyed = false;
		
		//------------------------------------------------------------------------------------------------
		[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
		private void RPC_OnRegionDestroyed(int regionNumber, bool destroyed, bool doStructuralIntegrityCheck, bool doDamageEffects)
		{
			if (destroyed)
				SetRegionDamage01(regionNumber, 1, doStructuralIntegrityCheck, doDamageEffects, false);
			else
				SetRegionDamage01(regionNumber, 0, doStructuralIntegrityCheck, doDamageEffects, false);
		}
		
		//------------------------------------------------------------------------------------------------
		//! Called when Item is initialized from replication stream. Carries the data from Master.
		override bool RplLoad(ScriptBitReader reader)
		{		
			bool undamaged;
			reader.Read(undamaged, 1);
			
			// Building undamaged, so stop reading
			if (undamaged)
			{
				SetDestroyedState(false);
				return true;
			}
			
			bool destroyed;
			reader.Read(destroyed, 1);
			
			// Building destroyed, so stop reading
			if (destroyed)
			{
				SetDestroyedState(true);
				return true;
			}
			
			// Set to undestroyed (damage to be passed)
			SetDestroyedState(false);
			
			// Now read the list of destroyed regions
			int numRegions, regionIndex;
			reader.Read(numRegions, 16);
			for (int i = 0; i < numRegions; i++)
			{
				reader.Read(regionIndex, 16);
				SetRegionDamage01(regionIndex, 1, false, false, false);
			}
			
			return true;
		}
	
		//------------------------------------------------------------------------------------------------
		//! Called when Item is getting replicated from Master to Slave connection. The data will be delivered to Slave using RplInit method.
		override bool RplSave(ScriptBitWriter writer)
		{
			// Building undamaged, so send true and return
			if (GetUndamaged())
			{
				writer.Write(1, 1);
				return true;
			}
			else
				writer.Write(0, 1);
			
			// Building destroyed, so send true and return
			if (GetDestroyed())
			{
				writer.Write(1, 1);
				return true;
			}
			else
				writer.Write(0, 1);
			
			autoptr array<int> destroyedRegions = new array<int>();
			int numRegions = GetRegionsCount();
			for (int i = 0; i < numRegions; i++)
			{
				if (GetRegionDestroyed(i))
					destroyedRegions.Insert(i);
			}
			numRegions = destroyedRegions.Count();
			writer.Write(numRegions, 16);
			for (int i = 0; i < numRegions; i++)
			{
				writer.Write(destroyedRegions.Get(i), 16);
			}
			
			return true;
		}
		
		//------------------------------------------------------------------------------------------------
		override void OnDamage(float damage, EDamageType type, IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, int colliderID, float speed)
		{
			if (!m_BuildingSetup)
				return;
			
			if (GetDestroyed())
				return;
			
			if (RplSession.Mode() == RplMode.Client)
	 			return;
			
			SCR_DestructibleBuildingEntityClass prefabData = SCR_DestructibleBuildingEntityClass.Cast(GetPrefabData());
			if (!prefabData)
				return;
			
			damage = SCR_Global.GetScaledStructuralDamage(damage, type);
			
			// Special case if the building is undamaged
			if (GetUndamaged())
			{
				if (damage < prefabData.GetMinUndamagedThreshold())
					return;
				
				// Force the building into the damaged state so we can find which region we hit and apply the damage
				UpdateBuildingModel(BuildingDamageState.DAMAGED, true);
				
				vector hitPos = outMat[0];
				vector hitDir = outMat[1];
				//vector hitNorm = outMat[2];
				
				autoptr TraceParam param = new TraceParam;
				
				param.Start = hitPos + hitDir * -10;
				param.End = hitPos + hitDir * 10;
				param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
				param.LayerMask = -1;
				
				int hitRegion = -1;
				
				// Trace the region
				float traceDist = GetWorld().TraceMove(param, SCR_Global.FilterCallback_IgnoreNotInList);
				if (traceDist < 1)
				{
					SCR_BuildingRegionEntity regionEnt = SCR_BuildingRegionEntity.Cast(param.TraceEnt);
					if (regionEnt)
						hitRegion = regionEnt.GetRegionNumber();
				}
				
				// Could not trace region, so get via bound box intersection
				if (hitRegion == -1 && m_BuildingSetup)
					hitRegion = m_BuildingSetup.GetBoundingRegionNumber(CoordToLocal(hitPos));
				
				// Still failed to get the rgion, so just apply to base region
				if (hitRegion == -1)
					hitRegion = 0;
				
				// Pass the damage to the region
				AddRegionDamage(hitRegion, damage);
			}
		}
		
		//------------------------------------------------------------------------------------------------
		#ifdef BUILDING_DESTRUCTION_DEBUG
			
			//------------------------------------------------------------------------------------------------
			bool GetCameraViewIntersect()
			{
				if (!m_BuildingSetup)
					return false;
				
				vector camMat[4];
				GetWorld().GetCurrentCamera(camMat);
				
				vector from = CoordToLocal(camMat[3]);
				vector to = VectorToLocal(camMat[2]);
				to = to * 1000 + from;
				float intersect = Math3D.IntersectRayBox(from, to, m_BuildingSetup.m_vBuildingMins, m_BuildingSetup.m_vBuildingMaxs);
				if (intersect >= 0)
					return true;
				else
					return false;
			}
			
			#ifdef WORKBENCH
			
				//------------------------------------------------------------------------------------------------
				override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
				{
					return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
				}
				
				//------------------------------------------------------------------------------------------------
				override void _WB_AfterWorldUpdate(float timeSlice)
				{
					if (m_BuildingConfig != string.Empty)
						m_BuildingSetup = SCR_BuildingConfigManagerEntity.GetBuildingSetupFromConfig(m_BuildingConfig);
					
					UpdateDebug(timeSlice);
				}
			#endif
			
			//------------------------------------------------------------------------------------------------
			override protected void EOnFrame(IEntity owner, float timeSlice) //EntityEvent.FRAME
			{
				UpdateDebug(timeSlice);
			}
			
			//------------------------------------------------------------------------------------------------
			private void UpdateDebug(float timeSlice)
			{
				if (!m_BuildingSetup)
					return;
				
				// If we are not looking at the building with the camera, delete highlight and return
				if (!GetCameraViewIntersect())
					return;
				
				DisplayRegions();
				
				vector pos = GetOrigin();
				vector textMat[4];
				
				GetWorld().GetCurrentCamera(textMat);
				textMat[3] = GetOrigin() + Vector(0, m_BuildingSetup.m_vBuildingMaxs[1] + 2, 0);
				
				if (GetDestroyed())
					CreateSimpleText("DESTROYED", textMat, 0.5, ARGB(0, 255, 0, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, null, 0.7);
				else if (GetDamaged())
				{
					CreateSimpleText("DAMAGED", textMat, 0.5, ARGB(0, 255, 128, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, null, 0.7);
					float dmgPct = 0;
					for (int i = 0; i < GetRegionsCount(); i++)
					{
						dmgPct += GetRegionDamage01(i) / GetRegionsCount();
					}
					textMat[3] = textMat[1] * -0.6 + textMat[3];
					CreateSimpleText(Math.Floor(dmgPct * 100).ToString() + "%", textMat, 0.5, ARGB(0, 255, 128, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, null, 0.7);
				}
				else
					CreateSimpleText("UNDAMAGED", textMat, 0.5, ARGB(0, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, null, 0.7);
			}
			
			//------------------------------------------------------------------------------------------------
			//! Display the regions for debugging etc
			void DisplayRegions()
			{
				if (!m_BuildingSetup)
					return;
				
				vector textMat[4];
				GetWorld().GetCurrentCamera(textMat);
				vector camDir = textMat[2];
				vector camPos = textMat[3];
				
				int highlightRegion = -1;
				float highlightDist = -1;
				
				// First get the region we are highlighting (if at all)
				for (int i = 0; i < GetRegionsCount(); i++)
				{
					SCR_BuildingRegion regionStruct = m_BuildingSetup.GetRegionStruct(i);
					if (!regionStruct)
						continue;
					
					vector regionPos = CoordToParent(regionStruct.GetRegionPos());
					
					vector intersectScreenDiff = regionPos.InvMultiply4(textMat);
					intersectScreenDiff[2] = 0;
					
					float distScale = vector.Distance(camPos, regionPos) * 0.1;
					distScale = Math.Clamp(distScale, 0.5, 5);
					
					float distToCircle = intersectScreenDiff.Length() / distScale;
					if (distToCircle < 0.75 && (distToCircle < highlightDist || highlightDist == -1))
					{
						highlightRegion = i;
						highlightDist = distToCircle;
					}
				}
				
				for (int i = 0; i < GetRegionsCount(); i++)
				{
					SCR_BuildingRegion regionStruct = m_BuildingSetup.GetRegionStruct(i);
					if (!regionStruct)
						continue;
					
					vector regionPos = CoordToParent(regionStruct.GetRegionPos());
					
					float distScale = vector.Distance(camPos, regionPos) * 0.1;
					distScale = Math.Clamp(distScale, 0.5, 3);
					
					textMat[3] = textMat[1] * 0.05 * distScale + regionPos;
					
					// Display debug info about the region
					if (i == highlightRegion)
					{
						int color = ARGBF(1, 1, 1, 1);
						CreateSimpleText("Region " + i.ToString(), textMat, 0.17 * distScale, color, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, null, 0.7, true, ARGB(128, 0, 0, 0));
						textMat[3] = textMat[1] * -0.3 * distScale + textMat[3];
						CreateSimpleText(GetRegionDamage(i).ToString() + "/" + GetRegionMaxDamage(i).ToString(), textMat, 0.17 * distScale, color, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, null, 0.7, true, ARGB(128, 0, 0, 0));
						CreateCircle(regionPos, textMat[2], 0.75 * distScale, ARGBF(0.25, 1, 1, 1), 24, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP);
						CreateCircle(regionPos, textMat[2], ((1 - GetRegionDamage01(i)) * 0.25 + 0.75) * distScale, color, 24, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
						CreateCircle(regionPos, textMat[2], distScale, ARGBF(0.25, 1, 1, 1), 24, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP);
					}
					else
					{
						int color = ARGBF(0.1, 1, 1 - GetRegionDamage01(i), 1 - GetRegionDamage01(i));
						CreateCircle(regionPos, textMat[2], 0.15 * distScale, color, 24, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP);
						CreateCircle(regionPos, textMat[2], 0.01 * distScale, color, 4, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP);
						textMat[3] = textMat[1] * 0.18 * distScale + textMat[3];
						CreateSimpleText(i.ToString(), textMat, 0.17 * distScale, color, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER);
					}
					
					// Display links to supported regions
					for (int s = 0; s < regionStruct.GetConnectionNumOut(); s++)
					{
						SCR_BuildingRegion suppRegionStruct = regionStruct.GetConnectedRegionOut(s);
						if (!suppRegionStruct)
							continue;
						
						int suppRegion = suppRegionStruct.GetRegionIndex();
						vector suppRegionPos = CoordToParent(suppRegionStruct.GetRegionPos());
						
						int arrowsColor = ARGBF(0.02, 1, 1, 1);
						if (i == highlightRegion || suppRegion == highlightRegion)
							arrowsColor = ARGB(255, 128, 128, 64);
						
						int numArrows = Math.Ceil(vector.Distance(suppRegionPos, regionPos) * 5 / distScale);
						if (numArrows == 0)
							numArrows = 1;
						CreateArrowLinkLines(regionPos, suppRegionPos, camDir, 0.08 * distScale, numArrows, arrowsColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER);
					}
				}
			}
		#endif
		
		//------------------------------------------------------------------------------------------------
		//! Called when a region is destroyed
		protected void OnRegionDestroyed(int region, bool doDamageEffects)
		{
			SCR_BuildingRegionEntity regionEnt = null;
			if (m_RegionEntities && m_RegionEntities.Count() > region)
				regionEnt = m_RegionEntities.Get(region);
			
			if (!regionEnt)
				return;
			
			SCR_DebrisBuildingEntity buildingDebris = null;
			if (doDamageEffects)
				buildingDebris = SpawnFallingRegionDebris(region);
			
			DestroyPropsForRegion(regionEnt, buildingDebris);
			
			// Rather than storing component pointer cached on a bulidng, we fetch it now.
			// This is because the number of components on building is too small and also
			// because this is a one-time operation. Therefore, it's not worth wasting memory
			// on every building for something that won't even happen for most of them.
			auto soundComponent = BaseSoundComponent.Cast(FindComponent(BaseSoundComponent));
			if (soundComponent)
			{
				vector camMat[4], regionMat[4];
				GetWorld().GetCurrentCamera(camMat);
				regionEnt.GetTransform(regionMat);
				
				soundComponent.SetSignalValueStr("partSoundID", region);
				soundComponent.SetSignalValueStr("Distance", vector.Distance(camMat[3], SCR_EntityHelper.GetEntityCenterWorld(regionEnt)));
				soundComponent.SoundEventTransform(SCR_SoundEvent.SOUND_BUILDING_CRACK, regionMat)
			}
		}
		
		//------------------------------------------------------------------------------------------------
		//! Called when a region is repaired
		protected void OnRegionRepaired(int region)
		{
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns the world position of a region
		vector GetRegionPosition(int regionNumber)
		{
			if (!m_BuildingSetup)
				return vector.Zero;
			
			SCR_BuildingRegion regionStruct = m_BuildingSetup.GetRegionStruct(regionNumber);
			if (!regionStruct)
				return vector.Zero;
			
			return CoordToParent(regionStruct.GetRegionPos());
		}
		
		//------------------------------------------------------------------------------------------------
		//! Spawns the huge falling debris of a region
		protected SCR_DebrisBuildingEntity SpawnFallingRegionDebris(int region)
		{
			// Get the region
			SCR_BuildingRegion regionStruct = m_BuildingSetup.GetRegionStruct(region);
			if (!regionStruct)
				return null;
			
			vector mat[4];
			GetTransform(mat);
			
			SCR_DebrisBuildingEntity buildingDebris = SCR_DebrisBuildingEntity.SpawnBuildingDebris(mat, m_BuildingSetup, region, GetRegionEntity(region));
			if (!buildingDebris)
				return null;
			
			vector regionDir = buildingDebris.CoordToParent(regionStruct.GetRegionPos()) - buildingDebris.GetOrigin();
			buildingDebris.SetLinearVelocity(buildingDebris.GetLinearVelocity() + regionDir * Math.RandomFloat(0, 0.1));
			buildingDebris.SetAngularVelocity(buildingDebris.GetAngularVelocity() + Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * 2);
			
			return buildingDebris;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Simulates structural integrity, destroying regions that are no longer supported enough (Called from setting damage to regions)
		protected void CheckStructuralIntegrity(int region)
		{
			if (!m_BuildingSetup)
				return;
			
			// Region is not destroyed, so no need for any structural checks
			if (!GetRegionDestroyed(region))
				return;
			
			// Random fall direction and speed (for any potential falling parts)
			//vector fallLinVel = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * 0.5;
			//vector fallAngVel = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * 5;
			
			// Create an array to hold all regions that must be checked
			autoptr array<int> toCheckRegionsList = new array<int>;
			toCheckRegionsList.Insert(region);
			
			while (toCheckRegionsList.Count() > 0)
			{
				// Get the first region in the to check list
				int checkRegion = toCheckRegionsList.Get(0);
				toCheckRegionsList.Remove(0);
				
				// Get the region
				SCR_BuildingRegion regionStruct = m_BuildingSetup.GetRegionStruct(checkRegion);
				if (!regionStruct)
					continue;
				
				// If this is the region that started the structural integrity check, skip creating falling debris etc
				if (checkRegion != region)
				{
					// Go through and get all regions that are supporting the checked region
					int numSupportingRegions = regionStruct.GetConnectionNumIn();
					int numOKSupportingRegions = 0;
					for (int n = 0; n < numSupportingRegions; n++)
					{
						SCR_BuildingRegion regionSupportingStruct = regionStruct.GetConnectedRegionIn(n);
						
						if (GetRegionDestroyed(regionSupportingStruct.GetRegionIndex())) // Already destroyed, skip
							continue;
						
						numOKSupportingRegions++;
					}
					
					// If the amount of regions holding us up above or equal to required, skip
					float heldPct = (float)numOKSupportingRegions / (float)numSupportingRegions;
					if (heldPct >= regionStruct.GetStructuralSupportPercentage())
						continue;
					
					// Not enough holding us up, so destroy the region too and create falling debris
					SetRegionDamage01(checkRegion, 1, false, true, false);
				}
				
				// Add all the regions the checked region is supporting to the list
				for (int i = 0; i < regionStruct.GetConnectionNumOut(); i++)
				{
					SCR_BuildingRegion regionSupportedStruct = regionStruct.GetConnectedRegionOut(i);
					
					int r = regionSupportedStruct.GetRegionIndex();
					if (toCheckRegionsList.Find(r) > -1) // Already in the list, skip
						continue;
					
					if (GetRegionDestroyed(r)) // Already destroyed, skip
						continue;
					
					toCheckRegionsList.Insert(r);
				}
			}
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns whether the building is undamaged
		bool GetUndamaged()
		{
			if (!m_bDestroyed && !m_RegionMask && !m_RegionDamage)
				return true;
			else
				return false;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns whether the building is destroyed
		bool GetDestroyed()
		{
			return m_bDestroyed;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns whether the building is damaged
		bool GetDamaged()
		{
			if (GetUndamaged())
				return false;
			else if (GetDestroyed())
				return false;
			else
				return true;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns whether the input region is in the region destroyed bitmask array
		bool GetRegionDestroyed(int regionNumber)
		{
			if (GetDestroyed())
				return true;
			
			if (!m_RegionMask)
				return false;
			
			return m_RegionMask.GetBit(regionNumber);
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns the number of regions
		int GetRegionsCount()
		{
			if (!m_BuildingSetup)
				return 0;
			
			return m_BuildingSetup.GetRegionNum();
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns the maximum damage the region can take
		float GetRegionMaxDamage(int regionNumber)
		{
			if (!m_BuildingSetup)
				return 0;
			
			if (regionNumber < 0) // Out of limits, return 0
				return 0;
			
			if (regionNumber >= GetRegionsCount()) // Above region count, return 0
				return 0;
			
			SCR_BuildingRegion regionStruct = m_BuildingSetup.GetRegionStruct(regionNumber);
			if (!regionStruct)
				return 0;
			
			return regionStruct.GetMaxHealth();
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns the damage of the input numbered region
		float GetRegionDamage(int regionNumber)
		{
			// Building undamaged, so neither is region
			if (GetUndamaged())
				return 0;
			
			// Building completely destroyed, so region is too
			if (GetDestroyed())
				return GetRegionMaxDamage(regionNumber);
			
			// Region is in the bit array, so is destroyed
			if (m_RegionMask && m_RegionMask.GetBit(regionNumber))
				return GetRegionMaxDamage(regionNumber);
			
			// No stored damage values, so this region is undamaged
			if (!m_RegionDamage)
				return 0;
			
			// Return the stored damage value
			return m_RegionDamage.Get(regionNumber);
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns the damage (in percent 0 - 1) of the input numbered region
		float GetRegionDamage01(int regionNumber)
		{
			if (regionNumber >= GetRegionsCount())
				return 0;
			
			float maxDamage = GetRegionMaxDamage(regionNumber);
			if (maxDamage == 0)
				return 0;
			
			return GetRegionDamage(regionNumber) / maxDamage;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Sets the damage value of the input region
		protected void SetRegionDamage(int regionNumber, float damage, bool doStructuralIntegrityCheck = true, bool doDamageEffects = true, bool sendRPC = true)
		{
			if (!m_BuildingSetup)
				return;
			
			if (regionNumber >= GetRegionsCount())
				return;
			
			float maxDamage = GetRegionMaxDamage(regionNumber);
			
			// If we are completely destroyed and wanting to set more damage, skip
			if (GetDestroyed() && damage >= maxDamage)
				return;
			
			// If we are completely undamaged and wanting to set no damage, skip
			if (GetUndamaged() && damage <= 0)
				return;
			
			// Get our state as 3 state int
			int state = 0;
			if (GetDamaged())
				state = 1;
			else if (GetDestroyed())
				state = 2;
			
			damage = Math.Clamp(damage, 0, maxDamage);
			float curDam = GetRegionDamage(regionNumber);
			
			// Setting same damage, skip
			if (curDam == damage)
				return;
			
			if (sendRPC && !IsProxy())
			{
				if (damage >= maxDamage)
					Rpc(RPC_OnRegionDestroyed, regionNumber, true, doStructuralIntegrityCheck, doDamageEffects);
				else if (damage <= 0)
					Rpc(RPC_OnRegionDestroyed, regionNumber, false, doStructuralIntegrityCheck, doDamageEffects);
			}
			
			// Building destroyed but setting damage below max, so 'undestroy' and set all zones to destroyed (this zone will be adjusted below)
			if (GetDestroyed() && damage < maxDamage)
			{
				SetDestroyedState(false, false);
				SetAllZonesDestroyed();
			}
			
			// Set the zone to undamaged or destroyed
			if (damage == 0 || damage == maxDamage)
			{
				if (damage == 0) // Undamaged, remove region from mask
				{
					// Remove region from the bitarray
					if (m_RegionMask)
					{
						m_RegionMask.SetBit(regionNumber, false);
						if (m_RegionMask.GetEmpty())
							delete m_RegionMask;
					}
					
					OnRegionRepaired(regionNumber);
				}
				else if (damage == maxDamage) // Destroyed, add region to mask
				{
					// Add region to the bitarray
					if (!m_RegionMask)
						m_RegionMask = new SCR_BitMaskArray(GetRegionsCount());
					m_RegionMask.SetBit(regionNumber, true);
					
					OnRegionDestroyed(regionNumber, doDamageEffects);
				}
				
				// Find our region in the map and remove it if found
				if (m_RegionDamage)
				{
					for (int i = 0; i < m_RegionDamage.Count(); i++)
					{
						if (m_RegionDamage.GetKey(i) == regionNumber)
						{
							m_RegionDamage.RemoveElement(i);
							break;
						}
					}
					
					// If no more stored damage values left, delete the map
					if (m_RegionDamage.Count() == 0)
						delete m_RegionDamage;
				}
			}
			else // Damage is non-zero and not maximum, so set it
			{
				// Remove region from the bitarray
				if (m_RegionMask)
				{
					m_RegionMask.SetBit(regionNumber, false);
					if (m_RegionMask.GetEmpty())
						delete m_RegionMask;
				}
				
				// Damage value map does not exist, so create it
				if (!m_RegionDamage)
					m_RegionDamage = new map<int, float>;
				
				if (m_RegionDamage.Contains(regionNumber)) // Region damage already stored, so change it
					m_RegionDamage.Set(regionNumber, damage);
				else // Add region damage to the map
					m_RegionDamage.Insert(regionNumber, damage);
			}
			
			// All regions have damage set, so go to destroyed state
			if (GetAllZonesAtMaxDamage())
				SetDestroyedState(true, false);
			
			// Get our new state as 3 stage int
			int newState = 0;
			if (GetDamaged())
				newState = 1;
			else if (GetDestroyed())
				newState = 2;
			
			// Our state has changed, so update the model to reflect this
			if (newState != state)
				UpdateBuildingModel();
			
			// Update the region display of the model based on damage and check structural integrity
			if (GetDamaged())
			{
				UpdateBuildingRegions();
				if (doStructuralIntegrityCheck)
					CheckStructuralIntegrity(regionNumber);
			}
		}
		
		//------------------------------------------------------------------------------------------------
		//! Sets the damage (in %) value of the input region
		void SetRegionDamage01(int regionNumber, float damagePct, bool doStructuralIntegrityCheck = true, bool doDamageEffects = true, bool sendRPC = true)
		{
			SetRegionDamage(regionNumber, GetRegionMaxDamage(regionNumber) * damagePct, doStructuralIntegrityCheck, doDamageEffects, sendRPC);
		}
		
		//------------------------------------------------------------------------------------------------
		//! Adds damage to the input region (may be negative for repair)
		void AddRegionDamage(int regionNumber, float damage, bool doStructuralIntegrityCheck = true, bool doDamageEffects = true, bool sendRPC = true)
		{
			SetRegionDamage(regionNumber, GetRegionDamage(regionNumber) + damage, doStructuralIntegrityCheck, doDamageEffects, sendRPC);
		}
		
		//------------------------------------------------------------------------------------------------
		//! Sets the main destruction state of the building (undamaged/damaged vs destroyed)
		void SetDestroyedState(bool destroyed, bool updateModel = true)
		{
			// Delete any stored damage values
			if (m_RegionDamage)
				delete m_RegionDamage;
			
			// Delete any stored region masks
			if (m_RegionMask)
				delete m_RegionMask;
			
			m_bDestroyed = destroyed;
			
			if (updateModel)
				UpdateBuildingModel();
		}
		
		//------------------------------------------------------------------------------------------------
		//! Get a region entity
		SCR_BuildingRegionEntity GetRegionEntity(int regionNumber)
		{
			if (!m_RegionEntities)
				return null;
			
			if (m_RegionEntities.Count() > regionNumber)
				return m_RegionEntities.Get(regionNumber);
			
			return null;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Spawn a region entity
		private SCR_BuildingRegionEntity SpawnRegionEntity(int regionNumber)
		{
			if (!m_RegionEntities)
				m_RegionEntities = new array<SCR_BuildingRegionEntity>;
			
			SCR_BuildingRegionEntity regionEnt = null;
			if (m_RegionEntities.Count() > regionNumber)
				regionEnt = m_RegionEntities.Get(regionNumber);
			
			if (regionEnt)
				return regionEnt;
			
			vector buildingMat[4];
			GetTransform(buildingMat);
			
			//regionEnt = SCR_BuildingRegionEntity.Cast(GetGame().SpawnEntityPrefab(Resource.Load("{944DD356E5175140}Prefabs/Test/BuildingRegion.et")));
		
			if (!regionEnt)
				return null;
		
			regionEnt.SetBuildingAndIndex(this, regionNumber);
			regionEnt.LoadRegionModel();
			
			AddChild(regionEnt, -1, EAddChildFlags.NONE);
			regionEnt.SetTransform(buildingMat);
			
			m_RegionEntities.InsertAt(regionEnt, regionNumber);
			
			return regionEnt;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns whether the input prop is of a type that allows destruction with the building
		bool GetPropDestructible(notnull IEntity prop)
		{
			GenericEntity genEnt = GenericEntity.Cast(prop);
			if (!genEnt)
				return false;
			
			// If we have a destructible component then we may be destroyed
			if (genEnt.FindComponent(SCR_DestructionDamageManagerComponent))
				return true;
			
			// If we have a ladder component then we may be destroyed
			if (genEnt.FindComponent(LadderComponent))
				return true;
			
			return false;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Checks for any door/window/etc intersection and fills the input list with them
		//! Always returns true as the function is used for filtering bounding box queries and returning false would end the check
		bool QueryRegionProp(IEntity prop, bool propIsChild = false, float traceUp = 0.1, float traceDown = 0.1)
		{
			if (!prop)
				return true;
			
			// Is a building
			if (SCR_DestructibleBuildingEntity.Cast(prop))
				return true;
			
			// Is a building region
			if (SCR_BuildingRegionEntity.Cast(prop))
				return true;
			
			// Prop has a parent that is not the building, so ignore (as the children of the building are destroyed)
			if (prop.GetParent() && prop.GetParent() != this)
				return true;
			
			// Prop is not a child, so limit what we can destroy to certain types
			if (!propIsChild && !GetPropDestructible(prop))
				return true;
			
			// Prop is already in the list
			if (s_aQueryPropsList.Find(prop) >= 0)
				return true;
			
			vector mins, maxs, mat[4];
			prop.GetWorldTransform(mat);
			/*prop.GetWorldBounds(mins, maxs);
			
			vector center = (maxs - mins) * 0.5 + mins;
			vector size;
			for (int a = 0; a < 3; a++)
			{
				size[a] = Math.AbsFloat(maxs[a] - mins[a]);
			}
			autoptr TraceParam param = new TraceParam;
			param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
			param.Start = center;
			param.End = center - vector.Up * (size[1]) + mat[2] * 0.1;
			*/
			prop.GetBounds(mins, maxs);
			for (int a = 0; a < 3; a++)
			{
				float min = mins[a] + 0.2;
				float max = maxs[a] - 0.2;
				
				if (min >= 0)
					min = -0.05;
				if (max <= 0)
					max = 0.05;
				
				mins[a] = min;
				maxs[a] = max;
			}
			
			autoptr TraceOBB param = new TraceOBB;
			param.Mat[0] = mat[0];
			param.Mat[1] = mat[1];
			param.Mat[2] = mat[2];
			param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
			param.LayerMask = EPhysicsLayerDefs.Default;
			param.Mins = mins;
			param.Maxs = maxs;
			// TODO: REMOVE THIS TEMPORARY OVERRIDE IN SIZE!!!!! Once box tracing is fixed.
			param.Mins = -vector.One;
			param.Maxs = vector.One;
			
			param.Start = mat[3] + vector.Up * 1;
			param.End = mat[3] - vector.Up * 1;
			
			SCR_Global.g_TraceFilterEnt = s_QueryBuildingRegion;
			float traced = GetWorld().TraceMove(param, SCR_Global.FilterCallback_IgnoreAllButEntity);
			
			// The prop is a child and is floating in mid-air somewhere or traced the building region we are interested in
			//if ((propIsChild && traced == 1) || param.TraceEnt == s_QueryBuildingRegion)
			if (traced < 1 && param.TraceEnt == s_QueryBuildingRegion)
				s_aQueryPropsList.Insert(prop);
			
			#ifdef BUILDING_DEBUG_TEMP
				Shape shp;
				
				int color = ARGB(255, 255, 0, 0);
				if (traced < 1)
					color = ARGB(255, 0, 255, 0);
				
				int color2 = ARGB(255, 0, 0, 255);
				
				vector mins2 = param.Mins - vector.One * 0.025;
				vector maxs2 = param.Maxs + vector.One * 0.025;
				
				// Start trace - target
				shp = Shape.Create(ShapeType.BBOX, color2, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, param.Mins, param.Maxs);
				mat[3] = param.Start;
				shp.SetMatrix(mat);
				debugShapes.Insert(shp);
				
				// End trace - target
				shp = Shape.Create(ShapeType.BBOX, color2, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, param.Mins, param.Maxs);
				mat[3] = param.End;
				shp.SetMatrix(mat);
				debugShapes.Insert(shp);
				
				// Trace - target
				shp = Shape.Create(ShapeType.LINE, color2, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER, param.Start, param.End);
				debugShapes.Insert(shp);
				
				// Start trace - result
				shp = Shape.Create(ShapeType.BBOX, color, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, mins2, maxs2);
				mat[3] = param.Start;
				shp.SetMatrix(mat);
				debugShapes.Insert(shp);
				
				// End trace - result
				shp = Shape.Create(ShapeType.BBOX, color, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, mins2, maxs2);
				mat[3] = (param.End - param.Start) * traced + param.Start;
				shp.SetMatrix(mat);
				debugShapes.Insert(shp);
				
				// Trace - result
				shp = Shape.Create(ShapeType.LINE, color, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER, param.Start + vector.One * 0.025, (param.End - param.Start) * traced + param.Start + vector.One * 0.025);
				debugShapes.Insert(shp);
				
				CreateSimpleText("REGION: " + s_QueryBuildingRegion.GetRegionNumber().ToString(), mat, 0.2, ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER, debugShapes, 0.7);
				
				PrintFormat("REGION: %1", s_QueryBuildingRegion.GetRegionNumber());
			#endif
			
			return true;
		}
		
		bool QueryRegionPropCallback(IEntity e)
		{
			return QueryRegionProp(e);
		}
		
		//------------------------------------------------------------------------------------------------
		//! Checks for any door/window/etc intersection and fills the input list with them
		void GetPropsForRegion(SCR_BuildingRegionEntity regionEnt)
		{
			#ifdef BUILDING_DEBUG_TEMP
				debugShapes.Clear();
			#endif
			
			s_aQueryPropsList.Clear();
			s_QueryBuildingRegion = regionEnt;
			
			if (!s_QueryBuildingRegion)
				return;
			
			// First go through direct children (which have correctly set hierarchies)
			IEntity child = GetChildren();
			while (child)
			{
				IEntity prop = child;
				child = child.GetSibling();
				
				QueryRegionProp(prop, true);
			}
			
			// Now go through objects within the bounding box of the region			
			vector regionMins, regionMaxs, regionMat[4];
			s_QueryBuildingRegion.GetWorldBounds(regionMins, regionMaxs);
			
			GetWorld().QueryEntitiesByAABB(regionMins, regionMaxs, QueryRegionPropCallback);
		}
		
		//------------------------------------------------------------------------------------------------
		// Copies the input prop object (and its children) to the building debris
		protected void CopyPropToBuildingDebris(SCR_DebrisBuildingEntity buildingDebris, IEntity prop, vector debrisMat[4])
		{
			if (!buildingDebris)
				return;
			
			if (!prop)
				return;
			
			// Recursively call through all children
			IEntity propChild = prop.GetChildren();
			while (propChild)
			{
				CopyPropToBuildingDebris(buildingDebris, propChild, debrisMat);
				propChild = propChild.GetSibling();
			}
			
			// Get the prop's model
			VObject obj = prop.GetVObject();
			if (!obj)
				return;
			
			// Get its relative transform to the building debris
			vector propMat[4], localPropMat[4];
			prop.GetWorldTransform(propMat);
			Math3D.MatrixInvMultiply4(debrisMat, propMat, localPropMat);
			
			// Create a copy of the prop (visual only) and attach it to the building debris
			GenericEntity propCopy = GenericEntity.Cast(GetGame().SpawnEntity(GenericEntity));
			propCopy.SetObject(obj, string.Empty);
			propCopy.SetTransform(localPropMat);
			buildingDebris.AddChild(propCopy, -1, EAddChildFlags.AUTO_TRANSFORM);
		}
		
		//------------------------------------------------------------------------------------------------
		// Checks for any door/window/etc intersection and destroys the object as well
		protected void DestroyPropsForRegion(SCR_BuildingRegionEntity regionEnt, SCR_DebrisBuildingEntity buildingDebris)
		{
			if (!regionEnt)
				return;
			
			GetPropsForRegion(regionEnt);
			
			vector debrisMat[4];
			if (buildingDebris)
				buildingDebris.GetWorldTransform(debrisMat);
			
			int numProps = s_aQueryPropsList.Count();
			for (int i = numProps - 1; i >= 0; i--)
			{
				IEntity prop = s_aQueryPropsList.Get(i);
				if (!prop)
					continue;
				
				// Destroy any destructibles
				GenericEntity genProp = GenericEntity.Cast(prop);
				if (genProp)
				{
					SCR_DestructionDamageManagerComponent destructible = SCR_DestructionDamageManagerComponent.Cast(genProp.FindComponent(SCR_DestructionDamageManagerComponent));
					if (destructible)
					{
						vector mat[3];
						Math3D.MatrixIdentity3(mat);
						destructible.HandleDamage(EDamageType.EXPLOSIVE, destructible.GetMaxHealth(), mat, destructible.GetOwner(), null, this, null, -1, -1);
					}
				}
				
				if (buildingDebris)
					CopyPropToBuildingDebris(buildingDebris, prop, debrisMat);
				
				// TODO: Hide prop and disable physics instead of deleting? To allow repair
				SCR_EntityHelper.DeleteEntityAndChildren(prop);
				//prop.ClearFlags(EntityFlags.VISIBLE | EntityFlags.ACTIVE, true);
				//Physics phys = prop.GetPhysics();
				//if (phys)
					// disable physics here!
			}
		}
		
		//------------------------------------------------------------------------------------------------
		// Delete a region entity
		private void DeleteRegionEntity(int regionNumber)
		{
			if (!m_RegionEntities || m_RegionEntities.Count() <= regionNumber)
				return;
			
			SCR_BuildingRegionEntity regionEnt = m_RegionEntities.Get(regionNumber);
			if (regionEnt)
				delete regionEnt;
			else
				return;
			
			// Check whether there are any regions left in the array
			for (int i = 0; i < m_RegionEntities.Count(); i++)
			{
				regionEnt = m_RegionEntities.Get(i);
				if (regionEnt) // Found a region, so return
					return;
			}
			
			// No region entities left, so delete the array
			delete m_RegionEntities;
			m_RegionEntities = null;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Deletes all region entities
		private void DeleteAllRegionEntities()
		{
			if (!m_RegionEntities)
				return;
			
			for (int i = m_RegionEntities.Count() - 1; i > -1; i--)
			{
				SCR_BuildingRegionEntity regionEnt = m_RegionEntities.Get(i);
				m_RegionEntities.Remove(i);
				delete regionEnt;
			}
		
			delete m_RegionEntities;
			m_RegionEntities = null;
		}
		
		//------------------------------------------------------------------------------------------------
		//! Spawns all region entities
		private void SpawnAllRegionEntities(bool addRegionsToTraceIgnoreList = false)
		{
			if (addRegionsToTraceIgnoreList)
				SCR_Global.g_TraceFilterList.Clear();
			
			DeleteAllRegionEntities();
			
			for (int i = 0; i < GetRegionsCount(); i++)
			{
				SCR_BuildingRegionEntity region = SpawnRegionEntity(i);
				if (addRegionsToTraceIgnoreList && region)
					SCR_Global.g_TraceFilterList.Insert(region);
			}
		}
		
		//------------------------------------------------------------------------------------------------
		//! Updates the building's model to reflect damaged/destroyed status
		private void UpdateBuildingModel(int forceState = -1, bool addRegionsToTraceIgnoreList = false)
		{
			if (!m_BuildingSetup)
				return;
			
			int buildingState = BuildingDamageState.UNDAMAGED;
			if (GetDestroyed())
				buildingState = BuildingDamageState.DESTROYED;
			else
				buildingState = BuildingDamageState.DAMAGED;
			
			// We are forcing a new state, so use it
			if (forceState != -1)
				buildingState = forceState;
			
			ResourceName assetPath = m_BuildingSetup.m_ModelUndamaged;
			if (buildingState == BuildingDamageState.DESTROYED)
				assetPath = m_BuildingSetup.m_ModelDestroyed;
			else if (buildingState == BuildingDamageState.DAMAGED)
				assetPath = "";
			
			// Delete any attachments to the building in destroyed state
			if (buildingState == BuildingDamageState.DESTROYED)
			{
				IEntity child = GetChildren();
				while (child)
				{
					//delete child;
					//child = GetChildren();
					child = child.GetSibling();
				}
			}
			else
			{
				//Print("=================");
				IEntity child = GetChildren();
				while (child)
				{
					//delete child;
					//child = GetChildren();
					if (child.GetFlags() & EntityFlags.PROXY)
					{
						child.ClearFlags(EntityFlags.PROXY, false);
						child.Update();
					}
					//if (child.GetFlags() & EntityFlags.PROXY)
					//	Print(child.GetVObject().ToString());
					child = child.GetSibling();
				}
			}
			
			// Updates physics and visual model
			Physics phys = GetPhysics();
			if (buildingState == BuildingDamageState.UNDAMAGED || buildingState == BuildingDamageState.DESTROYED)
			{
				if (phys)
					phys.Destroy();
				if (assetPath == "")
				{
					SetObject(null, "");
					ClearFlags(EntityFlags.VISIBLE, false);
				}
				else
				{
					Resource resource = Resource.Load(assetPath);
					VObject asset = resource.GetResource().ToVObject();
					SetObject(asset, "");
					SetFlags(EntityFlags.VISIBLE, false);
					
					Physics.CreateStatic(this, -1);
				}
			}
			else
			{
				ClearFlags(EntityFlags.VISIBLE, false);
				if (phys)
					phys.Destroy();
			}
			
			// Create the region entities if we are damaged
			if (buildingState == BuildingDamageState.DAMAGED)
				SpawnAllRegionEntities(addRegionsToTraceIgnoreList);
			else
				DeleteAllRegionEntities();
		}
		
		//------------------------------------------------------------------------------------------------
		//! Updates the building's model regions to reflect damage
		private void UpdateBuildingRegions()
		{
			if (!m_BuildingSetup)
				return;
			
			int regionNum = GetRegionsCount();
			for (int i = 0; i < regionNum; i++)
			{
				if (GetRegionDestroyed(i))
				{
					DeleteRegionEntity(i);
					continue;
				}
				
				SCR_BuildingRegionEntity regionEnt = SpawnRegionEntity(i);
			
				if (!regionEnt)
					return;
				
				// Create/destroy each region intersection debris
				for (int n = 0; n < regionNum; n++)
				{
					if (n == i) // Ignore the same region
						continue;
					
					// Other region is destroyed, so show the intersection debris
					if (GetRegionDestroyed(n))
						regionEnt.CreateIntersectDebris(n);
					else
						regionEnt.DeleteIntersectDebris(n);
				}
			}
		}
		
		//------------------------------------------------------------------------------------------------
		//! Sets all zones to destroyed without setting the building's destroyed state (called from damage functions)
		private void SetAllZonesDestroyed()
		{
			// Delete any stored damage values
			if (m_RegionDamage)
				delete m_RegionDamage;
			
			for (int i = 0; i < GetRegionsCount(); i++)
			{
				// Add region to the bitarray
				if (!m_RegionMask)
					m_RegionMask = new SCR_BitMaskArray(GetRegionsCount());
				m_RegionMask.SetBit(i, true);
			}
		}
		
		//------------------------------------------------------------------------------------------------
		//! Returns whether all zones have full damage set (called from damage functions)
		bool GetAllZonesAtMaxDamage()
		{
			// No bitmask array, so no destroyed regions
			if (!m_RegionMask)
				return false;
			
			for (int i = 0; i < GetRegionsCount(); i++)
			{
				// If the region is in the bitmask array, go to the next as it is destroyed
				if (m_RegionMask.GetBit(i))
					continue;
				
				// Not all zones are destroyed
				return false;
			}
			
			return true;
		}
		
		//------------------------------------------------------------------------------------------------
		protected override void EOnInit(IEntity owner)
		{
			// Insert building and children into replication
			RplComponent rplComponent = RplComponent.Cast(FindComponent(RplComponent));
			if (rplComponent)
				rplComponent.InsertToReplication();
		}
		
		//------------------------------------------------------------------------------------------------
		void SCR_DestructibleBuildingEntity(IEntitySource src, IEntity parent)
		{
			if (!GetGame().InPlayMode())
				return;
		
			
			//SetFlags(EntityFlags.ACTIVE, false);
			#ifdef BUILDING_DESTRUCTION_DEBUG
				SetEventMask(EntityEvent.FRAME);
			#endif
			SetEventMask(EntityEvent.INIT);
			
			if (m_BuildingConfig != string.Empty)
				m_BuildingSetup = SCR_BuildingConfigManagerEntity.GetBuildingSetupFromConfig(m_BuildingConfig);
		}
		
		//------------------------------------------------------------------------------------------------
		void ~SCR_DestructibleBuildingEntity()
		{
			if (!GetGame().InPlayMode())
				return;
			
			DeleteAllRegionEntities();
		}
	#endif
};