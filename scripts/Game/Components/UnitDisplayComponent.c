[ComponentEditorProps(category: "GameScripted/Spectating", description: "Displays a colored sphere over an entity - if FactionAffiliationComponent is present")]
class SCR_UnitDisplayComponentClass : ScriptComponentClass
{
}

class SCR_UnitDisplayComponent : ScriptComponent
{
	private FactionAffiliationComponent m_FactionAffiliatonComponent;
	private DamageManagerComponent m_DamageManagerComponent;
	
	private ref Shape m_Shape = null;
	private ref Shape m_ShapeHealth = null;
	private ref Shape m_ShapeHealthBackground = null;
	private float m_DistanceToCamera = 0.0;	
	
	// Holds offset from the entity attached to
	private vector m_Position = vector.Zero;
	private IEntity m_Entity = null;
	
	//------------------------------------------------------------------------------------------------
	//! Draw a debug shape every frame
	//! \param[in] owner
	//! \param[in] timeSlice
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// No faction affiliation component, delete shape if it exists, return
		if (!m_FactionAffiliatonComponent || !m_DamageManagerComponent)
		{
			if (m_Shape)
			{
				delete m_Shape;
				m_Shape = null;
			}
			if (m_ShapeHealth)
			{
				delete m_ShapeHealth;
				m_ShapeHealth = null;
			}
			if (m_ShapeHealthBackground)
			{
				delete m_ShapeHealthBackground;
				m_ShapeHealthBackground = null;
			}
			return;
		}
		// If unit is dead (destroyed) delete shape, return
		if (m_DamageManagerComponent)
		{
			EDamageState state = m_DamageManagerComponent.GetState();
			if (state == EDamageState.DESTROYED)
			{
				if (m_Shape)
				{
					delete m_Shape;
					m_Shape = null;
				}
				if (m_ShapeHealth)
				{
					delete m_ShapeHealth;
					m_ShapeHealth = null;
				}
				if (m_ShapeHealthBackground)
				{
					delete m_ShapeHealthBackground;
					m_ShapeHealthBackground = null;
				}
				return;
			}
		}
		
		// Get position
		m_Position = owner.GetOrigin();
		m_Position[1] = m_Position[1] + SCR_UnitDisplaySettings.s_fShapeHeightOffset;
		
		// Get camera matrix
		vector camMat[4];
		m_Entity.GetWorld().GetCurrentCamera(camMat);
		m_DistanceToCamera = vector.Distance(m_Position, camMat[3]);
		
		float scaleMult;
		if (SCR_UnitDisplaySettings.s_bShapeUseScaling)
		{
			// Calculate distance
			if (m_DistanceToCamera == 0)	// to prevent division by 0
				scaleMult = 1.0;
			else if (m_DistanceToCamera > 0)
				scaleMult = 1 / (SCR_UnitDisplaySettings.s_fShapeSizeGoal / m_DistanceToCamera);
			
			// Get scale multiplier, color from faction and update shape
			scaleMult = Math.Clamp(scaleMult, SCR_UnitDisplaySettings.s_fShapeSizeMinimum, SCR_UnitDisplaySettings.s_fShapeSizeMaximum);
		}
		else
		{
			scaleMult = 1.0;
		}
		
		// Get color from faction affiliation, get proper flags, create shape
		int color = 0;
		Faction affiliatedFaction =  m_FactionAffiliatonComponent.GetAffiliatedFaction();
		if (affiliatedFaction)
			color = affiliatedFaction.GetFactionColor().PackToInt();
		if (color != 0)
		{
			int flags = ShapeFlags.NOOUTLINE;
			if (SCR_UnitDisplaySettings.s_bShapeUseAlpha)
				flags |= ShapeFlags.TRANSP;
			m_Shape = Shape.CreateSphere(color, flags, m_Position, SCR_UnitDisplaySettings.s_fShapeRadius * scaleMult);
		}
		
		// Check if text should be drawn and draw it if so
		if (SCR_UnitDisplaySettings.s_bShapeUseText)
		{		
			// If entity is in text drawing distance, get cam matrix, use negative forward and draw text above
			if (m_DistanceToCamera <= SCR_UnitDisplaySettings.s_fShapeTextDistance)
			{	
				vector mat[4];
				owner.GetTransform(mat);
				mat[3][1] = mat[3][1] + SCR_UnitDisplaySettings.s_fShapeTextHeightOffset;
				mat[0] = camMat[0];
				mat[1] = camMat[1];
				mat[2] = -camMat[2];
				
				string name = owner.GetName();
				int textcolor = color;
				CreateSimpleText(name, mat, SCR_UnitDisplaySettings.s_fShapeTextSize, textcolor, ShapeFlags.ONCE, null, 1, false);
			}
		}
		
		if (SCR_UnitDisplaySettings.s_bShapeUseHealthbar)
		{
			// If entity is in text drawing distance, get cam matrix, use negative forward and draw text above
			if (m_DistanceToCamera <= SCR_UnitDisplaySettings.s_fShapeTextDistance)
			{
				string name = owner.GetName();
				int hpcolor = color;
				float health = m_DamageManagerComponent.GetHealthScaled();
				
				vector mat[4];
				owner.GetTransform(mat);			
				mat[0] = camMat[0];
				mat[1] = camMat[1];
				mat[2] = -camMat[2];
				mat[3][1] = mat[3][1] + SCR_UnitDisplaySettings.s_fShapeTextHeightOffset + SCR_UnitDisplaySettings.s_fShapeHealthbarHeight + 0.05;
				
				if (SCR_UnitDisplaySettings.s_bShapeUseText)
					mat[3][1] = mat[3][1] + SCR_UnitDisplaySettings.s_fShapeTextSize;				
				
				vector min, max;
				min = Vector(- SCR_UnitDisplaySettings.s_fShapeHealthbarWidth * 0.5, -SCR_UnitDisplaySettings.s_fShapeHealthbarHeight, 0.01);
				max = Vector( (health*SCR_UnitDisplaySettings.s_fShapeHealthbarWidth)-SCR_UnitDisplaySettings.s_fShapeHealthbarWidth * 0.5, SCR_UnitDisplaySettings.s_fShapeHealthbarHeight, 0.01);
				vector minAbsolute = min;
				minAbsolute[2] = 0.001;
				vector maxAbsolute = Vector(SCR_UnitDisplaySettings.s_fShapeHealthbarWidth * 0.5, SCR_UnitDisplaySettings.s_fShapeHealthbarHeight, 0.001);
				
				m_ShapeHealthBackground = Shape.Create(ShapeType.BBOX, ARGB(255,255,255,255), 0, minAbsolute, maxAbsolute);
				m_ShapeHealthBackground.SetMatrix(mat);
				m_ShapeHealth = Shape.Create(ShapeType.BBOX, hpcolor, 0, min, max);
				m_ShapeHealth.SetMatrix(mat);
			}
			else
			{
				if (m_ShapeHealth)
					delete m_ShapeHealth;
				if (m_ShapeHealthBackground)
					delete m_ShapeHealthBackground;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.FRAME);	
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_UnitDisplayComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{		
		GenericEntity genEnt = GenericEntity.Cast(ent);
		if (genEnt)
		{
			m_FactionAffiliatonComponent = FactionAffiliationComponent.Cast(genEnt.FindComponent(FactionAffiliationComponent));
			m_DamageManagerComponent = DamageManagerComponent.Cast(genEnt.FindComponent(DamageManagerComponent));
		}
		
		m_Entity = ent;
	}
}
