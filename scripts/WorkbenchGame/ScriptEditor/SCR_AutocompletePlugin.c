#ifdef WORKBENCH
//! This plugin allows for autocompletion for common structure / code excerpts and boilerplate code.
//!
//! Features:
//! if → adds an if structure
//! ife → adds an if-else structure
//! for → adds a for loop
//! forr → adds a for loop, reversed (i--)
//! foreach → adds a foreach loop
//! foreachi → adds a foreach loop with i index
//! switch → adds a switch case with one case and default
//! while → adds a while loop
//! class → adds a class with its constructor structure
//! method/func → adds a method with separator and Doxygen doc skeleton
//!
//! ctor → adds a constructor
//! dtor → adds a destructor
//! findcomp → adds SCR_ComponentClass component = SCR_ComponentClass.Cast(entity.FindComponent(SCR_ComponentClass));
//! print → adds a Print with normal LogLevel
//! Type something = new → Type something = new Type();
//!
//! nullcheck on cast (e.g "Class a = Class.Cast(b)") → adds an "if (!a) return;" check below if not present
//! validity check on Resource.Load → adds an "if (!resource.IsValid())" check below (or fixes, in case of "if (!resource)") if not present
//! potential attribute (e.g "ResourceName m_sResourceName;") → adds an [Attribute()] if not present
//! adds missing or incorrect LogLevel to Print/PrintFormat
//! adds missing () to incomplete instantiation
[WorkbenchPluginAttribute(name: "Autocomplete", description: "Helps autocompleting keywords, adding [Attribute()] decorators, cast nullchecks etc.", shortcut: "Ctrl+Return" /* "Ctrl+Space" */, wbModules: { "ScriptEditor" }, awesomeFontCode: 0xE2CA)]
class SCR_AutocompletePlugin : WorkbenchPlugin
{
	/*
		Category: General
	*/

	[Attribute(defvalue: "1", desc: "Add class constructor on \"" + CONSTRUCTOR_KEYWORD + "\" keyword", category: "General")]
	protected bool m_bAddConstructor;

	[Attribute(defvalue: "1", desc: "Add class destructor on \"" + DESTRUCTOR_KEYWORD + "\" keyword", category: "General")]
	protected bool m_bAddDestructor;

	[Attribute(defvalue: "1", desc: "Autocomplete \" = new\" statements, e.g:"
		+ "\n- array<int> myArray = new → array<int> myArray = {};"
		+ "\n- array<ref SCR_MyClass> myArray = new → array<ref SCR_MyClass> myArray = {};"
		+ "\n- map<int, ref SCR_MySuperClass> myMap = new → map<int, ref SCR_MySuperClass> myMap = new map<int, ref SCR_MySuperClass>();"
		+ "\n- bool m_bMyVar = new → bool m_bMyVar;", category: "General")]
	protected bool m_bAddInstantiation;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "On which line ending does instantion completion triggers", enums: SCR_ParamEnumArray.FromString("Both \"= new\" and \"=\";\"= new\";\"=\" only"), category: "General")]
	protected int m_iInstantiationTrigger;

	[Attribute(defvalue: "1", desc:"Fix \"new\" statements, e.g:"
		+ "\n- SCR_MyClass instance = new SCR_MyClass; → SCR_MyClass instance = new SCR_MyClass();"
		+ "\n- array<string> instance = new array<string>(); → array<string> instance = {};", category: "General")]
	protected bool m_bFixInstantiation;

	[Attribute(defvalue: "1", desc: "Add a nullcheck below a \"Class a = Class.Cast(b)\" statement if not present", category: "General")]
	protected bool m_bAddCastNullcheck;

	[Attribute(defvalue: "1", desc: "Add (or fix) a validity check below a \"Resource.Load\" statement if not present", category: "General")]
	protected bool m_bAddOrFixResourceLoadValidityCheck;

	[Attribute(defvalue: LogLevel.NORMAL.ToString(), desc: "Add a LogLevel to a Print or PrintFormat if not present", enums: {
		new ParamEnum("Disabled", "-1", "Do not add log level to Print/PrintFormat missing it"),
		new ParamEnum("LogLevel.NORMAL", LogLevel.NORMAL.ToString()),
		new ParamEnum("LogLevel.WARNING", LogLevel.WARNING.ToString()),
		new ParamEnum("LogLevel.ERROR", LogLevel.ERROR.ToString()),
		new ParamEnum("LogLevel.FATAL", LogLevel.FATAL.ToString()),
	}, uiwidget: UIWidgets.ComboBox, category: "General")]
	protected LogLevel m_eAddPrintLogLevel;

//	[Attribute(defvalue: "1", desc: "Keep comment indentation on commented keyword's usage (e.g \" ctor		// TODO\")")]
	protected bool m_bKeepCommentIndentation = true;

	/*
		Category: Keywords
	*/

	[Attribute(defvalue: "1", desc: "Use the tool's keyword list below", category: "Keywords")]
	protected bool m_bUseToolKeywords;

	[Attribute(desc: "Tool-defined keyword-replacement pairs - editable and resettable", category: "Keywords")]
	protected ref array<ref SCR_AutocompletePlugin_KeywordData> m_aToolKeywords;

	[Attribute(defvalue: "1", desc: "Use the user-defined keyword list below - this list will never be touched or reset in any way by the tool", category: "Keywords")]
	protected bool m_bUseUserKeywords;

	[Attribute(desc: "User-defined keyword-replacement pairs", category: "Keywords")]
	protected ref array<ref SCR_AutocompletePlugin_KeywordData> m_aUserKeywords;

	/*
		Category: Attributes
	*/

	[Attribute(defvalue: "1", desc: "Use Tool Attributes", category: "Attribute Decorators")]
	protected bool m_bUseToolAttributeDecorators;

	[Attribute(desc: "Tool-defined [Attribute()] decorators - editable and resettable", category: "Attribute Decorators")]
	protected ref array<ref SCR_AutocompletePlugin_AttributeData> m_aToolAttributeDecorators;

	[Attribute(defvalue: "1", desc: "Use the user-defined [Attribute()] decorator list below - this list will never be touched or reset in any way by the tool", category: "Attribute Decorators")]
	protected bool m_bUseUserAttributeDecorators;

	[Attribute(desc: "User-defined [Attribute()] decorators", category: "Attribute Decorators")]
	protected ref array<ref SCR_AutocompletePlugin_AttributeData> m_aUserAttributeDecorators;

	protected ref map<string, SCR_AutocompletePlugin_KeywordData> m_mKeywords = new map<string, SCR_AutocompletePlugin_KeywordData>();
	protected ref map<string, SCR_AutocompletePlugin_AttributeData> m_mAttributeDecorators = new map<string, SCR_AutocompletePlugin_AttributeData>();

	protected static const string SEP_NL = SCR_StringHelper.DOUBLE_SLASH + "------------------------------------------------------------------------------------------------\n";
	protected static const string CONSTRUCTOR_KEYWORD = "ctor";
	protected static const string DESTRUCTOR_KEYWORD = "dtor";
	protected static const string ENTITY_SUFFIX = "Entity"; // used to detect IEntity's ctor(IEntitySource src, IEntity parent)

	protected static const ref array<string> NATIVE_TYPES = {
		"bool", "float", "int", "string", "typename", "vector",
		"FactionKey", "LocalizedString", "ResourceName",
	};

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		AutoCompleteCurrentLine();
	}

	//------------------------------------------------------------------------------------------------
	protected void AutoCompleteCurrentLine()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
		{
			PrintFormat("[SCR_AutocompletePlugin.AutoCompleteCurrentLine] Script Editor is unavailable?! (%1 L%2)", __FILE__, __LINE__, level: LogLevel.ERROR);
			return;
		}

		string currentLine; // used as currentFile first
		if (!scriptEditor.GetCurrentFile(currentLine))
			return;

//		if (currentLine == __FILE__)
//		{
//			Print("cannot process plugin file");
//			return;
//		}

		if (!scriptEditor.GetLineText(currentLine))
		{
			PrintFormat("[SCR_AutocompletePlugin.AutoCompleteCurrentLine] Script Editor cannot read the current line (%1 L%2)", __FILE__, __LINE__, level: LogLevel.ERROR);
			return;
		}

		string indentation;
		SCR_BasicCodeFormatterPlugin.GetIndentAndLineContent(currentLine, indentation, currentLine);

		int commentIndex = currentLine.IndexOf(SCR_StringHelper.DOUBLE_SLASH);
		if (commentIndex == 0) // the whole line is a comment
			return;

		string indentedComment;
		if (commentIndex > 0)
		{
			if (m_bKeepCommentIndentation)
			{
				string codeAndCommentLine = currentLine;
				currentLine = currentLine.Substring(0, commentIndex);
				currentLine.TrimInPlace();
				indentedComment = codeAndCommentLine.Substring(currentLine.Length(), codeAndCommentLine.Length() - currentLine.Length());
				indentedComment = SCR_StringHelper.TrimRight(indentedComment);
			}
			else
			{
				indentedComment = currentLine.Substring(commentIndex, currentLine.Length() - commentIndex);
				indentedComment.TrimInPlace();
				indentedComment = SCR_StringHelper.SPACE + indentedComment;
				currentLine = currentLine.Substring(0, commentIndex); // trimmed below
			}
		}

		currentLine.TrimInPlace();
		if (!currentLine)
			return;

		if (m_bAddInstantiation)
		{
			if (((m_iInstantiationTrigger == 0 || m_iInstantiationTrigger == 2) && currentLine.EndsWith(" ="))
				|| ((m_iInstantiationTrigger == 0 || m_iInstantiationTrigger == 1) && currentLine.EndsWith(" = new")))
			{
				if (AddNewType(scriptEditor, indentation, currentLine, indentedComment))
					return;
			}
		}

		if (m_bFixInstantiation)
		{
			if (FixInstantiation(scriptEditor, indentation, currentLine, indentedComment))
				return;
		}

		if (m_bAddCastNullcheck && currentLine.Contains(".Cast(")) // )
		{
			if (AddCastCheck(scriptEditor, indentation, currentLine))
				return;
		}

		if (m_bAddOrFixResourceLoadValidityCheck && currentLine.Contains("Resource.Load(")) // )
		{
			if (AddResourceLoadValidityCheck(scriptEditor, indentation, currentLine))
				return;
		}

		if (m_eAddPrintLogLevel > -1
			&& (currentLine.StartsWith("Print(") || currentLine.StartsWith("PrintFormat(")) // ))
			&& currentLine.EndsWith(");"))
		{
			if (AddPrintLogLevel(scriptEditor, indentation, currentLine, indentedComment))
				return;
		}

		string lineLC = currentLine;
		lineLC.ToLower();

		if (m_bAddConstructor && lineLC == CONSTRUCTOR_KEYWORD)
		{
			if (AddConstructorDestructor(scriptEditor, true, indentation, indentedComment))
				return;
		}

		if (m_bAddDestructor && lineLC == DESTRUCTOR_KEYWORD)
		{
			if (AddConstructorDestructor(scriptEditor, false, indentation, indentedComment))
				return;
		}

		// [Attribute()] ?
		if (indentation == SCR_StringHelper.TAB
			&& currentLine.Contains(SCR_StringHelper.SPACE)
			&& currentLine.EndsWith(";")
			&& !currentLine.Contains("="))
		{
			CheckToolAttributeDecorators();
			FillAttributeDecoratorMap();
			if (AddAttributeDecorator(scriptEditor, indentation, currentLine))
				return;
		}

		// only keywords can save us here

		CheckToolKeywords();
		FillKeywordsMap();

		SCR_AutocompletePlugin_KeywordData data = m_mKeywords.Get(currentLine); // case-sensitive!
		if (data)
			scriptEditor.SetLineText(AddIndentation(string.Format(data.m_sValue, indentedComment), indentation));
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] scriptEditor
	//! \param[in] indentation
	//! \param[in] currentLine
	//! \param[in] comment
	//! \return
	protected bool AddNewType(notnull ScriptEditor scriptEditor, string indentation, string currentLine, string comment)
	{
		if (!currentLine.Contains(SCR_StringHelper.EQUALS))
			return false;

		array<string> tokens = {};
		currentLine.Split(SCR_StringHelper.EQUALS, tokens, true);

		int tokensCount = tokens.Count();
		if (m_iInstantiationTrigger == 1) // "= new" only
		{
			if (tokensCount < 2)
				return false;
		}
		else
		{
			if (tokensCount < 1)
				return false;
		}

		tokens[0].Split(SCR_StringHelper.SPACE, tokens, true);
		if (tokens.Count() < 2)
			return false;

		tokens.RemoveItemOrdered("auto");
		tokens.RemoveItemOrdered("autoptr");
		tokens.RemoveItemOrdered("protected");
		tokens.RemoveItemOrdered("private");
		tokens.RemoveItemOrdered("static");
		if (tokens.IsEmpty())
			return false;

		if (tokens[0] == "ref")
		{
			tokens.RemoveOrdered(0);

			if (indentation != SCR_StringHelper.TAB && currentLine.StartsWith("ref ") && currentLine != "ref ")
			{
				currentLine = currentLine.Substring(4, currentLine.Length() - 4);
				currentLine.TrimInPlace();
			}
		}

		tokensCount = tokens.Count();
		if (tokensCount < 2)
			return false;

		string newVarType;
		if (tokensCount == 2)
		{
			newVarType = tokens[0];
		}
		else
		{
			tokens.Remove(tokensCount - 1);
			newVarType = SCR_StringHelper.Join(SCR_StringHelper.SPACE, tokens, false);
		}

		currentLine.Replace(" = new", " =");

		if (NATIVE_TYPES.Contains(newVarType))
		{
			currentLine.Replace(" =", string.Empty);
			currentLine.TrimInPlace();
			currentLine += ";";

			scriptEditor.SetLineText(string.Format("%1%2%3", indentation, currentLine, comment));
			return true;
		}

		if (newVarType.StartsWith("array<"))
			currentLine += " {};";
		else
			currentLine += " new " + newVarType + "();";

		scriptEditor.SetLineText(string.Format("%1%2%3", indentation, currentLine, comment));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] scriptEditor
	//! \param[in] indentation
	//! \param[in] currentLine
	//! \return
	protected bool FixInstantiation(notnull ScriptEditor scriptEditor, string indentation, string currentLine, string comment)
	{
		if (!currentLine.Contains(" = new "))
			return false;

		array<string> tokens = {};
		currentLine.Split(SCR_StringHelper.EQUALS, tokens, true);

		int tokensCount = tokens.Count();
		if (m_iInstantiationTrigger == 1) // "= new" only
		{
			if (tokensCount < 2)
				return false;
		}
		else
		{
			if (tokensCount < 1)
				return false;
		}

		tokens[0].Split(SCR_StringHelper.SPACE, tokens, true);
		if (tokens.Count() < 2)
			return false;

		array<string> keywords;
		if (indentation == SCR_StringHelper.TAB) // member/static properties
			keywords = {};

		tokens.RemoveItemOrdered("auto");
		tokens.RemoveItemOrdered("autoptr");
		if (keywords)
		{
			if (tokens.RemoveItemOrdered("protected"))
				keywords.Insert("protected");
			else
			if (tokens.RemoveItemOrdered("private"))
				keywords.Insert("private");
		}
	
		if (!tokens.IsEmpty() && tokens[0] == "ref")
		{
			tokens.RemoveOrdered(0);
			if (keywords) // member/static property = keep ref
				keywords.Insert("ref");
		}

		tokensCount = tokens.Count();
		if (tokensCount < 2)
			return false;

		string varType, varName;
		if (tokensCount == 2)
		{
			varType = tokens[0];
			varName = tokens[1];
		}
		else
		{
			varName = tokens[tokensCount - 1];
			tokens.Remove(tokensCount - 1);
			varType = SCR_StringHelper.Join(SCR_StringHelper.SPACE, tokens, false);
			
		}

		string newLine;
		if (NATIVE_TYPES.Contains(varType))
		{
			newLine = string.Format("%1 %2;", varType, varName);
			if (keywords)
				keywords.RemoveItemOrdered("ref");
		}
		else
		{
			if (varType.StartsWith("array<"))
				newLine = string.Format("%1 %2 = {};", varType, varName);
			else
				newLine = string.Format("%1 %2 = new %1();", varType, varName);
		}

		if (keywords && !keywords.IsEmpty())
			newLine = SCR_StringHelper.Join(SCR_StringHelper.SPACE, keywords) + SCR_StringHelper.SPACE + newLine;

		if (currentLine == newLine)
			return false;

		scriptEditor.SetLineText(string.Format("%1%2%3", indentation, newLine, comment));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scriptEditor
	//! \param[in] indentation
	//! \param[in] currentLine
	//! \return
	protected bool AddCastCheck(notnull ScriptEditor scriptEditor, string indentation, string currentLine)
	{
		array<string> tokens = {};
		currentLine.Split(SCR_StringHelper.SPACE, tokens, true);

		int tokensCount = tokens.Count();
		if (tokensCount < 3)
			return false;

		int varIndex;
		if (tokensCount == 3)
			varIndex = 0;
		else // greater than that
			varIndex = 1;

		if (tokens[varIndex + 1] != "=")
			return false;

		string varName = tokens[varIndex];

		string belowLine;
		int currentLineNumber = scriptEditor.GetCurrentLine();
		if (!scriptEditor.GetLineText(belowLine, currentLineNumber + 1))
			return false;

		belowLine.TrimInPlace();
		if (belowLine.StartsWith("if (!" + varName + ")"))
			return false;

		string insert = "if (!" + varName + ")\n\treturn;";
		if (belowLine) // !.IsEmpty()
			insert += "\n";

		scriptEditor.InsertLine(AddIndentation(insert, indentation), scriptEditor.GetCurrentLine() + 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scriptEditor
	//! \param[in] indentation
	//! \param[in] currentLine
	//! \return
	protected bool AddResourceLoadValidityCheck(notnull ScriptEditor scriptEditor, string indentation, string currentLine)
	{
		array<string> tokens = {};
		currentLine.Split(SCR_StringHelper.SPACE, tokens, true);

		if (tokens[0] == "Resource")
			tokens.RemoveOrdered(0);

		if (tokens.Count() < 3)
			return false;

		if (tokens[1] != "=")
			return false;

		string belowLine;
		int nextLineNumber = scriptEditor.GetCurrentLine() + 1;
		if (!scriptEditor.GetLineText(belowLine, nextLineNumber))
			return false;

		string varName = tokens[0];
		belowLine.TrimInPlace();
		if (belowLine.StartsWith("if (!" + varName + ".IsValid())"))
			return false;

		string nullCheck = "if (!" + varName + ")";
		if (belowLine.StartsWith(nullCheck)) // fix
		{
			if (belowLine == nullCheck)
			{
				scriptEditor.SetLineText(indentation + "if (!" + varName + ".IsValid())", nextLineNumber)
			}
			else // move comment along
			{
				string restOfTheLine = belowLine.Substring(nullCheck.Length(), belowLine.Length() - nullCheck.Length());
				scriptEditor.SetLineText(indentation + "if (!" + varName + ".IsValid())" + restOfTheLine, nextLineNumber);
			}

			return true;
		}

		string insert = "if (!" + varName + ".IsValid())\n\treturn;";
		if (belowLine) // !.IsEmpty()
			insert += "\n";

		scriptEditor.InsertLine(AddIndentation(insert, indentation), nextLineNumber);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] scriptEditor
	//! \param[in] indentation
	//! \param[in] currentLine
	//! \param[in] comment
	//! \return
	protected bool AddPrintLogLevel(notnull ScriptEditor scriptEditor, string indentation, string currentLine, string comment)
	{
		if (currentLine.StartsWith("Print(")) // )
		{
			if (currentLine.Contains("LogLevel.") || currentLine.EndsWith(", level);") || currentLine.EndsWith(", logLevel);"))
				return false;

			scriptEditor.SetLineText(
				string.Format("%1%2, LogLevel.%3);%4",
					indentation,
					currentLine.Substring(0, currentLine.Length() - 2),
					typename.EnumToString(LogLevel, m_eAddPrintLogLevel),
					comment));
			return true;
		}

		if (currentLine.StartsWith("PrintFormat(")) // )
		{
			if (currentLine.Contains("level: "))
				return false;

			if (currentLine.Contains("LogLevel."))
				scriptEditor.SetLineText(indentation + SCR_StringHelper.InsertAt(currentLine, "level: ", currentLine.LastIndexOf("LogLevel.")) + comment);
			else
				scriptEditor.SetLineText(
					string.Format("%1%2, level: LogLevel.%3);%4",
						indentation,
						currentLine.Substring(0, currentLine.Length() - 2),
						typename.EnumToString(LogLevel, m_eAddPrintLogLevel),
						comment));

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scriptEditor
	//! \param[in] indentation
	//! \param[in] currentLine
	//! \return
	protected bool AddAttributeDecorator(notnull ScriptEditor scriptEditor, string indentation, string currentLine)
	{
		int currentLineNumber = scriptEditor.GetCurrentLine();
		if (currentLineNumber > 0)
		{
			string aboveLine;
			if (!scriptEditor.GetLineText(aboveLine, currentLineNumber - 1) || aboveLine.StartsWith("\t[Attribute(")) // )
				return false;
		}

		array<string> tokens = {};
		currentLine.Split(SCR_StringHelper.SPACE, tokens, true);

		tokens.RemoveItemOrdered("protected");
		tokens.RemoveItemOrdered("private");
		tokens.RemoveItemOrdered("ref");

		int count = tokens.Count();
		if (count < 2)
			return false;

		if (count > 2)
		{
			if (tokens[0] == "array<ref")
			{
				tokens[0] = tokens[0] + SCR_StringHelper.SPACE + tokens[1];
				tokens.RemoveOrdered(1);
				--count;
			}
		}

		if (count > 2 && tokens[2] != "=")
			return false;

		string name = tokens[1];
		if (!name.StartsWith("m_"))
			return false;

		int nameLength = name.Length();
		if (nameLength < 3) // m_W is valid
			return false;

		string hungarianPrefix = name[2];
		if (!SCR_StringHelper.LOWERCASE.Contains(hungarianPrefix))
			hungarianPrefix = string.Empty;

		string typeStr = tokens[0];
		string friendlyName = SCR_StringHelper.FormatValueNameToUserFriendly(name);

		SCR_AutocompletePlugin_AttributeData data = m_mAttributeDecorators.Get(typeStr);
		if (data && hungarianPrefix == data.m_sPrefix)
		{
			scriptEditor.InsertLine(indentation + string.Format(data.m_sValue, friendlyName, typeStr));
			return true;
		}

		// no exact type match found, dig deeper

		typename type = typeStr.ToType();

		// no map/set support
		if (type && (type.IsInherited(map) || type.IsInherited(set)))
			return false;

		// array
		if (typeStr.StartsWith("array<") || (type && type.IsInherited(array)))
		{
			if (hungarianPrefix == "a")
			{
				data = m_mAttributeDecorators.Get("array");
				if (data)
					scriptEditor.InsertLine(indentation + string.Format(data.m_sValue, friendlyName, typeStr));
				else
					scriptEditor.InsertLine(indentation + string.Format("[Attribute(desc: \"%1\")]", friendlyName, typeStr));
			}

			return true;
		}

		// enum
		if (hungarianPrefix == "e")
		{
			int length = typeStr.Length();
			bool isEnum = length > 2 && typeStr[0] == "E" && SCR_StringHelper.UPPERCASE.Contains(typeStr[1]); // e.g EFuelType
			if (!isEnum) // tag-prefixed?
			{
				int index = typeStr.IndexOf("_E");
				if (index > 1 && index < 9 && index + 2 < typeStr.Length() && SCR_StringHelper.UPPERCASE.Contains(typeStr[index + 2])) // if has _E + uppercase within the first 11 chars (tag = 2 to 8 chars)
					isEnum = true;
			}

			if (isEnum)
			{
				data = m_mAttributeDecorators.Get("enum");
				if (data)
					scriptEditor.InsertLine(indentation + string.Format(data.m_sValue, friendlyName, typeStr));
				else
					scriptEditor.InsertLine(indentation + string.Format("[Attribute(desc: \"%1\", uiwidget: UIWidgets.CheckBox, enumType: %2)]", friendlyName, typeStr));

				return true;
			}
		}

		// objects - also covers arrays, hence having them first
		if (type && type.IsInherited(Managed))
		{
			scriptEditor.InsertLine(indentation + string.Format("[Attribute(desc: \"%1\")]", friendlyName, typeStr));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Add indentation to the provided text, keeping empty lines empty
	//! \param[in] input
	//! \param[in] indentation
	//! \return the indented text
	protected string AddIndentation(string input, string indentation)
	{
		array<string> lines = {};
		input.Split(SCR_StringHelper.LINE_RETURN, lines, false);

		if (lines.IsEmpty())
			return string.Empty;

		foreach (int i, string line : lines)
		{
			if (line)
				lines[i] = indentation + line;
		}

		return SCR_StringHelper.Join(SCR_StringHelper.LINE_RETURN, lines, true);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scriptEditor
	//! \param[in] isConstructor true for constructor, false for destructor
	//! \param[in] indentation
	//! \param[in] comment
	//! \return
	protected bool AddConstructorDestructor(notnull ScriptEditor scriptEditor, bool isConstructor, string indentation, string comment)
	{
		string className;
		string methodName;
		if (!SCR_CopyClassAndMethodPlugin.GetCursorClassAndMethodNames(scriptEditor, className, methodName))
			return false;

		if (!className)
		{
			Print("Cannot define current class", LogLevel.WARNING);
			className = "DefaultClassName";
		}

		string text;
		if (isConstructor)
		{
			if (className.EndsWith(ENTITY_SUFFIX))
				text = string.Format(SEP_NL + SCR_StringHelper.DOUBLE_SLASH + " constructor\nvoid %1(IEntitySource src, IEntity parent)%2\n{\n\t\n}", className, comment);
			else
				text = string.Format(SEP_NL + SCR_StringHelper.DOUBLE_SLASH + " constructor\nvoid %1()%2\n{\n\t\n}", className, comment);
		}
		else
		{
			text = string.Format(SEP_NL + SCR_StringHelper.DOUBLE_SLASH + " destructor\nvoid ~%1()%2\n{\n\t\n}", className, comment);
		}

		scriptEditor.SetLineText(AddIndentation(text, indentation));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckToolKeywords()
	{
		set<string> keywords = new set<string>();
		SCR_AutocompletePlugin_KeywordData tmpData;
		for (int i = m_aToolKeywords.Count() - 1; i >= 0; i--)
		{
			tmpData = m_aToolKeywords[i];
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(tmpData.m_sKeyword))
				m_aToolKeywords.RemoveOrdered(i);
			else
				keywords.Insert(tmpData.m_sKeyword);
		}

		// structure
		if (!keywords.Contains("if"))				m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("if", "if (condition)%1\n{\n\t\n}"));
		if (!keywords.Contains("ife"))				m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("ife", "if (condition)%1\n{\n\t\n}\nelse\n{\n\t\n}"));
		if (!keywords.Contains("for"))				m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("for", "for (int i, count = arr.Count(); i < count; i++)%1\n{\n\t\n}"));
		if (!keywords.Contains("forr"))				m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("forr", "for (int i = arr.Count() - 1; i >= 0; i--)%1\n{\n\t\n}")); // for, reversed
		if (!keywords.Contains("foreach"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("foreach", "foreach (SCR_Class item : items)%1\n{\n\t\n}"));
		if (!keywords.Contains("foreachi"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("foreachi", "foreach (int i, SCR_Class item : items)%1\n{\n\t\n}")); // foreach with index
		if (!keywords.Contains("switch"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("switch", "switch (value)%1\n{\n\tcase 0:\n\t\tbreak;\n\n\tdefault:\n\t\tbreak;\n}"));
		if (!keywords.Contains("while"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("while", "while (condition)%1\n{\n\t\n}"));

		// big structure
		if (!keywords.Contains("class"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("class", "class SCR_MyClass%1\n{\n\tprotected string m_sValue = \"Generated class\";\n\n\t" + SEP_NL + "\t/" + "/! constructor\n\tvoid SCR_MyClass()\n\t{\n\t\t\n\t}\n}"));
		if (!keywords.Contains("func"))				m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("func", SEP_NL + SCR_StringHelper.DOUBLE_SLASH + "! \\param[in] parameter\n" + "protected void Method(int parameter)%1\n{\n\t\n}")); // meh alias
		if (!keywords.Contains("method"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("method", SEP_NL + SCR_StringHelper.DOUBLE_SLASH + "! \\param[in] parameter\n" + "protected void Method(int parameter)%1\n{\n\t\n}"));

		// syntactic sugar
//		if (!keywords.Contains("const"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("const", "protected static const int CONST_VALUE = 42;"));
//		if (!keywords.Contains("array"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("array", "protected ref array<string> m_aArray = {};"));
//		if (!keywords.Contains("map"))				m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("map", "protected ref map<int, string> m_mMap = new map<int, string>();"));
//		if (!keywords.Contains("set"))				m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("set", "protected ref set<string> m_mMap = new set<string>();"));
		if (!keywords.Contains("findcomp"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("findcomp", "SCR_ComponentClass component = SCR_ComponentClass.Cast(entity.FindComponent(SCR_ComponentClass));%1\nif (!comp)\n\treturn;"));
		if (!keywords.Contains("print"))			m_aToolKeywords.Insert(SCR_AutocompletePlugin_KeywordData.Create("print", "Print(\"fill\", LogLevel.NORMAL);"));
	}

	//------------------------------------------------------------------------------------------------
	protected void FillKeywordsMap()
	{
		m_mKeywords.Clear();

		if (m_bUseToolKeywords)
		{
			foreach (SCR_AutocompletePlugin_KeywordData data : m_aToolKeywords)
			{
				// data.m_sKeyword.TrimInPlace();
				if (data.m_bEnabled && !m_mKeywords.Contains(data.m_sKeyword))
					m_mKeywords.Insert(data.m_sKeyword, data);
			}
		}

		if (m_bUseUserKeywords)
		{
			foreach (SCR_AutocompletePlugin_KeywordData data : m_aUserKeywords)
			{
				// data.m_sKeyword.TrimInPlace();
				if (data.m_bEnabled && !m_mKeywords.Contains(data.m_sKeyword))
					m_mKeywords.Insert(data.m_sKeyword, data);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckToolAttributeDecorators()
	{
		set<string> attributeDecorators = new set<string>();
		SCR_AutocompletePlugin_AttributeData tmpData;
		for (int i = m_aToolAttributeDecorators.Count() - 1; i >= 0; i--)
		{
			tmpData = m_aToolAttributeDecorators[i];
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(tmpData.m_sType))
				m_aToolAttributeDecorators.RemoveOrdered(i);
			else
				attributeDecorators.Insert(tmpData.m_sType);
		}

		if (!attributeDecorators.Contains("array"))			m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("array",			"a", "[Attribute(defvalue: \"" + "\", desc: \"%1\")]"));
		if (!attributeDecorators.Contains("bool"))			m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("bool",			"b", "[Attribute(defvalue: \"1\", desc: \"%1\")]"));
		if (!attributeDecorators.Contains("Color"))			m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("Color",			"", "[Attribute(defvalue: \"1 1 1 1\", desc: \"%1\")]"));
		if (!attributeDecorators.Contains("enum"))			m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("enum",			"e", "[Attribute(defvalue: \"0\", desc: \"%1\", uiwidget: UIWidgets.ComboBox, enumType: %2)]"));
		if (!attributeDecorators.Contains("float"))			m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("float",			"f", "[Attribute(defvalue: \"0\", desc: \"%1\", params: \"0 inf 0.01\")]"));
		if (!attributeDecorators.Contains("int"))			m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("int",			"i", "[Attribute(defvalue: \"0\", desc: \"%1\", params: \"0 inf\")]"));
		if (!attributeDecorators.Contains("ResourceName"))	m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("ResourceName",	"s", "[Attribute(defvalue: \"" + "\", desc: \"%1\", uiwidget: UIWidgets.ResourcePickerThumbnail, params: \"edds et wav\")]"));
		if (!attributeDecorators.Contains("string"))		m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("string",			"s", "[Attribute(defvalue: \"Default value\", desc: \"%1\")]"));
		if (!attributeDecorators.Contains("vector"))		m_aToolAttributeDecorators.Insert(SCR_AutocompletePlugin_AttributeData.Create("vector",			"v", "[Attribute(defvalue: \"0 0 0\", desc: \"%1\")]"));
	}

	//------------------------------------------------------------------------------------------------
	protected void FillAttributeDecoratorMap()
	{
		m_mAttributeDecorators.Clear();

		if (m_bUseToolAttributeDecorators)
		{
			foreach (SCR_AutocompletePlugin_AttributeData data : m_aToolAttributeDecorators)
			{
				if (data.m_bEnabled && !m_mAttributeDecorators.Contains(data.m_sType))
					m_mAttributeDecorators.Insert(data.m_sType, data);
			}
		}

		if (m_bUseUserAttributeDecorators)
		{
			foreach (SCR_AutocompletePlugin_AttributeData data : m_aUserAttributeDecorators)
			{
				if (data.m_bEnabled && !m_mAttributeDecorators.Contains(data.m_sType))
					m_mAttributeDecorators.Insert(data.m_sType, data);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		CheckToolKeywords();
		CheckToolAttributeDecorators();
		Workbench.ScriptDialog("Autocomplete Plugin", "Autocompletion settings", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected int ButtonOK()
	{
		return 1;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sKeyword", "%1")]
class SCR_AutocompletePlugin_KeywordData
{
	[Attribute(defvalue: "1", desc: "Enable this keyword-text pair")]
	bool m_bEnabled;

	[Attribute(defvalue: "keyword", desc: "Keyword to be detected - case-sensitive")]
	string m_sKeyword;

	[Attribute(defvalue: "protected string m_sKeyword = \"keyword\";", desc: "Text replacing the detected keyword\n- \\n for line return\n- \\t for tabulation\n- %1 for the in-line comment that was after the keyword (if any)", uiwidget: UIWidgets.EditBoxMultiline)]
	string m_sValue;

	//------------------------------------------------------------------------------------------------
	//! "Constructor", as BaseContainerProps must have a normal constructor to be properly created and filled by the Workbench
	//! \param[in] keyword
	//! \param[in] value
	//! \param[in] enabled
	//! \return
	static SCR_AutocompletePlugin_KeywordData Create(string keyword, string value, bool enabled = true)
	{
		SCR_AutocompletePlugin_KeywordData result = new SCR_AutocompletePlugin_KeywordData();

		result.m_bEnabled = enabled;
		result.m_sKeyword = keyword;
		result.m_sValue = value;

		return result;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sType", "%1")]
class SCR_AutocompletePlugin_AttributeData
{
	[Attribute(defvalue: "1", desc: "Enable this attribute decorator")]
	bool m_bEnabled;

	[Attribute(defvalue: "array<ref SCR_Bird>", desc: "Classname or native type")]
	string m_sType;

	[Attribute(defvalue: "a", desc: "The 'x' in the m_xName prefix (e.g i for int, b for bool etc)\nCan be empty", precision: 1)]
	string m_sPrefix;

	[Attribute(defvalue: "[Attribute(defvalue: \"0\", desc: \"%1\")]", desc: "Value\n%1 = friendly variable name (e.g 'm_bCheckThis' → 'Check This')\n%2 = value type (e.g 'array<ref SCR_MyClass>')")]
	string m_sValue;

	//------------------------------------------------------------------------------------------------
	//! "Constructor", as BaseContainerProps must have a normal constructor to be properly created and filled by the Workbench
	//! \param[in] type
	//! \param[in] prefix
	//! \param[in] value
	//! \return
	static SCR_AutocompletePlugin_AttributeData Create(string type, string prefix, string value, bool enabled = true)
	{
		SCR_AutocompletePlugin_AttributeData result = new SCR_AutocompletePlugin_AttributeData();

		result.m_sType = type.Trim();

		prefix.TrimInPlace();
		if (prefix)
			result.m_sPrefix = prefix[0];

		value.TrimInPlace();
		result.m_sValue = value;
		result.m_bEnabled = enabled;

		return result;
	}
}
#endif
