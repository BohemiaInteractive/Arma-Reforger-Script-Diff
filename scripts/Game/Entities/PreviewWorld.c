class PreviewWorldExampleClass: GenericEntityClass
{
};

class PreviewWorldExample: GenericEntity
{
	static ref array<ResourceName> s_Models = {"{9A85D4C454E19ED7}entities/Props/Crates/Crate_02.et", "{A0190761A41888C6}entities/Props/Military/AmmoBoxes/AmmoBox_03.et", "{728DD36004461456}entities/Props/Furniture/Cupboard_01_B.et"};
	ref SharedItemRef m_World;
	RenderTargetWidget m_RenderWidget;
	IEntity m_Model;
	vector m_Angle;
	int m_ModelIndex;
	
	void PreviewWorldExample(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	void ~PreviewWorldExample()
	{
		delete m_RenderWidget;
	}
	
	override void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		m_World = BaseWorld.CreateWorld("Preview", "Preview");
		BaseWorld world = m_World.GetRef();
		m_RenderWidget = RenderTargetWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.RenderTargetWidgetTypeID, WidgetFlags.VISIBLE, new Color(), 100));
		FrameSlot.SetOffsets(m_RenderWidget, 0, 0, 0, 0);
		FrameSlot.SetAnchorMin(m_RenderWidget, 0.1, 0.1);
		FrameSlot.SetAnchorMax(m_RenderWidget, 0.5, 0.5);
		m_RenderWidget.Update();
		m_RenderWidget.SetWorld(world, 0);
		
		// create generic world
		Resource rsc = Resource.Load("{43DA8BEC8468C7F5}worlds/TestMaps/PreviewWorld/PreviewWorld.et");	
		if (rsc.IsValid())
			GetGame().SpawnEntityPrefab(rsc, world);
		
		// setup cam
		world.SetCamera(0, "0 0.5 -3", "0 0 0");
		world.SetCameraType(0, CameraType.PERSPECTIVE);
		world.SetCameraNearPlane(0, 0.1);
		world.SetCameraFarPlane(0, 4000);
		world.SetCameraVerticalFOV(0, 60);
		
		// load model
		LoadModel();
	}
	
	void LoadModel()
	{
		if (m_Model)
			delete m_Model;
		
		Resource rsc = Resource.Load(s_Models[m_ModelIndex]);		
		if (rsc.IsValid())
		{
			BaseWorld world = m_World.GetRef();
			m_Model = GetGame().SpawnEntityPrefab(rsc, world);
			m_Model.SetOrigin("0 0 0");
		}
		
		m_ModelIndex++;
		if (m_ModelIndex >= s_Models.Count())
			m_ModelIndex = 0;
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_Angle[0] = m_Angle[0] + timeSlice * 30;
		while (m_Angle[0] > 180.0)
		{
			LoadModel();
			m_Angle[0] = m_Angle[0] - 180.0;
		}
		
		m_Model.SetYawPitchRoll(m_Angle);
	}
};



