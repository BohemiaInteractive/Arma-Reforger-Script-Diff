[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_BlockUIInfo : SCR_UIInfo
{
	[Attribute()]
	protected ref array<ref SCR_SubBlockUIName> m_aDescriptionBlocks;
	
	override bool HasDescription()
	{
		return super.HasDescription() || m_aDescriptionBlocks && !m_aDescriptionBlocks.IsEmpty();
	}
	
	override LocalizedString GetDescription()
	{
		LocalizedString description = super.GetDescription();
		if (!description.IsEmpty())
			description += "<br/>";
		
		if (m_aDescriptionBlocks)
		{
			for (int i, count = m_aDescriptionBlocks.Count(); i < count; i++)
			{
				if (m_aDescriptionBlocks[i].HasName())
				{
					if (m_aDescriptionBlocks[i].IsInline() || description.IsEmpty())
						description += m_aDescriptionBlocks[i].GetName() + "<br/>";
					else
						description += "<br/>" + m_aDescriptionBlocks[i].GetName();
				}
			}
		}
		return description + "<br/>";
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Name of selected description block without formatting (raw).
	\param Int - index of Item in Description Block array
	\return Unformatted Name set in selected Description block. - Empty if index out of bounce.
	*/
	LocalizedString GetUnformattedDescriptionBlockName(int index)
	{
		if (index > m_aDescriptionBlocks.Count())
			return string.Empty;
		
		return m_aDescriptionBlocks[index].Name;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Changes the Name (text) of selected Description Block.
	Call SCR_HintManagerComponent.Refresh() to update shown hint to display changed text.
	\param Int - index of Item in Description Block array
	\param LocalizedString - string that is going to be inserted in the Name. (Needs to be Localized)
	*/
	void SetDescriptionBlockName(int index, LocalizedString text)
	{
		if (index > m_aDescriptionBlocks.Count())
			return;

		Name = text;
	}
};
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_SubBlockUIName: SCR_UIName
{
	[Attribute()]
	protected bool m_bInline;
	
	bool IsInline()
	{
		return m_bInline;
	}
};
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_BulletPointBlockUIName: SCR_SubBlockUIName
{	
	override LocalizedString GetName()
	{
		return "<image set='{73BAE6966DBC17CB}UI/Imagesets/Hint/Hint.imageset' name='Bullet' scale='1'/>" + " " + super.GetName(); //--- ToDo: Don't hard-code
	}
};
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_TipBlockUIName: SCR_SubBlockUIName
{
	override LocalizedString GetName()
	{
		return "<color rgba='255,255,255,160'><image set='{73BAE6966DBC17CB}UI/Imagesets/Hint/Hint.imageset' name='Tip' scale='1'/>" + " " + super.GetName() + "</color>"; //--- ToDo: Don't hard-code
	}
};
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_DeviceBlockUIName: SCR_SubBlockUIName
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Any", "0"), ParamEnum("Mouse & Keyboard", "1"), ParamEnum("Gamepad", "2")})]
	protected int m_iDevice;
	
	override bool HasName()
	{
		bool isInputMatch = true;
		switch (m_iDevice)
		{
			case 1: isInputMatch = GetGame().GetInputManager().IsUsingMouseAndKeyboard(); break;
			case 2: isInputMatch = !GetGame().GetInputManager().IsUsingMouseAndKeyboard(); break;
		}
		return isInputMatch;
	}
};
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_ActionBlockUIName: SCR_DeviceBlockUIName
{
	[Attribute()]
	protected string m_sActionName;
	
	protected const string m_sDelimiter = "<br/>";
	
	override LocalizedString GetName()
	{
		//--- ToDo: Turn color conversion to general function
		Color sRGBA = Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt());	

		return super.GetName() + m_sDelimiter + m_sDelimiter + string.Format("<color rgba=%2><action name='%1' scale='1.7'/></color>", m_sActionName, UIColors.FormatColor(sRGBA)) + m_sDelimiter;
	}
};
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_KeyBlockUIName: SCR_DeviceBlockUIName
{
	[Attribute()]
	protected ref array<ref SCR_KeyBlockEntry> m_aKeys;
	
	protected const LocalizedString m_sHoldModifier = "#ENF-HoldModifier";
	protected const LocalizedString m_sDoubleModifier = "#ENF-DoubleModifier";
	protected const LocalizedString m_sComboModifier = "#ENF-ComboModifier";
	protected const LocalizedString m_sEllipsisModifier = "..."; //--- ToDo: Localize
	protected const LocalizedString m_sDelimiter = "<br/>"; //--- ToDo: Localize
	
	override LocalizedString GetName()
	{
		//--- ToDo: Turn color conversion to general function
		Color sRGBA = Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt());
		
		LocalizedString keys;
		foreach (int i, SCR_KeyBlockEntry entry: m_aKeys)
		{
			if (i > 0)
			{
				switch (entry.m_iDelimiter)
				{
					case 0: keys += m_sComboModifier; break;
					case 1: keys += m_sEllipsisModifier; break;
				}
			}
			
			switch (entry.m_iModifier)
			{
				case 1: keys += m_sHoldModifier; break;
				case 2: keys += m_sDoubleModifier; break;
			}
			keys += string.Format("<%2 name='%1' scale='1.7'/>", entry.m_sKey, entry.m_Tag);
		}
		return super.GetName() + m_sDelimiter + m_sDelimiter + string.Format("<color rgba='%2'>%1</color>", keys, UIColors.FormatColor(sRGBA)) + m_sDelimiter;	
	}
};
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sKey")]
class SCR_KeyBlockEntry
{
	[Attribute()]
	string m_sKey;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum(" + ", "0"), ParamEnum("...", "1")})]
	int m_iDelimiter;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("None", "0"), ParamEnum("Hold", "1"), ParamEnum("Double", "2")})]
	int m_iModifier;
	
	[Attribute("key", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("key", "key"), ParamEnum("action", "action")})]
	string m_Tag;
};

[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_SimpleTagBlockUIName: SCR_SubBlockUIName
{
	[Attribute("0", uiwidget: UIWidgets.Flags, enums: { ParamEnum("<p>", "1"), ParamEnum("<b>", "2"), ParamEnum("<i>", "4"), ParamEnum("<h1>", "8"), ParamEnum("<h2>", "16") })]
	protected int m_iTags;
	
	override LocalizedString GetName()
	{
		LocalizedString text;
		
		if (m_iTags & 1) text += "<p>";
		if (m_iTags & 2) text += "<b>";
		if (m_iTags & 4) text += "<i>";
		if (m_iTags & 8) text += "<h1>";
		if (m_iTags & 16) text += "<h2>";
		
		text += super.GetName();
		
		if (m_iTags & 1) text += "</p>";
		if (m_iTags & 2) text += "</b>";
		if (m_iTags & 4) text += "</i>";
		if (m_iTags & 8) text += "</h1>";
		if (m_iTags & 16) text += "</h2>";
		
		return text;
	}
	
	override bool IsInline()
	{
		return super.IsInline() || m_iTags & 1; //--- <p> is always inline, we don't want extra <br/> there
	}
};

[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_ImageBlockUIName: SCR_SubBlockUIName
{
	[Attribute(params: "imageset", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sImageSet;
	
	[Attribute(desc: "This defines name of the image in the set.")]
	protected string m_sImage;
	
	[Attribute(desc: "Defines the scale of the Icon.", defvalue: "1")]
	protected float m_fScale;
	
	[Attribute(defvalue: "1 1 1 1")]
	protected ref Color m_fColor;

	//------------------------------------------------------------------------------------------------
	override LocalizedString GetName()
	{
		if (m_sImage == string.Empty)
		{
			Print("HINT SYSTEM: No Icon name defined! Check 'Image'!", LogLevel.WARNING);
			return string.Empty;
		}
		
		return string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(m_fColor), m_sImageSet, m_sImage, m_fScale) + "</br>";
	}
}

/*
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_TagBlockUIName: SCR_SubBlockUIName
{
	[Attribute()]
	protected ref array<ref SCR_BaseTagBlockEntry> m_aTags;
	
	override LocalizedString GetName()
	{
		LocalizedString text;
		int tagCount = m_aTags.Count();
		for (int i; i < tagCount; i++)
		{
			text += m_aTags[i].GetStart();
		}
		
		text += super.GetName();
		
		for (int i; i < tagCount; i++)
		{
			text += m_aTags[i].GetEnd();
		}
		return text;
	}
}

[BaseContainerProps()]
class SCR_BaseTagBlockEntry
{
	string GetStart();
	string GetEnd();
}
[BaseContainerProps()]
class SCR_CustomTagBlockEntry: SCR_BaseTagBlockEntry
{
	[Attribute()]
	protected string m_sStart;
	
	[Attribute()]
	protected string m_sEnd;
	
	override string GetStart()	{ return m_sStart; }
	override string GetEnd()	{ return m_sEnd; }
}
[BaseContainerProps()]
class SCR_BoldTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<b>"; }
	override string GetEnd()	{ return "</b>"; }
}
[BaseContainerProps()]
class SCR_ItalicTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<i>"; }
	override string GetEnd()	{ return "</i>"; }
}
[BaseContainerProps()]
class SCR_HeaderTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<h1>"; }
	override string GetEnd()	{ return "</h1>"; }
}
[BaseContainerProps()]
class SCR_SubHeaderTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<h2>"; }
	override string GetEnd()	{ return "</h2>"; }
}
*/
