typedef func MenuItemCallback;
void MenuItemCallback(MenuBase menu = null, string menuItem = "", bool changed = false, bool finished = false); 

enum DialogPriority
{
	INFORMATIVE,
	WARNING,
	CRITICAL	
}

enum DialogResult
{
	PENDING,
	OK,
	YES,
	NO,
	CANCEL,
}

enum ScriptMenuPresetEnum
{
	
}

enum GameLibMenusEnum : ScriptMenuPresetEnum
{
	
}

class MenuBindAttribute
{
	string m_MenuItemName;
	
	void MenuBindAttribute(string menuItemName = "")
	{
		m_MenuItemName = menuItemName;
	}
}

class MessageBox: MenuBase
{
	[MenuBindAttribute()]
	void Ok()
	{
		Close();
	}
}
