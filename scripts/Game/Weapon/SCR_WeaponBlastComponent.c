class SCR_WeaponBlastComponentClass : ScriptComponentClass
{
	[Attribute(desc: "List effects that will be applied")]
	protected ref array<ref SCR_WeaponBlastEffect> m_aBlastEffects;

	[Attribute("3.0", desc: "How far will the blast propagate. Also limits propagation sideways.", params: "0.1 inf")]
	protected float m_fBlastLength;

	[Attribute("30", desc: "How much angle deviation from foward vector of the blast is allowed", params: "0.1 89.9")]
	protected float m_fBlastConeAngle;

	[Attribute("1", desc: "Determines if game should check if blast will hit something that may cause it to ricochet")]
	protected bool m_bCanBlastRicochet;

	[Attribute("10", desc: "Determines how strongly ricochet angle of the blast is flattend upon impacting the surface", params: "1 10000")]
	protected float m_fDeflectionFlatteningStrength;

	[Attribute("2", desc: "Final amount of damage dealt to destructible entities will be multiplied by this factor", params: "0 inf")]
	protected float m_fDestructibleDamageMultiplier;

	[Attribute("0", desc: "Determines if blast shouldnt damage AI characters", category: "AI")]
	protected bool m_bIgnoreAIUnits;

	[Attribute("1", desc: "Determines if blast that was caused by AI character can damage other characters.", category: "AI")]
	protected bool m_bAICanBlastCharacters;

	[Attribute("1", desc: "Determines if blast that was caused by AI character can damage things like f.e. wooden fences", category: "AI")]
	protected bool m_bAICanBlastDestructible;

	[Attribute("0", desc: "Determines if blast that was caused by AI character can damage it when it bouces from f.e. a wall.\nIf m_bAICanBlastCharacters or m_bIgnoreAIUnits is true then this will be considered true", category: "AI")]
	protected bool m_bAICanDamageItself;

	//------------------------------------------------------------------------------------------------
	float GetBlastLength()
	{
		return m_fBlastLength;
	}

	//------------------------------------------------------------------------------------------------
	float GetBlastConeAngle()
	{
		return m_fBlastConeAngle;
	}

	//------------------------------------------------------------------------------------------------
	bool CanBlastRicochet()
	{
		return m_bCanBlastRicochet;
	}

	//------------------------------------------------------------------------------------------------
	float GetDeflectionFlatteningStrength()
	{
		return m_fDeflectionFlatteningStrength;
	}

	//------------------------------------------------------------------------------------------------
	float GetDestructibleDamageMultiplier()
	{
		return m_fDestructibleDamageMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	bool ShouldIgnoreAIUnits()
	{
		return m_bIgnoreAIUnits;
	}

	bool CanAIBlastCharacters()
	{
		return m_bAICanBlastCharacters;
	}

	//------------------------------------------------------------------------------------------------
	bool CanAIBlastDestructible()
	{
		return m_bAICanBlastDestructible;
	}

	//------------------------------------------------------------------------------------------------
	bool CanAIDamageItself()
	{
		return m_bAICanDamageItself;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] outBlastEffects array to which blast effects are going to be added
	//! \return
	int GetBlastEffects(out notnull array<SCR_WeaponBlastEffect> outBlastEffects)
	{
		foreach (SCR_WeaponBlastEffect effect : m_aBlastEffects)
		{
			if (!effect)
				continue;

			outBlastEffects.Insert(effect);
		}

		return outBlastEffects.Count();
	}
}

class SCR_WeaponBlastComponent : ScriptComponent
{
	[Attribute(desc: "Position and direction in which blast will propagate\nDirection is determined by forward vector (blue arrow)")]
	protected ref PointInfo m_BlastOrigin;

	protected ref TraceSphere m_Trace;
	protected ref Instigator m_Instigator;
	protected ref array<IEntity> m_aFoundCharacters;
	protected IEntity m_VerifiedEntity;
	protected bool m_bIsAiCharacter;

	static protected const EQueryEntitiesFlags QUERY_FLAGS = EQueryEntitiesFlags.DYNAMIC;
	static protected const int MAX_BLAST_MEMBERS = 32;
#ifdef ENABLE_DIAG
	// Debug
	static protected ref SCR_DebugShapeManager m_DebugShapeMgr;
	protected const float DEBUG_SPHERE_RADIUS = 0.1;
	protected const ShapeFlags DEBUG_SHAPE_FLAGS = ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP;
	protected const int DEBUG_CONE_SUBDIV = 8;
	protected const int dbgPanelXSize = 2;
	protected const int dbgPanelYSize = 20;
	protected ref array<ref TStringArray> m_aDebugData;

	//------------------------------------------------------------------------------------------------
	//! Method used for adding debug lists, and entries to already existing lists
	//! \param[in] label of the list
	//! \param[in] entry which is going to be added to such list
	protected void AddDebugInfo(string label, string entry)
	{
		foreach (TStringArray arr : m_aDebugData)
		{
			if (arr[0] != label)
				continue;

			arr.Insert(entry);
			return;
		}

		m_aDebugData.Insert({label, entry});
	}

	//------------------------------------------------------------------------------------------------
	//! Method that returns just the name of the prefab from which provided entity was created
	//! \param[in] ent
	protected string GetPrefabName(IEntity ent)
	{
		string prefab = SCR_ResourceNameUtils.GetPrefabName(ent);
		return FilePath.StripPath(prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Method that adds a colored line entry to the debug ui
	//! \param[in] entryName which is unique to other entries in this window
	//! \param[in] xSize of the color indicator
	//! \param[in] ySize of the color indicator
	//! \param[in] color of the indicator
	//! \param[in] text
	protected void AddDebugLegendEntry(string entryName, int xSize, int ySize, int color, string text)
	{
		DbgUI.Panel(entryName, xSize, ySize, color);
		DbgUI.SameLine();
		DbgUI.Text(text);
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! Method used for calculating the outcome direction from impact
	protected void CalculateRicochetDirection(vector hitPosDirNorm[3], out vector ricochetTransform[4], float ricochetFlatteningStrength = 1)
	{
		if (!ricochetTransform)
			ricochetTransform = {};

		vector dirInverted = -hitPosDirNorm[1];

		// Matrix of surface normal
		vector normalMatrix[3];
		Math3D.MatrixFromUpVec(hitPosDirNorm[2], normalMatrix);

		// Put direction vector into normal space
		vector dirInvertedNormalSpace = dirInverted.InvMultiply3(normalMatrix);

		// Create rotation matrix to deflect incoming vector
		vector rotationMatrix[3];
		Math3D.AnglesToMatrix({180, 0, 0}, rotationMatrix);

		// Rotate normal matrix
		vector rotatedNormalMatrix[3];
		Math3D.MatrixMultiply3(normalMatrix, rotationMatrix, rotatedNormalMatrix);

		// Return direction vector back to world space using rotated normal matrix
		vector outDirectionDeflected = dirInvertedNormalSpace.Multiply3(rotatedNormalMatrix);

		// Create vector perpendicular to surface normal and combine with deflected vector scaled by m_fDeflectionFlatteningStrength
		vector outDirectionPerpendicular = (dirInverted * hitPosDirNorm[2]) * hitPosDirNorm[2];
		vector outDirection = (outDirectionDeflected + outDirectionPerpendicular * ricochetFlatteningStrength).Normalized();
		Math3D.DirectionAndUpMatrix(outDirection, outDirection.Perpend(), ricochetTransform);
		ricochetTransform[3] = hitPosDirNorm[0] + outDirection * 0.1;//move away from that surface to not block the trace or start the trace inside of it
	}

	//------------------------------------------------------------------------------------------------
	//! Method used for filtering out entities that are not meant to be gathered for further investigation of the blast mechanic
	protected bool QueryFilter(IEntity ent)
	{
		if (ent == GetOwner())
			return false;

		if (!ChimeraCharacter.Cast(ent))
			return false;

		if (ent == m_VerifiedEntity)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used for adding found characters to the list of blsted characters
	protected bool QueryAddEntity(IEntity ent)
	{
		if (!ent)
			return true;

		if (m_aFoundCharacters.Contains(ent))
			return true;

		m_aFoundCharacters.Insert(ent);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Method used to filter out objects that shouldnt block the blast like particles or worn equipment
	protected bool TraceFilter(notnull IEntity ent)
	{
		if (!ent.GetPrefabData())
			return false;

		if (ent == ent.GetRootParent())
			return true;

		IEntity parent = ent.GetParent();
		while (parent)
		{
			if (parent == GetOwner())
				return false;

			parent = parent.GetParent();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to determine which items that were found by the query should stop the trace
	protected bool ObstructionTraceFilter(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		if (m_VerifiedEntity == e)
			return true; // our target should be included

		IEntity parent = e.GetParent();
		while (parent)
		{//ignore character items
			if (ChimeraCharacter.Cast(parent))
				return false;

			parent = parent.GetParent();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method that is meant to be set before weapon will fire in order to control who is blamed for the blast
	void OverrideInstigator(IEntity newInstigatorEntity)
	{
		m_Instigator = Instigator.CreateInstigator(newInstigatorEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Method for finding the the point that is the closest to the center line of the cone
	protected vector FindClosestCharacterPoint(vector startingPos, vector direction, notnull ChimeraCharacter character, out float distance)
	{
		const vector charAimPosition = character.AimingPosition();//center of mass of the character
		float distanceAim = vector.Distance(startingPos, charAimPosition);
		vector charDirection = vector.Direction(startingPos, charAimPosition).Normalized();
		const float dotAim = vector.Dot(charDirection, direction);
		const vector nearestPositionAim = startingPos + direction * dotAim * distanceAim;
		distanceAim = vector.Distance(nearestPositionAim, charAimPosition);

		//Compare this with EYE position
		vector charPosition = character.EyePosition();
		distance = vector.Distance(startingPos, charPosition);
		charDirection = vector.Direction(startingPos, charPosition).Normalized();
		float dot = vector.Dot(charDirection, direction);
		vector nearestPosition = startingPos + direction * dot * distance;
		if (distanceAim > vector.Distance(nearestPosition, charPosition) && dot >= 0 && dot > dotAim)
		{
#ifdef ENABLE_DIAG
			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) == 3)
			{//GREEN SPHERE == Character eye position being the closest, usable position
				m_DebugShapeMgr.AddSphere(charPosition, DEBUG_SPHERE_RADIUS * 0.3, Color.SPRING_GREEN, DEBUG_SHAPE_FLAGS);
				m_DebugShapeMgr.AddLine(charPosition, nearestPosition, Color.SPRING_GREEN);
				m_DebugShapeMgr.AddSphere(nearestPosition, DEBUG_SPHERE_RADIUS * 0.3, Color.DARK_GREEN, DEBUG_SHAPE_FLAGS);

				AddDebugInfo("Distance:", "Eyes - " + GetPrefabName(character) + " at " + charPosition + " distance = " + distance.ToString(lenDec: 2));
			}
#endif
			return charPosition;
		}

		//If eye position wasnt closer then check root position
		charPosition = character.GetOrigin();
		distance = vector.Distance(startingPos, charPosition);
		charDirection = vector.Direction(startingPos, charPosition).Normalized();
		dot = vector.Dot(charDirection, direction);
		nearestPosition = startingPos + direction * dot * distance;
		if (distanceAim > vector.Distance(nearestPosition, charPosition) && dot >= 0 && dot > dotAim)
		{
#ifdef ENABLE_DIAG
			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) == 3)
			{//GRAY SPHERE == Character root position being the closest, usable position
				m_DebugShapeMgr.AddSphere(charPosition, DEBUG_SPHERE_RADIUS * 0.3, Color.GRAY_25, DEBUG_SHAPE_FLAGS);
				m_DebugShapeMgr.AddLine(charPosition, nearestPosition, Color.SPRING_GREEN);
				m_DebugShapeMgr.AddSphere(nearestPosition, DEBUG_SPHERE_RADIUS * 0.3, Color.GRAY, DEBUG_SHAPE_FLAGS);

				AddDebugInfo("Distance:", "Root - " + GetPrefabName(character) + " at " + charPosition + " distance = " + distance.ToString(lenDec: 2));
			}
#endif
			return charPosition;
		}

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) == 3)
		{//BLUE SPHERE == Character center of mass being the closest, usable position
			m_DebugShapeMgr.AddSphere(charAimPosition, DEBUG_SPHERE_RADIUS * 0.3, Color.DODGER_BLUE, DEBUG_SHAPE_FLAGS);
			m_DebugShapeMgr.AddLine(charAimPosition, nearestPositionAim, Color.SPRING_GREEN);
			m_DebugShapeMgr.AddSphere(nearestPositionAim, DEBUG_SPHERE_RADIUS * 0.3, Color.DARK_BLUE, DEBUG_SHAPE_FLAGS);

			AddDebugInfo("Distance:", "Center of mass - " + GetPrefabName(character) + " at " + charAimPosition + " distance = " + distanceAim.ToString(lenDec: 2));
		}
#endif
		distance = distanceAim;
		return charAimPosition;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetClosestHitZonePosition(out vector entPosition, notnull ChimeraCharacter character, vector nearestPosition)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageMgr)
			return false;

		Physics physics = character.GetPhysics();
		if (!physics)
			return false;

		array<HitZone> charHitZones = {};
		damageMgr.GetAllHitZones(charHitZones);
		if (charHitZones.IsEmpty())
			return false;

		float colliderDistance;
		vector chosenCollider;
		vector colliderTransform[4] = {};
		array<int> colliderIDs = {};
		foreach (HitZone hitZone : charHitZones)
		{
			if (!hitZone.HasColliderNodes())
				continue;

			colliderIDs.Clear();
			hitZone.GetColliderIDs(colliderIDs);
			foreach (int ID : colliderIDs)
			{
				physics.GetGeomWorldTransform(ID, colliderTransform);
				colliderDistance = vector.DistanceSq(nearestPosition, colliderTransform[3]);
				if (chosenCollider[2] == 0 || chosenCollider[0] > colliderDistance)
				{
					chosenCollider = {colliderDistance, ID, 1};
					entPosition = colliderTransform[3];
				}
			}
		}

		return chosenCollider[2] != 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void QueryBlastedCharacters(vector startingPos[4], float length, out notnull array<ref SCR_BlastedEntityEntry> blastedEntities, float additionalDistance = 0)
	{
		if (!m_aFoundCharacters)
			m_aFoundCharacters = {};
		else
			m_aFoundCharacters.Clear();

		SCR_WeaponBlastComponentClass data = SCR_WeaponBlastComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		const float blastLength = data.GetBlastLength();
		const float coneAngle = data.GetBlastConeAngle();
		const BaseWorld world = GetOwner().GetWorld();
		const float radius = Math.Clamp(length * Math.Tan(coneAngle * Math.DEG2RAD), 0, length);
		const vector mins = {-radius, -radius, 0};
		const vector maxs = {radius, radius, length};

		//Query characters that are in the danger zone
		world.QueryEntitiesByOBB(mins, maxs, startingPos, QueryAddEntity, QueryFilter, QUERY_FLAGS);

		const vector direction = vector.Direction(startingPos[3], vector.Forward.Multiply3(startingPos) * length + startingPos[3]).Normalized();

#ifdef ENABLE_DIAG
		int debugValue = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST);
		bool showDebug = debugValue < 5;
		if (additionalDistance != 0)
			showDebug = debugValue < 4 || debugValue == 5;

		if (showDebug && debugValue != 0)
		{
			if (debugValue == 2)//BLACK == Area of query
				m_DebugShapeMgr.AddRectangle(startingPos[3], direction, length, radius * 2, Color.BLACK);

			//WHITE == AoE of the main blast
			if (additionalDistance == 0)
				m_DebugShapeMgr.Add(CreateCone(startingPos[3], direction, coneAngle, coneAngle, length / Math.Cos(coneAngle * Math.DEG2RAD), Color.WHITE, DEBUG_CONE_SUBDIV, ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP));
			else //YELLOW == AoE of the ricochet
				m_DebugShapeMgr.AddTrapezoidalPrism(startingPos[3], direction, additionalDistance * Math.Tan(coneAngle * Math.DEG2RAD), (length + additionalDistance) * Math.Tan(coneAngle * Math.DEG2RAD), length, DEBUG_CONE_SUBDIV, Color.YELLOW, ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP);
		}
#endif

		if (m_aFoundCharacters.IsEmpty())
			return;

		if (m_aFoundCharacters.Count() > MAX_BLAST_MEMBERS)
		{//Sorting is expensive so do it only when we need it
			SCR_EntityHelper.QuickSortEntitiesByDistanceToPoint(m_aFoundCharacters, startingPos[3], 0, m_aFoundCharacters.Count() - 1);
			m_aFoundCharacters.Resize(MAX_BLAST_MEMBERS);
		}

		const float maxCos = Math.Cos(coneAngle * Math.DEG2RAD);
		float distance, dot;
		vector hitPosDirNorm[3];
		vector entDirection, entPosition, nearestPosition;
		CharacterControllerComponent controller;

		//Prepare reusable part of the trace
		TraceParam traceParam = new TraceParam();
		traceParam.Start = startingPos[3];
		traceParam.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		traceParam.LayerMask = EPhysicsLayerDefs.Projectile;
		traceParam.Exclude = GetOwner();

		//Filter found entities
		ChimeraCharacter character;
		foreach (IEntity ent : m_aFoundCharacters)
		{
			character = ChimeraCharacter.Cast(ent);
			if (!character)
				continue;//Shouldnt happen but just in case reject those

			if (m_bIsAiCharacter && !data.CanAIDamageItself() && character == m_Instigator.GetInstigatorEntity())
				continue;//Skip if ai shouldnt harm itself

			controller = character.GetCharacterController();
			if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD)
				continue;//Ignore the dead to not waste time

			if (!controller.IsPlayerControlled() && data.ShouldIgnoreAIUnits())
				continue;

			nearestPosition = FindClosestCharacterPoint(startingPos[3], direction, character, distance);
			if (distance > length)
			{//Reject those that seem too far away
#ifdef ENABLE_DIAG
				if (showDebug && (debugValue == 2 || debugValue >= 4))
				{//VIOLET || PINK == too far from the source
					if (character == m_Instigator.GetInstigatorEntity())
						m_DebugShapeMgr.AddArrow(nearestPosition + vector.Up, nearestPosition, 0, Color.PINK);
					else
						m_DebugShapeMgr.AddArrow(nearestPosition + vector.Up, nearestPosition, 0, Color.VIOLET);

					AddDebugInfo("Distance:", "Rejected " + GetPrefabName(character) + " at " + nearestPosition + " - too far");
				}
#endif
				continue;
			}

			entDirection = vector.Direction(startingPos[3], nearestPosition).Normalized();
			dot = vector.Dot(direction, entDirection);
			if (dot < 0)
			{//Reject those that seem to be in the wrong direction
#ifdef ENABLE_DIAG
				if (showDebug && (debugValue == 2 || debugValue >= 4))
				{//CYAN == wrong direction
					if (character == m_Instigator.GetInstigatorEntity())
						m_DebugShapeMgr.AddArrow(nearestPosition + vector.Up, nearestPosition, 0, Color.CYAN);
					else
						m_DebugShapeMgr.AddArrow(nearestPosition + vector.Up, nearestPosition, 0, Color.DARK_CYAN);

					AddDebugInfo("Direction:", "Rejected " + GetPrefabName(character) + " at " + nearestPosition + " - wrong direction");
				}
#endif
				continue;
			}

			//Find nearest hit zone position
			if (!GetClosestHitZonePosition(entPosition, character, nearestPosition))
				continue;//Reject if we didnt find any usable position

			//Check the range agian this time to the actuall hitzone to ensure that its in range
			distance = vector.Distance(entPosition, startingPos[3]);
			if (distance > length)
			{
#ifdef ENABLE_DIAG
				if (showDebug && (debugValue == 2 || debugValue >= 4))
				{//GREEN == too far from the source
					if (character == m_Instigator.GetInstigatorEntity())
						m_DebugShapeMgr.AddArrow(entPosition + vector.Up, entPosition, 0, Color.SPRING_GREEN);
					else
						m_DebugShapeMgr.AddArrow(entPosition + vector.Up, entPosition, 0, Color.DARK_GREEN);

					AddDebugInfo("Distance:", "Rejected " + GetPrefabName(character) + " at " + entPosition + " - too far");
				}
#endif
				continue;
			}

			entDirection = vector.Direction(startingPos[3], entPosition).Normalized();
			dot = vector.Dot(direction, entDirection);
			//In case of ricochet just check if its within 90deg arc and then recalculate dot compensating for the starting size of the ricochet blast area
			if (additionalDistance > 0 && dot >= 0 && dot < maxCos)
				dot = vector.Dot(vector.Direction(startingPos[3] - vector.Forward.Multiply3(startingPos) * additionalDistance, entPosition).Normalized(), direction);

			//Reject based on the angle from the blast origin
			if (dot < maxCos)
			{
#ifdef ENABLE_DIAG
				if (showDebug && (debugValue == 2 || debugValue >= 4))
				{//BLUE == too much deviation from forward vector
					if (character == m_Instigator.GetInstigatorEntity())
						m_DebugShapeMgr.AddArrow(entPosition + vector.Up, entPosition, 0, Color.DODGER_BLUE);
					else
						m_DebugShapeMgr.AddArrow(entPosition + vector.Up, entPosition, 0, Color.DARK_BLUE);

					AddDebugInfo("Angle:", "Rejected " + GetPrefabName(character) + " at " + entPosition + " - outside of the max angle");
				}
#endif
				continue;
			}

			traceParam.End = entPosition;
			traceParam.TraceEnt = null;
			m_VerifiedEntity = character;
			world.TraceMove(traceParam, ObstructionTraceFilter);

			//Reject if there is something else on the way to the target
			if (traceParam.TraceEnt && character != traceParam.TraceEnt)
			{
#ifdef ENABLE_DIAG
				if (showDebug && (debugValue == 2 || debugValue >= 4))
				{//GRAY == something on the way to target
					if (character == m_Instigator.GetInstigatorEntity())
						m_DebugShapeMgr.AddArrow(entPosition + vector.Up, entPosition, 0, Color.GRAY_25);
					else
						m_DebugShapeMgr.AddArrow(entPosition + vector.Up, entPosition, 0, Color.GRAY);

					AddDebugInfo("Obstructed:", "At " + entPosition + " " + GetPrefabName(character) + " by " + GetPrefabName(traceParam.TraceEnt));
				}
#endif
				continue;
			}

			hitPosDirNorm[0] = entDirection * distance + traceParam.Start;
			hitPosDirNorm[1] = entDirection;
			hitPosDirNorm[2] = traceParam.TraceNorm.Normalized();

			SCR_BlastedEntityEntry entry = new SCR_BlastedEntityEntry(character, hitPosDirNorm[0], hitPosDirNorm[1], hitPosDirNorm[2], dot, vector.Distance(startingPos[3], entPosition) + additionalDistance, traceParam.NodeIndex, traceParam.ColliderIndex, traceParam.SurfaceProps);
			blastedEntities.Insert(entry);
#ifdef ENABLE_DIAG
			if (showDebug && debugValue != 0)
			{//RED == hit
				if (character == m_Instigator.GetInstigatorEntity())
					m_DebugShapeMgr.AddArrow(traceParam.Start, entPosition, 0, Color.RED);
				else
					m_DebugShapeMgr.AddArrow(traceParam.Start, entPosition, 0, Color.DARK_RED);

				AddDebugInfo("Hit:", GetPrefabName(character) + " at " + entPosition);
			}
#endif
		}

		m_aFoundCharacters.Clear();
		m_VerifiedEntity = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when launcher fires; Trace in the opposite direction of the warhead and apply damage to surrounding area
	protected void OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		if (!m_Instigator || m_Instigator.GetInstigatorType() == InstigatorType.INSTIGATOR_NONE)
		{
			SCR_ChimeraCharacter ownerCharacter = GetCharacterOwner();
			m_Instigator = Instigator.CreateInstigator(ownerCharacter);
		}

		m_bIsAiCharacter = !SCR_CharacterHelper.IsAPlayer(m_Instigator.GetInstigatorEntity());
		vector startTransform[4];
		m_BlastOrigin.GetWorldTransform(startTransform);
		array<ref SCR_BlastedEntityEntry> blastedEntities = {};
#ifdef ENABLE_DIAG
		if (!m_DebugShapeMgr)
			m_DebugShapeMgr = new SCR_DebugShapeManager();
		else
			m_DebugShapeMgr.Clear();

		DbgUI.BeginCleanupScope();
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) != 0)
		{
			DbgUI.Begin("Weapon blast legend");
			AddDebugLegendEntry("VIOLET", dbgPanelXSize, dbgPanelYSize, Color.VIOLET, "VIOLET Arrow - Rejected. Target entity is too far");
			AddDebugLegendEntry("PINK", dbgPanelXSize, dbgPanelYSize, Color.PINK, "PINK Arrow - Rejected. Character who caused the blast is too far");
			AddDebugLegendEntry("CYAN", dbgPanelXSize, dbgPanelYSize, Color.CYAN, "CYAN Arrow - Rejected. Outside of the area of influence");
			AddDebugLegendEntry("GREEN", dbgPanelXSize, dbgPanelYSize, Color.GREEN, "GREEN Arrow - Rejected. Selected hit zone is too far from the blast source");
			AddDebugLegendEntry("BLUE", dbgPanelXSize, dbgPanelYSize, Color.BLUE, "BLUE Arrow - Rejected. Direction to it deviates too much from the direction of the blast");
			AddDebugLegendEntry("GRAY", dbgPanelXSize, dbgPanelYSize, Color.GRAY, "GRAY Arrow - Rejected. Something obstructing it");
			AddDebugLegendEntry("RED", dbgPanelXSize, dbgPanelYSize, Color.RED, "RED Arrow - Object hit");
			AddDebugLegendEntry("GREEN_S", dbgPanelYSize, dbgPanelYSize, Color.GREEN, "GREEN Sphere - Using character eye position as it is the closest point");
			AddDebugLegendEntry("GRAY_S", dbgPanelYSize, dbgPanelYSize, Color.GRAY, "GRAY Sphere - Using character root position as it is the closest point");
			AddDebugLegendEntry("BLUE_S", dbgPanelYSize, dbgPanelYSize, Color.BLUE, "BLUE Sphere - Using character center of mass as it is the closest point");
			AddDebugLegendEntry("BLACK_S", dbgPanelYSize, dbgPanelYSize, Color.BLACK, "BLACK Sphere - Rejected. Inssuficient amount of damage");
			AddDebugLegendEntry("RED_S", dbgPanelYSize, dbgPanelYSize, Color.RED, "RED Sphere - Object damaged");
			DbgUI.End();

			if (!m_aDebugData)
				m_aDebugData = {};
			else
				m_aDebugData.Clear();
		}
#endif

		IEntity owner = GetOwner();
		SCR_WeaponBlastComponentClass data = SCR_WeaponBlastComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		float blastLength = data.GetBlastLength();
		if (!m_bIsAiCharacter || data.CanAIBlastCharacters())
			QueryBlastedCharacters(startTransform, blastLength, blastedEntities);

		if (data.CanBlastRicochet())
		{
			//Trace for a surface to ricochet from
			BaseWorld world = owner.GetWorld();
			TraceParam traceParam = new TraceParam();
			traceParam.Start = startTransform[3];
			traceParam.End = vector.Forward.Multiply3(startTransform) * blastLength + traceParam.Start;
			traceParam.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
			array<IEntity> excludeArray = {owner, m_Instigator.GetInstigatorEntity()};
			traceParam.ExcludeArray = excludeArray;
			traceParam.LayerMask = EPhysicsLayerDefs.Projectile;
			float hit = world.TraceMove(traceParam, TraceFilter);

			if (traceParam.TraceEnt && hit < 1 && !float.AlmostEqual(hit, 1, 0.1))
			{
				bool ricochetFromDamageEnt;
				foreach (SCR_BlastedEntityEntry entry : blastedEntities)
				{
					if (entry.GetTargetEntity() == traceParam.TraceEnt)
					{
						ricochetFromDamageEnt = true;
						entry.SetAngleToTarget(1);
						break;
					}
				}

				vector hitPosDirNorm[3];
				vector dir = traceParam.End - traceParam.Start;
				hitPosDirNorm[0] = dir * hit + traceParam.Start;
				hitPosDirNorm[1] = dir.Normalized();
				hitPosDirNorm[2] = traceParam.TraceNorm.Normalized();

				float hitDistance = blastLength * hit;
				bool ricochetFromCharacter = ChimeraCharacter.Cast(traceParam.TraceEnt) != null;
				if (!ricochetFromDamageEnt && (!m_bIsAiCharacter || !ricochetFromCharacter && data.CanAIBlastDestructible() || ricochetFromCharacter && data.CanAIBlastCharacters()))
					blastedEntities.Insert(new SCR_BlastedEntityEntry(traceParam.TraceEnt, hitPosDirNorm[0], hitPosDirNorm[1], hitPosDirNorm[2], 1, hitDistance, traceParam.NodeIndex, traceParam.ColliderIndex, traceParam.SurfaceProps));

				CalculateRicochetDirection(hitPosDirNorm, startTransform, data.GetDeflectionFlatteningStrength());
#ifdef ENABLE_DIAG
				if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) != 0)
				{
					m_DebugShapeMgr.AddArrow(traceParam.Start, hitPosDirNorm[0], 0, Color.ORANGE);
					m_DebugShapeMgr.AddSphere(startTransform[3], DEBUG_SPHERE_RADIUS, Color.ORANGE, DEBUG_SHAPE_FLAGS);
				}
#endif
				m_VerifiedEntity = traceParam.TraceEnt;//to ignore it in the query

				if (!m_bIsAiCharacter || data.CanAIBlastCharacters())
					QueryBlastedCharacters(startTransform, blastLength - hitDistance, blastedEntities, hitDistance);
			}
		}

		if (blastedEntities.Count() > 0)
			ApplyDamage(blastedEntities);

		m_Instigator = Instigator.CreateInstigator(null);
		m_bIsAiCharacter = false;
#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) != 0 && m_aDebugData && !m_aDebugData.IsEmpty())
		{
			// This is necessary as otherwise the list is not going to be recreated with next shot and thus it will have old content
			const string time = System.GetUnixTime().ToString();
			int outSelection;

			DbgUI.Begin("Weapon blast details", y: 300);
			foreach (int i, TStringArray arr : m_aDebugData)
			{
				DbgUI.List("List_" + i + time, outSelection, arr);
			}

			DbgUI.End();
		}
		DbgUI.EndCleanupScope();
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyDamage(notnull array<ref SCR_BlastedEntityEntry> blastedEntities)
	{
		IEntity owner = GetOwner();
		SCR_WeaponBlastComponentClass data = SCR_WeaponBlastComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		array<SCR_WeaponBlastEffect> blastEffects = {};
		if (data.GetBlastEffects(blastEffects) < 1)
			return;

		bool isCharacter;
		SCR_DestructibleEntity destructibleEntity;
		vector hitPosDirNorm[3];
		SCR_DamageContext context;
		SCR_DamageManagerComponent damageManager;
		HitZone hitZone;
		float maxHealth;
		float computedDamageAmount;
		float damageValue;
		const float blastLength = data.GetBlastLength();
		const float destructibleDamageMultiplier = data.GetDestructibleDamageMultiplier();

		foreach (SCR_BlastedEntityEntry entry : blastedEntities)
		{
			//Check if the entity is destructible entity
			destructibleEntity = SCR_DestructibleEntity.Cast(entry.GetTargetEntity());
			if (destructibleEntity)
			{
				foreach (int i, SCR_WeaponBlastEffect effect : blastEffects)
				{
					damageValue = effect.GetComputedDamage(entry.GetDistanceToTarget() / blastLength, entry.GetAngleToTarget()) * destructibleDamageMultiplier;

					entry.GetTargetHitPosDirNorm(hitPosDirNorm);
					destructibleEntity.HandleDamage(effect.GetDamageType(), damageValue, hitPosDirNorm);
#ifdef ENABLE_DIAG
					if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) != 0)
					{//TEAL SPHERE == Damage amount for destructible entity
						float factor = Math.Lerp(0, 1, (1 - entry.GetDistanceToTarget() / blastLength) * entry.GetAngleToTarget());
						Color damageColor = new Color(factor * 0.1, factor, factor, 1);
						m_DebugShapeMgr.AddSphere(hitPosDirNorm[0], DEBUG_SPHERE_RADIUS + i * 0.05, damageColor.PackToInt(), DEBUG_SHAPE_FLAGS);

						AddDebugInfo("Damage dealt:", GetPrefabName(destructibleEntity) + " at " + hitPosDirNorm[0] + ", value = " + damageValue.ToString(lenDec: 3));
					}
#endif
				}
				continue;
			}

			isCharacter = ChimeraCharacter.Cast(entry.GetTargetEntity()) != null;
			//Check if the entity has the damage manager component
			damageManager = SearchHierarchyForDamageManager(entry.GetTargetEntity(), entry.GetTargetColliderId(), hitZone);
			if (!damageManager || !hitZone)
				continue;

			entry.GetTargetHitPosDirNorm(hitPosDirNorm);
			maxHealth = hitZone.GetMaxHealth();
			foreach (int i, SCR_WeaponBlastEffect effect : blastEffects)
			{
				damageValue = effect.GetComputedDamage(entry.GetDistanceToTarget() / blastLength, entry.GetAngleToTarget());
				if (!isCharacter)
					damageValue *= destructibleDamageMultiplier;//Damage multiplier for destructibles

				context = new SCR_DamageContext(effect.GetDamageType(), damageValue, hitPosDirNorm,
												entry.GetTargetEntity(), hitZone, m_Instigator,
												entry.GetTargetSurfaceProps(), entry.GetTargetColliderId(), entry.GetTargetNodeId());

				context.damageEffect = effect.GetDamageEffect();
				context.damageSource = owner;
				computedDamageAmount = hitZone.ComputeEffectiveDamage(context, false);
				if (computedDamageAmount == 0 || computedDamageAmount / maxHealth < 0.01)
				{
#ifdef ENABLE_DIAG
					if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) != 0)
					{//BLACK SPHERE == NOT ENOUGH DAMAGE
						m_DebugShapeMgr.AddSphere(hitPosDirNorm[0], DEBUG_SPHERE_RADIUS + i * 0.05, Color.BLACK, DEBUG_SHAPE_FLAGS);

						AddDebugInfo("Damage insufficient:", "Effect nr = " + i + " " + GetPrefabName(context.hitEntity) + " at " + context.hitPosition + ", value = " + computedDamageAmount + " < " + (maxHealth * 0.01));
					}
#endif
					continue;//Skip damage handling if damage is negligible
				}

				damageManager.HandleDamage(context);
#ifdef ENABLE_DIAG
				if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST) != 0)
				{//RED SPHERE == DAMAGE AMOUNT
					float factor = Math.Lerp(0, 1, context.damageValue / effect.GetDamageValueRaw());
					Print(factor);
					Color damageColor = new Color(factor, factor * 0.1, factor * 0.1, 1);
					m_DebugShapeMgr.AddSphere(hitPosDirNorm[0], DEBUG_SPHERE_RADIUS + i * 0.05, damageColor.PackToInt(), DEBUG_SHAPE_FLAGS);

					AddDebugInfo("Damage dealt:", "Effect nr = " + i + " " + GetPrefabName(context.hitEntity) + " at " + context.hitPosition + ", value = " + context.damageValue.ToString(lenDec: 3) + " (max dmg = " + effect.GetDamageValueRaw() + ")");
				}
#endif
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_DamageManagerComponent SearchHierarchyForDamageManager(IEntity startEntity, int colliderIndex, out HitZone hitZone)
	{
		if (!startEntity)
			return null;

		SCR_DamageManagerComponent damageManager;
		while (startEntity)
		{
			damageManager = SCR_DamageManagerComponent.GetDamageManager(startEntity);
			if (damageManager)
				break;

			startEntity = startEntity.GetParent();
		}

		if (damageManager)
		{
			hitZone = damageManager.GetHitZoneByColliderID(colliderIndex);
			if (!hitZone)
				hitZone = damageManager.GetDefaultHitZone();
		}

		return damageManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the top most owner of this object or the operator of this turret
	protected SCR_ChimeraCharacter GetCharacterOwner()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		if (Turret.Cast(owner))
		{
			SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
			if (!compartmentManager || !compartmentManager.AnyCompartmentsOccupiedOrLocked())
				return null;

			array<BaseCompartmentSlot> outCompartments = {};
			if (compartmentManager.GetCompartments(outCompartments) < 1)
				return null;

			foreach (BaseCompartmentSlot compartment : outCompartments)
			{
				if (TurretCompartmentSlot.Cast(compartment))
					return SCR_ChimeraCharacter.Cast(compartment.GetOccupant());
			}
		}
		else
		{
			IEntity parent = owner.GetParent();
			SCR_ChimeraCharacter character;
			while (parent)
			{
				character = SCR_ChimeraCharacter.Cast(parent);
				if (character)
					return character;

				parent = parent.GetParent();
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		SCR_MuzzleEffectComponent muzzleEffectComponent = SCR_MuzzleEffectComponent.Cast(owner.FindComponent(SCR_MuzzleEffectComponent));
		if (!muzzleEffectComponent)
			return;

		muzzleEffectComponent.GetOnWeaponFired().Remove(OnWeaponFired);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (rplComp && rplComp.Role() != RplRole.Authority)
			return;

		SCR_WeaponBlastComponentClass data = SCR_WeaponBlastComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		if (!m_BlastOrigin)
			return;

		if (data.GetBlastLength() == 0)
			return;

		SCR_MuzzleEffectComponent muzzleEffectComponent = SCR_MuzzleEffectComponent.Cast(owner.FindComponent(SCR_MuzzleEffectComponent));
		if (!muzzleEffectComponent)
			return;

		muzzleEffectComponent.GetOnWeaponFired().Insert(OnWeaponFired);
		m_BlastOrigin.Init(owner);
#ifdef ENABLE_DIAG
		//NOTE: This is only present for the authority (f.e. selfhosted game or workbench)
		DiagMenu.RegisterItem(SCR_DebugMenuID.DEBUGUI_WEAPONS_BLAST, "", "Visualize weapon blast", "Damage", "disabled,hit,all,posDebug,onlyMainBlast,onlyRicochet");
#endif
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}
