// This component handles visualization of weapon selection menu
// 
//------------------------------------------------------------------------------------------------

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Missing a prefix

class EditBoxFilterComponent : ScriptedWidgetComponent
{

//---- REFACTOR NOTE END ----
	
	[Attribute("500", UIWidgets.EditBox, "Set maximum character limit of the editbox")]
	private int m_iCharacterLimit;

	[Attribute("true", UIWidgets.CheckBox, "Allow punctuation character, like space, brackets, ,./@#$%^ etc")]
	private bool m_bPunctuation;
	
	[Attribute("true", UIWidgets.CheckBox, "Allow numerical characters")]
	private bool m_bNumbers;
	
	[Attribute("true", UIWidgets.CheckBox, "Allow basic set of characters")]
	private bool m_bASCIIchars;
	
	[Attribute("true", UIWidgets.CheckBox, "Allow UTF-8 multibyte symbols - including special symbols and diacritics")]
	private bool m_bUTFMultibyte;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----	
// Aren't these mutually exclusive? Also filling them this way might make reading the values rather annoying. How are different alphabets handled?
	
	[Attribute("", UIWidgets.EditBox, "Explicitly blacklisted characters, write without spaces or or commas")]
	private string m_sCharBlacklist;
	
	[Attribute("", UIWidgets.EditBox, "Explicitly whitelisted characters, write without spaces or or commas")]
	private string m_sCharWhitelist;

//---- REFACTOR NOTE END ----
	
	private ref array<int> m_aBlacklist = new array<int>;
	private ref array<int> m_aWhitelist = new array<int>;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Only works with single line edit box, because the widgets do not share a common parent, even though in the end they should be functionally almost identical
	
	private EditBoxWidget m_wEditBox;
	protected MultilineEditBoxWidget m_wMultilineEditBox;
	
//---- REFACTOR NOTE END ----
	
	private int m_iCharacterCount;
	
	protected bool m_bLastInputValid = true;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Untyped invokers
	
	ref ScriptInvoker m_OnInvalidInput = new ScriptInvoker();
	ref ScriptInvoker m_OnTextTooLong = new ScriptInvoker();
	ref ScriptInvoker m_OnValidInput = new ScriptInvoker();
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wEditBox = EditBoxWidget.Cast(w);
		if (!m_wEditBox)
			m_wMultilineEditBox = MultilineEditBoxWidget.Cast(w);
		
		if (!m_wEditBox || !m_wMultilineEditBox)
		{
			Print("EditBoxFilterComponent used on invalid widget type.", LogLevel.WARNING);
			return;
		}
		
		int blacklistSize = m_sCharBlacklist.Length();
		for (int i = 0; i < blacklistSize; i++)
		{
			int char = m_sCharBlacklist.Get(i).ToAscii();
			m_aBlacklist.Insert(char);
		}
		
		int whitelistSize = m_sCharWhitelist.Length();
		for (int i = 0; i < whitelistSize; i++)
		{
			int char = m_sCharWhitelist.Get(i).ToAscii();
			m_aWhitelist.Insert(char);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		m_aBlacklist = null;
		m_aWhitelist = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnChange(Widget w, bool finished)
	{
		bool validInput = true;
		if (!m_wEditBox)
		{
			if (m_wMultilineEditBox)
				return OnChangeMultiline();
			
			return false;
		}
		
		string text = m_wEditBox.GetText();
		int length = text.Length();
		string shortText = text;
		
		if (length > 0)
		{
			if (length > m_iCharacterLimit) 
			{
				shortText = text.Substring(0, m_iCharacterLimit);
				m_wEditBox.SetText(shortText);
				
				m_OnTextTooLong.Invoke();
				validInput = false;
			}
			else
			{
				// Find invalid characters 
				shortText = FilterSymbolsFromText(text);
				
				// Invoke wrong input if texts are not same
				if (shortText != text)
				{
					m_wEditBox.SetText(shortText);
					m_OnInvalidInput.Invoke();
					validInput = false;
				}
			}
		}
		
		m_iCharacterCount = length;
		
		if (validInput)
			m_OnValidInput.Invoke();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnChangeMultiline()
	{
		bool validInput = true;
		if (!m_wMultilineEditBox)
			return false;
		
		string text = m_wMultilineEditBox.GetText();
		int length = text.Length();
		string shortText = text;
		
		if (length > 0)
		{
			if (length > m_iCharacterLimit) 
			{
				shortText = text.Substring(0, m_iCharacterLimit);
				m_wMultilineEditBox.SetText(shortText);
				
				m_OnTextTooLong.Invoke();
				validInput = false;
			}
			else
			{
				// Find invalid characters 
				shortText = FilterSymbolsFromText(text);
				
				// Invoke wrong input if texts are not same
				if (shortText != text)
				{
					m_wMultilineEditBox.SetText(shortText);
					m_OnInvalidInput.Invoke();
					validInput = false;
				}
			}
		}
		
		m_iCharacterCount = length;
		
		if (validInput)
			m_OnValidInput.Invoke();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetNumLimitOfCharacters()
	{
		return m_iCharacterLimit;
	}
	
	
	//------------------------------------------------------------------------------------------------
	private bool FilterSymbol(string char)
	{
		bool found = false;
		int asciiCode = char.ToAscii();
				
		if (asciiCode < 0)
		{
			if (m_bUTFMultibyte)
				found = true;
		}
		else if (asciiCode >= 32 && asciiCode <= 47)
		{
			if (m_bPunctuation)
				found = true;
		}
		else if (asciiCode <= 57)
		{
			if (m_bNumbers)
				found = true;
		}
		else
		{
			if (m_bASCIIchars)
				found = true;
		}

		bool valid;
		if (found)
		{
			// Apply blacklist
			valid = !SearchWhitelist(asciiCode, true);
		}
		else
		{
			// Apply whitelist
			valid = SearchWhitelist(asciiCode, false);
		}
		
		return valid;
	}
	
	//------------------------------------------------------------------------------------------------
	private bool SearchWhitelist(int char, bool blacklist)
	{
		array<int> list;
		if (blacklist)
			list = m_aBlacklist;
		else
			list = m_aWhitelist;

		if (list)
		{
			foreach (int val : list)
			{
				if (val == char)
					return true;
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get positino of last change 
	//! Only do simple comparision since it's checking for not allowed inputs 
	protected string FilterSymbolsFromText(string text)
	{
		string newText = string.Empty;
		
		// Compare text 
		for (int i = 0; i < text.Length(); i++)
		{
			string char = text[i];
			
			m_bLastInputValid = FilterSymbol(char);
			if (!m_bLastInputValid)
				continue;
			
			newText += char;
		}

		return newText;
	}
	
	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetCharacterLimit(int limit)
	{
		m_iCharacterLimit = limit;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPunctuation(bool enabled)
	{
		m_bPunctuation = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNumbers(bool enabled)
	{
		m_bNumbers = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetASCIIchars(bool enabled)
	{
		m_bASCIIchars = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetUTFMultibyte(bool enabled)
	{
		m_bUTFMultibyte = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCharBlacklist(string list)
	{
		m_aBlacklist.Clear();
		
		int blacklistSize = list.Length();
		for (int i = 0; i < blacklistSize; i++)
		{
			int char = list.Get(i).ToAscii();
			m_aBlacklist.Insert(char);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCharWhiteList(string list)
	{
		m_aWhitelist.Clear();
		
		int whitelistSize = list.Length();
		for (int i = 0; i < whitelistSize; i++)
		{
			int char = list.Get(i).ToAscii();
			m_aWhitelist.Insert(char);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCharacterLimit(string list)
	{
		m_sCharWhitelist = list;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLastInputValid()
	{
		return m_bLastInputValid;
	}
};