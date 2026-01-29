#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Bookmark  1", category: "Bookmarks", shortcut: "Ctrl+1", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin1 : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	protected int GetBookmarkIndex()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);

		array<ResourceName> selection = {};
		resourceManager.GetResourceBrowserSelection(selection.Insert, true);

		int bookmarkIndex = GetBookmarkIndex();
		SCR_BookmarkSettingsPlugin settings = SCR_BookmarkSettingsPlugin.Cast(Workbench.GetModule(ResourceManager).GetPlugin(SCR_BookmarkSettingsPlugin));
		ResourceName bookmarkFile = settings.GetBookmarkFile(bookmarkIndex);

		// 0 = open
		// 1 = set/replace
		// -1 = error
		int action;
		if (selection.IsEmpty() || bookmarkFile == selection[0])
		{
			action = 0;	// open
		}
		else if (bookmarkFile) // !.IsEmpty()
		{
			SCR_BookmarkPluginPrompt prompt = new SCR_BookmarkPluginPrompt();
			if (Workbench.ScriptDialog("Open or Replace", string.Format("Do you want to open the existing bookmark '%1'\nor replace it (bookmark %2) by the selected file '%3'?", FilePath.StripPath(bookmarkFile), bookmarkIndex, FilePath.StripPath(selection[0])), prompt) == 0)
				return; // cancel

			action = prompt.GetSelectedAction();
		}
		else
		{
			action = 1;	// set/replace
		}

		switch (action)
		{
			case -1:	// error - cannot happen
				return;

			case 0:		// open
				if (bookmarkFile) // !.IsEmpty()
				{
					PrintFormat("Opening bookmark %1: @\"%2\"", bookmarkIndex, bookmarkFile, level: LogLevel.NORMAL);
					resourceManager.SetOpenedResource(bookmarkFile);
				}
				else
				{
					PrintFormat("Bookmark %1 is not defined", bookmarkIndex, level: LogLevel.NORMAL);
				}

				break;

			case 1:		// set/replace
				bookmarkFile = selection[0];
				if (settings.SetBookmarkFile(bookmarkIndex, bookmarkFile))
					PrintFormat("Bookmark %1 now points to @\"%2\"", bookmarkIndex, bookmarkFile.GetPath(), level: LogLevel.NORMAL);
				else
					PrintFormat("Cannot set bookmark's value, the provided index is invalid (%1)", bookmarkIndex, level: LogLevel.ERROR);

				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool Close()
	{
		return true;
	}
}

[WorkbenchPluginAttribute(name: "Bookmark  2", category: "Bookmarks", shortcut: "Ctrl+2", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin2 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 2; } }

[WorkbenchPluginAttribute(name: "Bookmark  3", category: "Bookmarks", shortcut: "Ctrl+3", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin3 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 3; } }

[WorkbenchPluginAttribute(name: "Bookmark  4", category: "Bookmarks", shortcut: "Ctrl+4", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin4 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 4; } }

[WorkbenchPluginAttribute(name: "Bookmark  5", category: "Bookmarks", shortcut: "Ctrl+5", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin5 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 5; } }

[WorkbenchPluginAttribute(name: "Bookmark  6", category: "Bookmarks", shortcut: "Ctrl+6", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin6 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 6; } }

[WorkbenchPluginAttribute(name: "Bookmark  7", category: "Bookmarks", shortcut: "Ctrl+7", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin7 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 7; } }

[WorkbenchPluginAttribute(name: "Bookmark  8", category: "Bookmarks", shortcut: "Ctrl+8", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin8 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 8; } }

[WorkbenchPluginAttribute(name: "Bookmark  9", category: "Bookmarks", shortcut: "Ctrl+9", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin9 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 9; } }

[WorkbenchPluginAttribute(name: "Bookmark 10", category: "Bookmarks", shortcut: "Ctrl+0", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkPlugin0 : SCR_BookmarkPlugin1 { override protected int GetBookmarkIndex() { return 10; } }

class SCR_BookmarkPluginPrompt
{
	protected int m_iAction = -1;

	//------------------------------------------------------------------------------------------------
	int GetSelectedAction()
	{
		return m_iAction;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Open", true)]
	protected int BtnOpen()
	{
		m_iAction = 0;
		return 1; // action selected
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Replace")]
	protected int BtnReplace()
	{
		m_iAction = 1;
		return 1; // action selected
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int BtnCancel()
	{
		m_iAction = -1;
		return 0; // choice cancelled - same as clicking "close" on the window
	}
}

[WorkbenchPluginAttribute(name: "Bookmarks Configuration", description: "Configure bookmarks", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF02E)]
class SCR_BookmarkSettingsPlugin : WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_1; // underscore to have a space between "bookmark" and "1"

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_2;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_3;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_4;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_5;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_6;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_7;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_8;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_9;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Bookmarks")]
	protected ResourceName m_sBookmark_10;

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		array<ResourceName> save = { m_sBookmark_1, m_sBookmark_2, m_sBookmark_3, m_sBookmark_4, m_sBookmark_5, m_sBookmark_6, m_sBookmark_7, m_sBookmark_8, m_sBookmark_9, m_sBookmark_10 };
		if (Workbench.ScriptDialog("Configure Bookmarks", "Select which files will be opened upon pressing bookmarks's shortcuts (Ctrl+#).", this) == 0)
		{
			// Cancel = revert values

			m_sBookmark_1 = save[0];
			m_sBookmark_2 = save[1];
			m_sBookmark_3 = save[2];
			m_sBookmark_4 = save[3];
			m_sBookmark_5 = save[4];
			m_sBookmark_6 = save[5];
			m_sBookmark_7 = save[6];
			m_sBookmark_8 = save[7];
			m_sBookmark_9 = save[8];
			m_sBookmark_10 = save[9];
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] bookmarkIndex a value in range 1..10
	//! \return bookmark's resource name or empty if invalid value provided
	ResourceName GetBookmarkFile(int bookmarkIndex)
	{
		switch (bookmarkIndex)
		{
			case 1: return m_sBookmark_1;
			case 2: return m_sBookmark_2;
			case 3: return m_sBookmark_3;
			case 4: return m_sBookmark_4;
			case 5: return m_sBookmark_5;
			case 6: return m_sBookmark_6;
			case 7: return m_sBookmark_7;
			case 8: return m_sBookmark_8;
			case 9: return m_sBookmark_9;
			case 10: return m_sBookmark_10;
		}

		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] bookmarkIndex a value in range 1..10
	//! \param[in] newBookmark the new bookmark value - can be empty
	//! \return true on success, false otherwise (wrong bookmark index provided)
	bool SetBookmarkFile(int bookmarkIndex, ResourceName newBookmark)
	{
		switch (bookmarkIndex)
		{
			case 1: m_sBookmark_1 = newBookmark; return true;
			case 2: m_sBookmark_2 = newBookmark; return true;
			case 3: m_sBookmark_3 = newBookmark; return true;
			case 4: m_sBookmark_4 = newBookmark; return true;
			case 5: m_sBookmark_5 = newBookmark; return true;
			case 6: m_sBookmark_6 = newBookmark; return true;
			case 7: m_sBookmark_7 = newBookmark; return true;
			case 8: m_sBookmark_8 = newBookmark; return true;
			case 9: m_sBookmark_9 = newBookmark; return true;
			case 10: m_sBookmark_10 = newBookmark; return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Save")]
	protected int BtnSave()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int BtnCancel()
	{
		return 0;
	}
}
#endif // WORKBENCH
