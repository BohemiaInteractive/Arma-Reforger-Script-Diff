//! Class used to hold UI name.
//! Data are intentionally *READ ONLY*, because the class is often used on prefabs, not instances.
//! Instead of adding SetXXX() methods here, consider using specialised inherited class.
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_UIName
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString Name;
	
	//------------------------------------------------------------------------------------------------
	//! Get The unformatted Name from SCR_HintUIInfo instance.
	//! \return LocalizedString Name set in SCR_HintUIInfo instance.
	LocalizedString GetUnformattedName()
	{
		return Name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change the Name from SCR_HintUIInfo instance.
	//! Call SCR_HintManagerComponent.Refresh() to update shown hint to display changed text.
	//! \param LocalizedString text to which name should be updated to.
	void SetName(LocalizedString name)
	{
		Name = name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get name.
	//! When using it to fill TextWidget, use SetNameTo() if possible.
	//! \return Name text
	LocalizedString GetName()
	{
		return Name;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the info has a name defined.
	//! \return True when the name is defined
	bool HasName()
	{
		return !GetName().IsEmpty();
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----	
// This class is meant to hold data, not manipulate widgets! Such methods should be in SCR_WidgetTools or SCR_WidgetHelper. In this case, I would expect whatever component is handling the layout to make sure the text widget exists, so this method is not needed
	
	//------------------------------------------------------------------------------------------------
	//! Set name to given text widget.
	//! When possible, use this function instead of retrieving the name using GetName() and setting it manually.
	//! Custom UI info classes may be using parameters which would not be applied otherwise.
	//! \param textWidget Target text widget
	//! \return True when the name was set
	bool SetNameTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		textWidget.SetText(GetName());
		return true;
	}

//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//! Print out contents of this UI info
	//! \param prefix
	//! \param logLevel
	void Log(string prefix = string.Empty, LogLevel logLevel = LogLevel.VERBOSE)
	{
		Print(string.Format(prefix + "%1: \"%2\"", Type(), Name), logLevel);
	}
	
	//--- Protected, to be overridden and/or made public by inherited classes
	protected void CopyFrom(SCR_UIName source)
	{
		if (source)
			Name = source.Name;
	}
}
