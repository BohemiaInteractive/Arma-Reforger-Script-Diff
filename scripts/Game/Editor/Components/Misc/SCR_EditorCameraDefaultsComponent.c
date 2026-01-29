[ComponentEditorProps(category: "GameScripted/Editor", description: "")]
class SCR_EditorCameraDefaultsComponentClass: ScriptComponentClass
{
};

/** @ingroup Editable_Entities
*/

/*!
Starting camera positions for the editor.
*/
class SCR_EditorCameraDefaultsComponent : ScriptComponent
{
	[Attribute()]
	protected ref array<ref SCR_CameraCoordinates> m_aPositions;

	/*!
	Get random camera position from the list of pre-defined positions.
	\param[out] transform Transformation matrix
	\return True if a position was found
	*/
	bool GetRandomPosition(out vector transform[4])
	{
		if (!m_aPositions || m_aPositions.Count() == 0) return false;
		
		SCR_CameraCoordinates coordinates = m_aPositions[Math.RandomInt(0, m_aPositions.Count())];
		coordinates.GetTransform(transform);
		return true;
	}
	/*!
	Get instance o this component.
	It's first searched on BaseGameMode. If it's not found there, GenericWorldEntity is queried next.
	\return Component instance
	*/
	static SCR_EditorCameraDefaultsComponent GetInstance()
	{
		SCR_EditorCameraDefaultsComponent result;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
		{
			result = SCR_EditorCameraDefaultsComponent.Cast(gameMode.FindComponent(SCR_EditorCameraDefaultsComponent));
			if (result) return result;
		}
		
		GenericWorldEntity world = GetGame().GetWorldEntity();
		if (world)
		{
			result = SCR_EditorCameraDefaultsComponent.Cast(world.FindComponent(SCR_EditorCameraDefaultsComponent));
			if (result) return result;
		}
		
		return null;
	}
	
#ifdef WORKBENCH
	override void _WB_OnContextMenu(IEntity owner, int id)
	{
		GenericEntity genericOwner = GenericEntity.Cast(owner);
		if (!genericOwner) return;
		
		WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
		if (!api) return;
		
		BaseWorld world = api.GetWorld();
		if (!world) return;
		
		api.BeginEntityAction();
		
		IEntitySource ownerSource = api.EntityToSource(owner);
		int index = m_aPositions.Count();
		array<ref ContainerIdPathEntry> path = {new ContainerIdPathEntry(Type().ToString())};
		api.CreateObjectArrayVariableMember(ownerSource, path, "m_aPositions", "SCR_CameraCoordinates", index);
		
		vector transform[4];
		world.GetCurrentCamera(transform);
		vector pos = transform[3];
		vector dir = Math3D.MatrixToAngles(transform);
		
		path.Insert(new ContainerIdPathEntry("m_aPositions", index));
		api.SetVariableValue(ownerSource, path, "m_vCameraPosition", string.Format("%1 %2 %3", pos[0], pos[1], pos[2]));
		api.SetVariableValue(ownerSource, path, "m_vCameraAngles", string.Format("%1 %2 %3", dir[1], dir[0], dir[2]));
		
		path = null;
		api.EndEntityAction();
	}
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner)
	{
		return { new WB_UIMenuItem("Add current camera coordinates", 0) };
	}
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (!m_aPositions) return;
		
		GenericEntity genericOwner = GenericEntity.Cast(owner);
		if (!genericOwner) return;
		
		WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
		if (!api || !api.IsEntitySelected(api.EntityToSource(owner))) return;
		
		const float length = 200; // TODO: check for good const usage
		const float width = 160; // TODO: check for good const usage
		const float height = 90; // TODO: check for good const usage
		
		vector transform[4];
		foreach (SCR_CameraCoordinates coordinate: m_aPositions)
		{
			coordinate.GetTransform(transform);
			
			vector points[12];
			
			points[0] = transform[3];
			points[1] = transform[3] + transform[2] * length + transform[1] * height + transform[0] * -width;
			points[2] = transform[3] + transform[2] * length + transform[1] * height + transform[0] * width;
			
			points[3] = transform[3];
			points[4] = transform[3] + transform[2] * length + transform[1] * -height + transform[0] * width;
			points[5] = transform[3] + transform[2] * length + transform[1] * height + transform[0] * width;
			
			points[6] = transform[3];
			points[7] = transform[3] + transform[2] * length + transform[1] * -height + transform[0] * -width;
			points[8] = transform[3] + transform[2] * length + transform[1] * -height + transform[0] * width;
			
			points[9] = transform[3];
			points[10] = transform[3] + transform[2] * length + transform[1] * -height + transform[0] * -width;
			points[11] = transform[3] + transform[2] * length + transform[1] * height + transform[0] * -width;

			Shape.CreateTris(ARGBF(0.1, 1, 0, 1), ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE, points, 4);	
			Shape.CreateSphere(ARGBF(1, 1, 0, 1), ShapeFlags.ONCE, transform[3], 1);
		}
	}
#endif
};

[BaseContainerProps()]
class SCR_CameraCoordinates
{
	[Attribute(desc: "Camera position in format lateral, vertical, longitudinal")]
	protected vector m_vCameraPosition;
	
	[Attribute(desc: "Camera angles in format pitch, yaw, roll\n(the same as World Editor camera, you can copy the value from there)")]
	protected vector m_vCameraAngles;
	
	void GetTransform(out vector transform[4])
	{
		vector angles = Vector(m_vCameraAngles[1], m_vCameraAngles[0], m_vCameraAngles[2]);
		Math3D.AnglesToMatrix(angles, transform);
		transform[3] = m_vCameraPosition;
	}
};