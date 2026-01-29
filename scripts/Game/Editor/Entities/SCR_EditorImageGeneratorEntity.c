[EntityEditorProps(category: "GameScripted/Editor", description: "Manager of image generators", color: "255 0 0 255", dynamicBox: true)]
class SCR_EditorImageGeneratorEntityClass: GenericEntityClass
{
};

/** @ingroup Editor_Entities
*/

/*!

*/
class SCR_EditorImageGeneratorEntity : GenericEntity
{
	[Attribute("800")]
	protected int m_iImageWidth;

	[Attribute("600")]
	protected int m_iImageHeight;

	[Attribute(desc: "When enabled, the system will not iterate through prefabs on its own.\nInstead, you'll have to press 'Space' to continue to next prefab.")]	
	protected bool m_bManualProgress;
	
	[Attribute(desc: "When enabled, the system will not check for correct viewport resolution and screenshot format upon start, and not capture any screenshots.\nFOR DEBUGGING ONLY!")]
	protected bool m_bDebugMode;
	
#ifdef WORKBENCH
	protected int m_iInit = 0;
	protected ref SCR_SortedArray<SCR_EditorImagePositionEntity> m_aPositions = new SCR_SortedArray<SCR_EditorImagePositionEntity>();
	protected ref array<ref SCR_EditorImageGeneratorPrefab> m_aSelectedPrefabs = {};
	protected int m_iSelectedPrefabsCount;
	protected static ref array<vector> s_aSelectedPositions = {};
	protected float m_fTime;
	protected float m_fTimeNext = -1;
	protected float m_fTimeRemaining;
	protected int m_iPrefabIndex = -1;
	protected SCR_EditorImageGeneratorPrefab m_CurrentPrefab;
	protected bool m_bIsScreenshotMade;
	
	//------------------------------------------------------------------------------------------------
	static SCR_EditorImageGeneratorEntity GetInstance()
	{
		return SCR_EditorImageGeneratorEntity.Cast(GetGame().FindEntity("SCR_EditorImageGeneratorEntity"));
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EditorImagePositionEntity FindSuitablePosition(array<EEditableEntityLabel> labels)
	{
		SCR_EditorImagePositionEntity position;
		for (int i = m_aPositions.Count() - 1; i >= 0; i--)
		{
			position = m_aPositions.GetValue(i);
			if (position.IsSuitable(labels) && IsPositionSelected(position))
				return position;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsPositionSelected(SCR_EditorImagePositionEntity position)
	{
		if (s_aSelectedPositions.IsEmpty())
			return true;
		
		for (int i = 0, count = s_aSelectedPositions.Count(); i < count; i++)
		{
			if (vector.DistanceSq(position.GetOrigin(), s_aSelectedPositions[i]) < 1)
				return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPosition(SCR_EditorImagePositionEntity position)
	{
		m_aPositions.Insert(position.GetPriority(), position);
	}
	
	//------------------------------------------------------------------------------------------------
	static void AddSelectedPosition(SCR_EditorImagePositionEntity position)
	{
		s_aSelectedPositions.Insert(position.GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Add selected assets in resource browser to get generated images.
	\Selected asset has to exist in worl Eden_AssetImage and has to have same labels set
	\Used for GetResourceBrowserSelection callback
	*/
	protected void AddSelectedPrefab(ResourceName prefab, string filePath)
	{
		string ext;
		FilePath.StripExtension(prefab, ext);
		if (ext != "et")
			return;
		
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return;
		
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(res);
		if (!editableEntitySource)
			return;
		
		SCR_EditableEntityUIInfo info = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
		if (!info)	
		{
			Print(string.Format("Prefab @\"%1\" does not have UI info defined in SCR_EditableEntityComponent!", prefab.GetPath()), LogLevel.WARNING);
			return;
		}
		
		ResourceName imagePath = info.GetImage();
		if (imagePath.IsEmpty())
		{
			Print(string.Format("Prefab @\"%1\" does not have image path defined in UI info of SCR_EditableEntityComponent!", prefab.GetPath()), LogLevel.WARNING);
			return;
		}
		
		array<EEditableEntityLabel> labels = {};
		info.GetEntityLabels(labels);
		SCR_EditorImagePositionEntity position = FindSuitablePosition(labels);
		if (!position)
		{
			if (s_aSelectedPositions.IsEmpty())
			{
				string labelsLine;
				foreach (int i, EEditableEntityLabel label: labels)
				{
					if (i > 0)
						labelsLine += ", ";
					
					labelsLine += typename.EnumToString(EEditableEntityLabel, label);
				}
				Print(string.Format("No suitable position found for @\"%1\" with labels %2!", prefab, labelsLine), LogLevel.WARNING);
			}
			return;
		}
		
		imagePath = FilePath.StripExtension(imagePath.GetPath());
		m_aSelectedPrefabs.Insert(new SCR_EditorImageGeneratorPrefab(prefab, imagePath, position));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RequestClose()
	{
		GetGame().RequestClose();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		if (m_iInit <= 1 && !SCR_Global.IsEditMode(this))
		{
			if (m_iInit == 1)
			{
				//--- Validate screen size
				if (!m_bDebugMode)
				{
					float screenW = GetGame().GetWorkspace().GetWidth();
					float screenH = GetGame().GetWorkspace().GetHeight();
					if (screenW != m_iImageWidth || screenH != m_iImageHeight)
					{
						Debug.Error2(Type().ToString(), string.Format("Cannot generate images! Current resolution is %1x%2, expected %3x%4!\n", screenW, screenH, m_iImageWidth, m_iImageHeight));
						GetGame().GetCallqueue().CallLater(RequestClose, false, 1);
					}
				}
				
				//--- Validate selected prefabs
				WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
				worldEditor.GetResourceBrowserSelection(AddSelectedPrefab, true);
				if (m_aSelectedPrefabs.IsEmpty())
				{
					Debug.Error2(Type().ToString(), "No suitable prefabs selected!\n");
					GetGame().GetCallqueue().CallLater(RequestClose, false, 1);
				}
				m_iSelectedPrefabsCount = m_aSelectedPrefabs.Count();
				
				
				//--- Set initial delay
				m_fTimeNext = m_fTime + 1;
				
				if (s_aSelectedPositions.IsEmpty())
					Print(string.Format("Initiating image generation for %1 prefab(s).", m_iSelectedPrefabsCount), LogLevel.DEBUG);
				else
					Print(string.Format("Initiating image generation for %1 prefab(s) on %2 pre-selected position(s).", m_iSelectedPrefabsCount, s_aSelectedPositions.Count()), LogLevel.DEBUG);
			}
			m_iInit++; //--- Wait one frame before evaluating screen dimensions, otherwise they will still be default 128x128
		}
		return m_iInit > 1;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Init())
			return;
		
		bool canContinue;
		if (m_fTimeNext != -1 || !m_CurrentPrefab)
		{
			canContinue = m_fTime > m_fTimeNext;
		}
		else if (Debug.KeyState(KeyCode.KC_SPACE))
		{
			Debug.ClearKey(KeyCode.KC_SPACE);
			canContinue = true;
		}
		
		if (canContinue)
		{
			//--- Take a screenshot of existing prefab
			if (m_CurrentPrefab && !m_bIsScreenshotMade)
			{
				m_bIsScreenshotMade = true;
				if (!m_bDebugMode)
				{
					string addonName = SCR_AddonTool.GetResourceLastAddon(m_CurrentPrefab.m_Prefab);
					addonName = SCR_AddonTool.ToFileSystem(addonName);
					System.MakeScreenshot(addonName + m_CurrentPrefab.m_ImagePath);
					Print(string.Format("Image of prefab '%1' at position '%2' saved to @\"%3.png\"", FilePath.StripPath(m_CurrentPrefab.m_Prefab), m_CurrentPrefab.m_Position.GetName(), m_CurrentPrefab.m_ImagePath), LogLevel.DEBUG);
					return; //--- Wait until next frame to give MakeScreenshot function enough time to actually capture the screen
				}
				else
				{
					Print(string.Format("SIMULATION: Image of prefab '%1' at position '%2' would be saved to @\"%3.png\"", FilePath.StripPath(m_CurrentPrefab.m_Prefab), m_CurrentPrefab.m_Position.GetName(), m_CurrentPrefab.m_ImagePath), LogLevel.VERBOSE);
				}
			}
			
			//--- Next prefab
			m_iPrefabIndex++;
			if (m_iPrefabIndex >= m_iSelectedPrefabsCount)
			{
				GetGame().GetCallqueue().CallLater(RequestClose, false, 1);
				return;
			}
			
			//--- Delete existing prefab
			if (m_CurrentPrefab)
				m_CurrentPrefab.m_Position.DeactivatePosition();
			
			//--- Create a new prefab
			m_bIsScreenshotMade = false;
			SCR_EditorImageGeneratorPrefab currentPrefab = m_aSelectedPrefabs[m_iPrefabIndex];
			if (currentPrefab.m_Position.ActivatePosition(currentPrefab.m_Prefab))
			{
				m_CurrentPrefab = currentPrefab;
				
				if (m_bManualProgress)
					m_fTimeNext = -1;
				else
					m_fTimeNext = m_fTime + currentPrefab.m_Position.GetDelay();
			}
			else
			{
				m_fTimeNext = 0;
			}
			
			m_fTimeRemaining = 0;
			for (int i = m_iPrefabIndex, count = m_aSelectedPrefabs.Count(); i < count; i++)
			{
				m_fTimeRemaining += m_aSelectedPrefabs[i].m_Position.GetDelay();
			}
		}
		
		//--- Show progress
		DbgUI.Begin("SCR_EditorImageGeneratorEntity");
		if (m_CurrentPrefab)
		{
			DbgUI.Text(string.Format("%1\n\n", m_CurrentPrefab.m_Prefab.GetPath()));
			DbgUI.Text(string.Format("Progress: %1 of %2", m_iPrefabIndex, m_iSelectedPrefabsCount));
			
			if (m_fTimeNext == -1)
				DbgUI.Text("Press 'Space' to take screenshot and continue to next prefab\n\n");
			else
				DbgUI.Text(string.Format("Estimated remaining time: %1", SCR_FormatHelper.GetTimeFormatting(m_fTimeRemaining, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS)));
		}
		else
		{
			DbgUI.Text("Initializing...");
		}
		DbgUI.End();
		
		m_fTime += timeSlice;
		m_fTimeRemaining -= timeSlice;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EditorImageGeneratorEntity(IEntitySource src, IEntity parent)
	{
		SetName("SCR_EditorImageGeneratorEntity");
		SetEventMask(EntityEvent.FRAME);
		
		if (SCR_Global.IsEditMode(this))
			s_aSelectedPositions.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_ALWAYS;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api.IsEntitySelected(api.EntityToSource(this)))
			return;
			
		int screenW = api.GetScreenWidth();
		int screenH = api.GetScreenHeight();
		
		string textCurrent = string.Format("Current resolution: %1x%2", screenW, screenH);
		string textTarget = string.Format("Target resolution: %1x%2", m_iImageWidth, m_iImageHeight);
		
		int color = Color.RED;
		if (screenW == m_iImageWidth && screenH == m_iImageHeight)
			color = Color.GREEN;
		else if (screenW == m_iImageWidth || screenH == m_iImageHeight)
			color = Color.ORANGE;
		
		DebugTextScreenSpace.Create(GetWorld(), textCurrent, DebugTextFlags.ONCE, 0, 0, 18, color, Color.BLACK);
		DebugTextScreenSpace.Create(GetWorld(), textTarget, DebugTextFlags.ONCE, 0, 18, 18, Color.WHITE, Color.BLACK);
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		GetWorld().GetBoundBox(min, max);
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Generate new image file with placeholder texture 
	//! Use when asset has no preview image
	protected void CreatePreviewImageFile()
	{
		string sourceFile = FilePath.ReplaceExtension(FilePath.StripPath(targetPath), m_sImagePlaceholderExt);
		if (sourceFile.IsEmpty() || m_sImagePlaceholderSource.IsEmpty())
			return;

		//--- Read source meta file
		string placeholderPath = m_ImagePlaceholder.GetPath();
		string absolutePath;
		Workbench.GetAbsolutePath(placeholderPath, absolutePath);

		//--- Create directory
		string imageDirectoryPath = FilePath.StripFileName(targetPath);
		if (!config.CreateDirectoryFor(imageDirectoryPath, addonName))
			return;

		//--- Copy texture source
		FileIO.CopyFile(m_sImagePlaceholderPath + m_sImagePlaceholderSource, addonName + FilePath.StripFileName(targetPath) + sourceFile);

		//--- Register the file
		Workbench.GetAbsolutePath(addonName + FilePath.StripFileName(targetPath) + sourceFile, absolutePath, false);
		MetaFile metaContainer = m_ResourceManager.RegisterResourceFile(absolutePath);
		if (metaContainer)
		{
			//--- Update meta file
			targetPath = metaContainer.GetResourceID();
			BaseContainerList configurations = metaContainer.GetObjectArray("Configurations");
			if (configurations)
			{
				configurations.Get(0).Set("ColorSpace", "ToSRGB"); //--- Assume PC is the first
				metaContainer.Save();
				Print(string.Format("Editable entity preview image ADDED: @\"%1\"", targetPath), LogLevel.DEBUG);
				return;
			}
		}
	}
	*/
#endif
};

class SCR_EditorImageGeneratorPrefab
{
	ResourceName m_Prefab;
	string m_ImagePath;
	SCR_EditorImagePositionEntity m_Position;
	
	void SCR_EditorImageGeneratorPrefab(ResourceName prefab, string imagePath, SCR_EditorImagePositionEntity position)
	{
		m_Prefab = prefab;
		m_ImagePath = imagePath;
		m_Position = position;
	}
};