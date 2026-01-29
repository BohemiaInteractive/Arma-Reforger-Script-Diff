//#define DEBUG_SHAPE
[EntityEditorProps(category: "GameScripted/Editor", description: "Core Editor manager", color: "251 91 0 255", dynamicBox: true)]
class SCR_CompositionValidationEntityClass: SCR_GenericBoxEntityClass
{
}

class SCR_CompositionValidationEntity : SCR_GenericBoxEntity
{
	[Attribute(category: "Slot")]
	private vector m_vSlotSize;
	
	[Attribute(category: "Slot")]
	private bool m_bIsRectangle;
	
	[Attribute(defvalue: "0.1")]
	private float m_fRefreshRate;
	
#ifdef WORKBENCH
	private bool m_bCompositionSearched;
	private IEntitySource m_CompositionSource;
	private float m_fTime;
	private ref DebugTextScreenSpace m_Title;
	private ref DebugTextScreenSpace m_Text;
#ifdef DEBUG_SHAPE
	private ref Shape m_Shape;
#endif

	//------------------------------------------------------------------------------------------------
	protected bool InArea(vector pos)
	{
		pos = CoordToLocal(pos);
		float sizeX = m_vSlotSize[0];
		float sizeY = m_vSlotSize[1];
		float sizeZ = m_vSlotSize[2];
		
		if (m_bIsRectangle)
		{
			return (pos[0] > -sizeX && pos[0] < sizeX) && (pos[2] > -sizeZ && pos[2] < sizeZ);
		}
		else
		{
			return Vector(pos[0], 0, pos[2]).Length() < sizeX;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindComposition()
	{
		if (m_bCompositionSearched) 
		{
			if (m_CompositionSource)
				return true;
			else 
				return false;
		}
		
		m_bCompositionSearched = true;
		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return false;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
			return false;
		
		for (int i = 0, containersCount = worldEditor.GetNumContainers(); i < containersCount; i++)
		{
			IEntitySource entitySource = IEntitySource.Cast(worldEditor.GetContainer(i));
			if (!entitySource || entitySource.GetSubScene() != worldEditorAPI.GetCurrentSubScene() || entitySource.GetLayerID() != worldEditorAPI.GetCurrentEntityLayerId())
				continue;
			
			//--- Get composition prefab
			m_CompositionSource = entitySource;
			break;
		}

		if (!m_CompositionSource)
		{
			Print("Cannot find the composition, no entities are present in the current layer.", LogLevel.WARNING);
			return false;
		}

		m_Title.SetText(m_CompositionSource.GetResourceName());

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessEntities(IEntitySource entitySource, out int count)
	{
		IEntitySource childSource;
		IEntity child;
		vector boundMin, boundMax;
		vector boundPoints[8];
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		for (int i = 0, countChildren = entitySource.GetNumChildren(); i < countChildren; i++)
		{
			childSource = entitySource.GetChild(i);
			child = worldEditorAPI.SourceToEntity(childSource);
			if (!child)
				continue;
			
			//--- Check if it's inside slot area
			child.GetBounds(boundMin, boundMax);
			if (!InArea(child.CoordToParent(Vector(boundMin[0], boundMin[1], boundMin[2])))
			 || !InArea(child.CoordToParent(Vector(boundMax[0], boundMin[1], boundMin[2])))
			 || !InArea(child.CoordToParent(Vector(boundMin[0], boundMax[1], boundMin[2])))
			 || !InArea(child.CoordToParent(Vector(boundMax[0], boundMax[1], boundMin[2])))
			 || !InArea(child.CoordToParent(Vector(boundMin[0], boundMin[1], boundMax[2])))
			 || !InArea(child.CoordToParent(Vector(boundMax[0], boundMin[1], boundMax[2])))
			 || !InArea(child.CoordToParent(Vector(boundMin[0], boundMax[1], boundMax[2])))
			 || !InArea(child.CoordToParent(Vector(boundMax[0], boundMax[1], boundMax[2])))
			)
			{
				vector pos = child.GetOrigin();
				DebugTextWorldSpace.Create(GetWorld(), "!", DebugTextFlags.FACE_CAMERA | DebugTextFlags.CENTER | DebugTextFlags.ONCE, pos[0], pos[1], pos[2], 30, Color.WHITE, Color.RED);
			}
			
			//--- Check if it has replication component
			/*if (child.FindComponent(RplComponent))
			{
				vector pos = child.GetOrigin();
				DebugTextWorldSpace.Create(GetWorld(), "Rpl", DebugTextFlags.FACE_CAMERA | DebugTextFlags.CENTER | DebugTextFlags.ONCE, pos[0], pos[1], pos[2], 10, Color.WHITE, Color.ORANGE);
			}*/
			
			//--- Increase counter and go deeper
			count += 1;
			ProcessEntities(childSource, count);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (!FindComposition())
			return;
		
		//--- Save performance by not refreshing every frame
		if (m_fTime < m_fRefreshRate)
		{
			m_fTime += timeSlice;
			return;
		}
		
		//Print(worldEditorAPI.GetSelectedEntity(0));
		
		int count = 1;
		ProcessEntities(m_CompositionSource, count);
		m_Text.SetText(string.Format("Entities: %1", count));
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_CompositionValidationEntity(IEntitySource src, IEntity parent)
	{
		m_Title = DebugTextScreenSpace.Create(GetWorld(), "", DebugTextFlags.FACE_CAMERA, 10, 10, 20, ARGBF(1, 1, 1, 1), ARGBF(1, 0, 0, 0));
		m_Text = DebugTextScreenSpace.Create(GetWorld(), "", DebugTextFlags.FACE_CAMERA, 10, 33, 14, ARGBF(1, 1, 1, 1), ARGBF(1, 0, 0, 0));
		
#ifdef DEBUG_SHAPE
		if (m_bIsRectangle)
			m_Shape = Shape.Create(ShapeType.BBOX, ARGBF(0.1, 1, 1, 1), ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE | ShapeFlags.ADDITIVE | ShapeFlags.DOUBLESIDE, GetOrigin() - m_vSlotSize, GetOrigin() + m_vSlotSize);
		else
			m_Shape = Shape.CreateCylinder(ARGBF(0.1, 1, 1, 1), ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE | ShapeFlags.ADDITIVE | ShapeFlags.DOUBLESIDE, GetOrigin(), m_vSlotSize[0], m_vSlotSize[2]);
#endif
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CompositionValidationEntity()
	{
#ifdef DEBUG_SHAPE
		delete m_Shape;
#endif
	}
#endif
}
