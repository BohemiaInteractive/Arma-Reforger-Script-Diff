//! @ingroup ManualCamera

//! Parent class from which all SCR_ManualCamera components inherit.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_BaseManualCameraComponent
{
	[Attribute("1")]
	private bool m_bEnabled;
	
	[Attribute("", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EManualCameraFlag))]
	private EManualCameraFlag m_FlagsWhitelist;
	
	[Attribute("", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EManualCameraFlag))]
	private EManualCameraFlag m_FlagsBlacklist;
	
	[Attribute("", UIWidgets.Auto, "Component processing priority. Higher value = higher priority")]
	protected int m_iPriority;
	
	protected SCR_ManualCamera m_Camera;
	protected InputManager m_InputManager;

	//! @name Override Methods
	//! Methods to be overridden by child classes.
	//! @{
	//------------------------------------------------------------------------------------------------
	//! Evaluate the component every frame.
	//! \param[in] param Object carrying camera properties
	void EOnCameraFrame(SCR_ManualCameraParam param);

	//------------------------------------------------------------------------------------------------
	//! Initialise the component.
	//! \return True if the component should be evaluated every frame
	bool EOnCameraInit();

	//------------------------------------------------------------------------------------------------
	//! Terminate the component.
	void EOnCameraExit();

	//------------------------------------------------------------------------------------------------
	//! Camera value are reset.
	void EOnCameraReset();

	//------------------------------------------------------------------------------------------------
	//! Terminate the component.
	//! \param[in] attached
	//! \param[in] parent
	void EOnCameraParentChange(bool attached, IEntity parent);

	//------------------------------------------------------------------------------------------------
	//! Save persistent data.
	//! \param[in] data
	void EOnCameraSave(SCR_ManualCameraComponentSave data);

	//------------------------------------------------------------------------------------------------
	//! Load persistent data.
	//! \param[in] data
	void EOnCameraLoad(SCR_ManualCameraComponentSave data);
	//! @}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Custom functions

	//------------------------------------------------------------------------------------------------
	protected SCR_ManualCamera GetCameraEntity()
	{
		return m_Camera;
	}

	//------------------------------------------------------------------------------------------------
	protected InputManager GetInputManager()
	{
		return m_InputManager;
	}

	//------------------------------------------------------------------------------------------------
	protected vector CoordToCamera(vector pos)
	{
		if (!m_Camera.GetParent())
			return pos;

		return m_Camera.GetParent().CoordToLocal(pos);
	}

	//------------------------------------------------------------------------------------------------
	protected vector CoordFromCamera(vector pos)
	{
		if (!m_Camera.GetParent())
			return pos;

		return m_Camera.GetParent().CoordToParent(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Enable the component. EOnCameraFrame is not evaluated for disabled components.
	//! \param[in] True to enable the component, false to disable it
	void SetEnabled(bool enabled)
	{
		m_bEnabled = enabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the component is enabled.
	//! \return True when enabled
	int IsEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Get evaluation priority.
	//! \return Priority. Higher value = higher priority.
	int GetPriority()
	{
		return m_iPriority;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the required flag is active.
	//! \param[in] Camera flag
	//! \return True if the camera flag contains the required flag
	bool HasFlag(EManualCameraFlag flag)
	{
		return m_bEnabled && (flag & m_FlagsWhitelist) == m_FlagsWhitelist && (flag & m_FlagsBlacklist) == 0;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] camera
	//! \return
	bool InitBase(SCR_ManualCamera camera)
	{
		m_Camera = camera;
		m_InputManager = GetGame().GetInputManager();
		return EOnCameraInit();
	}
}

class SCR_BaseManualCameraComponentTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_", "");
		title.Replace("ManualCameraComponent", "");
		
		EManualCameraFlag flagsWhitelist = 0;
		source.Get("m_FlagsWhitelist", flagsWhitelist);
		string textWhitelist;
		if (flagsWhitelist != 0)
		{
			array<int> bitValues = {};
			SCR_Enum.BitToIntArray(flagsWhitelist, bitValues);
			foreach (int i, int value: bitValues)
			{
				if (i != 0)
					textWhitelist += " && ";

				textWhitelist += typename.EnumToString(EManualCameraFlag, value);
			}
		}
		
		EManualCameraFlag flagsBlacklist = 0;
		source.Get("m_FlagsBlacklist", flagsBlacklist);
		string textBlacklist;
		if (flagsBlacklist != 0)
		{
			if (flagsWhitelist != 0)
				textBlacklist += " && ";

			array<int> bitValues = {};
			SCR_Enum.BitToIntArray(flagsBlacklist, bitValues);
			foreach (int i, int value: bitValues)
			{
				if (i != 0)
					textBlacklist += " && ";

				textBlacklist += "!" + typename.EnumToString(EManualCameraFlag, value);
			}
		}
		
		if (flagsWhitelist != 0 || flagsBlacklist != 0)
			title = string.Format("[%1] %2", textWhitelist + textBlacklist, title);
		
		int priority = 0;
		if (source.Get("m_iPriority", priority))
			title = string.Format("%1: %2", priority.ToString(2), title);

		bool enabled = true;
		if (source.Get("m_bEnabled", enabled) && !enabled)
			title = string.Format("// %1", title);
		
		return true;
	}
}
