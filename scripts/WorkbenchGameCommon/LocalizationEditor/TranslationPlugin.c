#ifdef WORKBENCH
//! This plugin contacts an API through JSON to obtain translations from selected rows.\n
//! - Locales are determined by removing the provided prefix from its attributes (e.g Target_en_us - Target_ = en_us)\n
//! - Locale matchings and meta fields are defined in the Match Config (see this config as a "stringtable item codec")
// calling it delocalised localisationer would have been so much better
[WorkbenchPluginAttribute(
	name: PLUGIN_NAME,
	description: "Use a network API to translate selected/filtered rows",
	shortcut: "Ctrl+Shift+T",
	wbModules: { "LocalizationEditor" },
	awesomeFontCode: 0xF1AB)]
class TranslationPlugin : LocalizationEditorPlugin
{
	//
	//	TEMP
	//

	[Attribute(defvalue: "1", desc: "Disable global locales", category: "TEMP")]
	protected bool m_bDisableLocalesGrouping;

	//
	//	Selection
	//

	[Attribute(defvalue: ETranslationPlugin_EditedRowsMode.PROCESS_ALL.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Define the behaviour of edited entries (among the selected ones)", enumType: ETranslationPlugin_EditedRowsMode, category: "Selection")]
	protected ETranslationPlugin_EditedRowsMode m_eEditedEntriesMode;

	//
	//	Actions
	//

	[Attribute(defvalue: "0", desc: "Overwrite existing translations if the entry itself is eligible to translation; otherwise only fill empty fields", category: "Actions")]
	protected bool m_bOverwriteExistingTranslations;

	[Attribute(defvalue: "1", desc: "Set entry's edited value as original value"
		+ "\ne.g Target_en_us takes the value of Target_en_us_edited and Target_en_us_edited gets cleared",
		category: "Actions")]
	protected bool m_bSetEditedAsOriginal;

	//
	//	Advanced
	//

	[Attribute(desc: "[Mandatory] The translation server API URL, e.g http:/" + "/myserver/?mode=translate\nDefault protocol: " + DEFAULT_PROTOCOL + ":/" + "/", category: "Advanced")]
	protected string m_sServerURL;

	[Attribute(desc: "The translation server's authentication token, if needed", category: "Advanced")]
	protected string m_sServerToken;

	[Attribute(defvalue: "0", desc: "Show which generative AI model was used, cost etc (if available)", category: "Advanced")]
	protected bool m_bDisplayAdvancedStatsOnSuccess;

	[Attribute(desc: "[Mandatory] Locale-matching and special fields listing (see it as a string table item adapter)", category: "Advanced")]
	protected ref TranslationPluginMatchConfig m_MatchConfig;

	//
	//	Debug
	//

	[Attribute(defvalue: "0", desc: "Log when translation changes happen", category: "Debug")]
	protected bool m_bLogChanges;

	[Attribute(defvalue: "0", desc: "Log when network happens", category: "Debug")]
	protected bool m_bLogNetwork;

	[Attribute(defvalue: ETranslationPlugin_ProcessMode.PRODUCTION.ToString(), uiwidget: UIWidgets.ComboBox, enumType: ETranslationPlugin_ProcessMode, category: "Debug")]
	protected ETranslationPlugin_ProcessMode m_eProcessMode;

	// data that must be kept between REST send and receive (async)
	protected ref map<string, ref TranslationPlugin_DataWrapper> m_mIdDataMap;
	protected int m_iTranslationQueriesCount;
	protected ref array<string> m_aGlobalLocales;

	// REST check and variables
	protected bool m_bWaitingOnRestAPI;
	protected int m_iLastUsage;
	protected ref RestCallback m_RestCallback;
	protected ref WBProgressDialog m_ProgressBar;

	protected static const string PLUGIN_NAME = "Translation plugin";

	protected static const string SOURCE_ID = "SOURCE";
	protected static const string SOURCE_EDITED_ID = "SOURCE_EDITED";

	protected static const int MAX_DISPLAYED_IDS = 6;	//!< max item ids displayed in the "are you sure?" modal

	// enums
	protected static const int GENDER_MALE_INDEX = 1; // 0 (and any other value) = NONE
	protected static const int GENDER_FEMALE_INDEX = 2;
	protected static const int GENDER_OTHER_INDEX = 3;

	protected static const string GENDER_MALE_VALUE = "M";
	protected static const string GENDER_FEMALE_VALUE = "F";
	protected static const string GENDER_OTHER_VALUE = "O"; // see DICOM standards's 0010:0040 patient field

	protected static const string PROTOCOL_SEPARATOR = ":/" + "/";
	protected static const string DEFAULT_PROTOCOL = "https";
	protected static const ref array<string> ACCEPTED_PROTOCOLS = { "http" + PROTOCOL_SEPARATOR, DEFAULT_PROTOCOL + PROTOCOL_SEPARATOR };

	// token filter - ^[a-zA-Z0-9-._~+/=]+$
	protected static const string TOKEN_WHITELIST = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~+/="; // QnD

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		if (!m_MatchConfig)
			ResetConfig();

		LocalizationEditor stringEditor = Workbench.GetModule(LocalizationEditor);
		if (!stringEditor)
		{
			PrintDialog("LocalizationEditor is not available", PLUGIN_NAME, LogLevel.ERROR);
			return;
		}

		BaseContainer stringTable = stringEditor.GetTable();
		if (!stringTable)
		{
			PrintDialog("No file opened.", PLUGIN_NAME, LogLevel.WARNING);
			return;
		}

		BaseContainerList stringTableItems = stringTable.GetObjectArray("Items");
		if (!stringTableItems)
		{
			PrintDialog("Error within the opened file - no Items property found.", PLUGIN_NAME, LogLevel.ERROR);
			return;
		}

		int stringTableItemsCount = stringTableItems.Count();
		if (stringTableItemsCount < 1)
		{
			PrintDialog("The current file is empty.", PLUGIN_NAME, LogLevel.NORMAL);
			return;
		}

		// waiting on REST API? (not too long)
		if (m_bWaitingOnRestAPI)
		{
			PrintDialog("Please wait for the result", PLUGIN_NAME, LogLevel.NORMAL);
			return;
		}

		array<int> selectedRows = {};
		stringEditor.GetSelectedRows(selectedRows);
		int potentialRowsCount = selectedRows.Count();

		if (potentialRowsCount < 1)
		{
			stringEditor.GetFilteredRows(selectedRows);
			potentialRowsCount = selectedRows.Count();
			if (potentialRowsCount < 1 || potentialRowsCount >= stringTableItemsCount)
			{
				PrintDialog("No lines selected/filtered.", PLUGIN_NAME, LogLevel.NORMAL);
				return;
			}
		}

		if (!CheckFields(stringTableItems.Get(selectedRows[0])))
			return; // CheckFields already has PrintDialogs

		int start = System.GetTickCount();
		map<string, ref TranslationPlugin_DataWrapper> idWrapperMap = GetIdWrapperMap(stringTableItems, selectedRows);
		PrintFormat("%1 ms - Finding translation needs", System.GetTickCount(start), level: LogLevel.NORMAL);

		int queriesCount = idWrapperMap.Count();
		if (queriesCount < 1)
		{
			PrintDialog("No selected rows need translation following the set options.", PLUGIN_NAME, LogLevel.NORMAL);
			return;
		}

		string message;
		if (queriesCount == 1)
		{
			string id = idWrapperMap.GetKey(0);
			if (id.IsEmpty())
				id = "Unknown";

			if (potentialRowsCount == 1)
				message = "You are about to translate the \"" + id + "\" row. Continue?";
			else
				message = "You are about to only translate the \"" + id + "\" row out of " + potentialRowsCount + " rows. Continue?";
		}
		else
		{
			if (queriesCount == potentialRowsCount)
				message = string.Format("You are about to translate the following %1 rows:\n", queriesCount);
			else
				message = string.Format("You are about to translate the following %1 out of %2 rows:\n", queriesCount, potentialRowsCount);

			int count;
			foreach (string id, TranslationPlugin_DataWrapper wrapper : idWrapperMap)
			{
				++count; // will replace e.g the last wanted element by (...) in order to keep the good number of lines
				if (count >= MAX_DISPLAYED_IDS)
				{
					message += "\n\t- (...)";
					break;
				}

				if (id.IsEmpty())
					message += "\n\t- Unknown";
				else
					message += "\n\t- " + id;
			}

			message += "\n\nContinue?";
		}

		message += "\n\nOptions:";
		if (m_bOverwriteExistingTranslations)
			message += "\n✔ Overwrite existing translations";
		else
			message += "\n✖ Overwrite existing translations";

		if (m_bSetEditedAsOriginal)
		{
			if (m_MatchConfig.m_sDefaultSourceEdited.IsEmpty())
				message += "\n✖ Set edited translation as translation source (Default Source Edited field is empty)";
			else
				message += "\n✔ Set edited translation as translation source";
		}
		else
		{
			message += "\n✖ Set edited translation as translation source";
		}

		string processMode = typename.EnumToString(ETranslationPlugin_ProcessMode, m_eProcessMode);
		if (Workbench.ScriptDialog(string.Format("[%1] %2", processMode, PLUGIN_NAME), message, new WorkbenchDialog_OKCancel()) == 0)
			return;

		m_iLastUsage = System.GetTickCount();
		m_iTranslationQueriesCount = queriesCount;

		StartProcess(idWrapperMap);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if Id, Default Source and eventually Default Source Edited config fields are correct
	//! \param[in] item
	//! \return true on success, false otherwise
	protected bool CheckFields(notnull BaseContainer item)
	{
		array<string> wrongFields = {};
		string tmp;
		if (!item.Get(m_MatchConfig.m_sIdField, tmp))
			wrongFields.Insert("Id Field (" + m_MatchConfig.m_sIdField + ")");

		if (!item.Get(m_MatchConfig.m_sDefaultSource, tmp))
			wrongFields.Insert("Default Source (" + m_MatchConfig.m_sDefaultSource + ")");
		
		if (m_bSetEditedAsOriginal && !m_MatchConfig.m_sDefaultSourceEdited.IsEmpty() && !item.Get(m_MatchConfig.m_sDefaultSourceEdited, tmp))
			wrongFields.Insert("Default Source Edited (" + m_MatchConfig.m_sDefaultSourceEdited + ")");

		if (!wrongFields.IsEmpty())
		{
			string message = "Wrong fields have been detected in configuration (bad config used?)";
			foreach (string wrongField : wrongFields)
			{
				message += "\n- Cannot find " + wrongField;
			}

			PrintDialog(message, PLUGIN_NAME, LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Send REST request or Simulate
	//! \param[in] idWrapperMap map with required translations
	//! \return true on success, false otherwise
	protected bool StartProcess(notnull map<string, ref TranslationPlugin_DataWrapper> idWrapperMap)
	{
		int count = idWrapperMap.Count();
		if (count < 1)
			return false;

		Game game = GetGame();
		if (!game)
		{
			Print("Cannot obtain game with GetGame()", LogLevel.ERROR);
			return false;
		}

		RestApi restAPI = game.GetRestApi();
		if (!restAPI)
		{
			Print("Cannot obtain REST API with game.GetRestApi()", LogLevel.ERROR);
			return false;
		}

		bool areLocalesSame = !m_bDisableLocalesGrouping;

		TranslationPluginRequest request = new TranslationPluginRequest();
		request.locales = {};
		request.locales.Copy(idWrapperMap.GetElement(0).m_Query.locales);
		request.locales.Sort();
		foreach (string id, TranslationPlugin_DataWrapper dataWrapper : idWrapperMap)
		{
			request.queries.Insert(dataWrapper.m_Query);
			dataWrapper.m_Query.locales.Sort();
			if (count > 1 && areLocalesSame && !AreLocalesEqual(request.locales, dataWrapper.m_Query.locales))
				areLocalesSame = false;
		}

		if (areLocalesSame)
		{
			m_aGlobalLocales = request.locales;
			foreach (string id, TranslationPlugin_DataWrapper dataWrapper : idWrapperMap)
			{
				dataWrapper.m_Query.locales = null;
			}
		}
		else
		{
			request.locales = null;
		}

		m_mIdDataMap = idWrapperMap; // to survive async
		if (m_eProcessMode == ETranslationPlugin_ProcessMode.PRODUCTION)
		{
			bool result = SendRequest(restAPI, request);
			if (!result)
				ResetRequestState();

			return result;
		}

		m_bWaitingOnRestAPI = true;

		switch (m_eProcessMode)
		{
			case ETranslationPlugin_ProcessMode.SIMULATE_SUCCESS: ProcessSuccessResponse(GetFakeResponse()); return true;

			case ETranslationPlugin_ProcessMode.SIMULATE_ERROR_403: ProcessErrorResponse(HttpCode.HTTP_CODE_403, ERestResult.EREST_ERROR); break;
			case ETranslationPlugin_ProcessMode.SIMULATE_ERROR_404: ProcessErrorResponse(HttpCode.HTTP_CODE_404, ERestResult.EREST_ERROR); break;
			case ETranslationPlugin_ProcessMode.SIMULATE_ERROR_408: ProcessErrorResponse(HttpCode.HTTP_CODE_408, ERestResult.EREST_ERROR_TIMEOUT); break;
			case ETranslationPlugin_ProcessMode.SIMULATE_ERROR_418: ProcessErrorResponse(HttpCode.HTTP_CODE_418, ERestResult.EREST_ERROR_UNKNOWN); break;
			case ETranslationPlugin_ProcessMode.SIMULATE_ERROR_500: ProcessErrorResponse(HttpCode.HTTP_CODE_500, ERestResult.EREST_ERROR_SERVERERROR); break;
			case ETranslationPlugin_ProcessMode.SIMULATE_ERROR_501: ProcessErrorResponse(HttpCode.HTTP_CODE_501, ERestResult.EREST_ERROR_NOTIMPLEMENTED); break;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SendRequest(notnull RestApi restAPI, notnull TranslationPluginRequest request)
	{
		string protocol, address, query;
		if (!SplitURL(m_sServerURL, protocol, address, query))
		{
			Print("URL \"" + m_sServerURL + "\" is invalid.", LogLevel.ERROR);
			return false;
		}

		string protocolAndAddress = protocol + address;

		m_RestCallback = new RestCallback();
		m_RestCallback.SetOnSuccess(REST_OnSuccess);
		m_RestCallback.SetOnError(REST_OnError);

		RestContext restContext = restAPI.GetContext(protocolAndAddress);
		if (!restContext)
		{
			Print("Cannot obtain REST context for " + protocolAndAddress, LogLevel.ERROR);
			return false;
		}

		if (!m_sServerToken.IsEmpty())
		{
			string sanitisedToken = SanitizeToken(m_sServerToken); // must be already sanitised by now
			if (!sanitisedToken.IsEmpty())
				restContext.SetHeaders("Authorization,Bearer " + sanitisedToken + ",Content-Type,application/json");
		}

		restContext.SetTimeout(300); // response can take long time
		
		request.Pack();
		string json = request.AsString();

		if (m_bLogNetwork)
		{
			if (query.IsEmpty())
				PrintFormat("Sending JSON by POST to %1, no query\nJSON: %2", protocolAndAddress, json, level: LogLevel.NORMAL);
			else
				PrintFormat("Sending JSON by POST to %1, query %2\nJSON: %3", protocolAndAddress, query, json, level: LogLevel.NORMAL);
		}

		int queriesCount = request.queries.Count();
		int localesCount;
		if (request.locales)
		{
			localesCount = request.locales.Count() * queriesCount;
		}
		else
		{
			foreach (TranslationPluginQuery translationQuery : request.queries)
			{
				localesCount += translationQuery.locales.Count();
			}
		}

		m_ProgressBar = new WBProgressDialog(string.Format("Fetching %1 translation(s) for %2 row(s)...", localesCount, queriesCount), Workbench.GetModule(LocalizationEditor));
		m_ProgressBar.SetProgress(0.42);

		restContext.POST(m_RestCallback, query, json);
		m_bWaitingOnRestAPI = true;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called by m_RestCallback's OnSuccess calls ProcessSuccessResponse quickly, kept short for simulation purpose
	//! \param[in] cb
	protected void REST_OnSuccess(RestCallback cb = null)
	{
		if (!cb)
		{
			PrintDialog("No REST callback returned!", PLUGIN_NAME, LogLevel.ERROR);
			return;
		}

		if (cb != m_RestCallback)
			return; // old request, do not treat

		TranslationPluginResponse response;
		string json = cb.GetData();
		if (m_bLogNetwork)
			Print("Received JSON: " + json, LogLevel.NORMAL);

		response = new TranslationPluginResponse();
		response.ExpandFromRAW(json);

		ProcessSuccessResponse(response);
	}

	//------------------------------------------------------------------------------------------------
	//! Processes successful response, whether real or simulated
	//! \param[in] response
	protected void ProcessSuccessResponse(notnull TranslationPluginResponse response)
	{
		LocalizationEditor stringEditor = Workbench.GetModule(LocalizationEditor);
		if (!stringEditor)
		{
			PrintDialog("LocalizationEditor is not available", PLUGIN_NAME, LogLevel.ERROR);
			ResetRequestState();
			return;
		}

		int start = System.GetTickCount();
		int modifiedRows = ApplyTranslations(stringEditor, response);
		PrintFormat("%1 ms - Applying translations", System.GetTickCount(start), level: LogLevel.NORMAL);

		if (modifiedRows > 0)
			stringEditor.RefreshUI();

		string message = string.Format(
			"Modified %1 out of %2 selected/filtered lines - processing time %3",
			modifiedRows,
			m_iTranslationQueriesCount,
			FormatDurationMs(System.GetTickCount(m_iLastUsage)));

		if (m_bDisplayAdvancedStatsOnSuccess && response.meta)
		{
			string metaMessage;
			if (!response.meta.model.IsEmpty())
				metaMessage += "\nAI model: " + response.meta.model;

			if (response.meta.usage)
			{
				if (response.meta.usage.prompt_tokens > 0 && response.meta.usage.completion_tokens > 0 && response.meta.usage.total_tokens > 0)
					metaMessage += string.Format(
						"\nTokens used: %1 (prompt) + %2 (completion) = %3 total",
						response.meta.usage.prompt_tokens,
						response.meta.usage.completion_tokens,
						response.meta.usage.total_tokens);
				else
				if (response.meta.usage.total_tokens > 0)
					metaMessage += "\nTokens used: " + response.meta.usage.total_tokens;
			}

			if (response.meta.total_cost > 0)
				metaMessage += "\nTotal cost: " + response.meta.total_cost + response.meta.currency;

			if (!metaMessage.IsEmpty())
				message += "\n" + metaMessage;
		}

		ResetRequestState();

		PrintDialog(message, PLUGIN_NAME, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called by m_RestCallback's OnError - calls ProcessErrorResponse quickly, kept short for simulation purpose
	//! \param[in] cb
	protected void REST_OnError(RestCallback cb = null)
	{
		if (!cb)
		{
			PrintDialog("No REST callback returned!", PLUGIN_NAME, LogLevel.ERROR);
			return;
		}

		if (cb != m_RestCallback)
			return; // old request, do not treat

		ProcessErrorResponse(cb.GetHttpCode(), cb.GetRestResult());
	}

	//------------------------------------------------------------------------------------------------
	//! Processes error response, whether real or simulated
	//! \param[in] errorCode
	//! \param[in] restResult
	protected void ProcessErrorResponse(HttpCode errorCode, ERestResult restResult)
	{
		ResetRequestState();

		string errorCodeStr = typename.EnumToString(HttpCode, errorCode);
		int httpCode = errorCodeStr.ToInt(offset: 10); // remove "HTTP_CODE_"; HTTP_CODE_NULL returns 0

		string errorName;
		switch (httpCode)
		{
			case 301: errorName = "Moved Permanently";					break;
			case 400: errorName = "Bad Request";						break;
			case 401: errorName = "Unauthorized";						break;
			case 403: errorName = "Forbidden";							break;
			case 403: errorName = "Forbidden";							break;
			case 404: errorName = "Not Found";							break;
			case 408: errorName = "Request Timeout";					break;
			case 410: errorName = "Gone";								break;
			case 418: errorName = "I'm a teapot";						break;
			case 500: errorName = "Internal Server Error";				break;
			case 501: errorName = "Not Implemented";					break;
			case 502: errorName = "Bad Gateway";						break;
			case 503: errorName = "Service Unavailable";				break;
			case 504: errorName = "Gateway Timeout";					break;
			case 511: errorName = "Network Authentication Required";	break;
			default: errorName = "Unknown - timeout/wrong target?";		break; // HTTP_CODE_NULL goes here
		}

		string message = string.Format(
			"Error communicating with the server:\n\nURL: %1\nError: %2 %3\n\n(%4 %5)",
			m_sServerURL,
			httpCode, errorName,
			errorCodeStr, typename.EnumToString(ERestResult, restResult));

		PrintDialog(message, PLUGIN_NAME + " - " + errorName, LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	//! Get required translations from selected/filtered rows
	//! \param[in] stringTableItems
	//! \param[in] rowsToTranslate
	//! \return a map filled with id-dataWrapper key-value pairs, or empty - never null
	protected map<string, ref TranslationPlugin_DataWrapper> GetIdWrapperMap(notnull BaseContainerList stringTableItems, notnull array<int> rowsToTranslate)
	{
		map<string, ref TranslationPlugin_DataWrapper> result = new map<string, ref TranslationPlugin_DataWrapper>();
		foreach (int rowToTranslate : rowsToTranslate)
		{
			BaseContainer item = stringTableItems.Get(rowToTranslate);
			if (!item)
				continue;

			string id;
			if (!item.Get(m_MatchConfig.m_sIdField, id))
			{
				PrintFormatDialog("Wrong ID field: \"%1\" is invalid", m_MatchConfig.m_sIdField, caption: PLUGIN_NAME, level: LogLevel.WARNING);
				break;
			}

			if (!item.Get(m_MatchConfig.m_sIdField, id) || id.IsEmpty())
			{
				Print("Element at row " + rowToTranslate + " does not have an ID", LogLevel.WARNING);
				continue;
			}

			if (!item.GetClassName().ToType() || !item.GetClassName().ToType().IsInherited(ScriptStringTableItem))
			{
				Print("Element " + id + " is not ScriptStringTableItem", LogLevel.WARNING);
				continue;
			}

			TranslationPlugin_DataWrapper dataWrapper = CreateDataWrapper(id, item);
			if (!dataWrapper) // error or no need to translate
				continue;

			if (result.Contains(id))
			{
				Print("ID " + id + " duplicate detected - skipping", LogLevel.WARNING);
				continue;
			}

			result.Insert(id, dataWrapper);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	//! \param[in] item
	//! \return a wrapper instance if a translation is needed, null otherwise
	protected TranslationPlugin_DataWrapper CreateDataWrapper(string id, notnull BaseContainer item)
	{
		if (id.IsEmpty())
			return null;

		bool skip;
		item.Get(m_MatchConfig.m_sSkipField, skip);
		if (skip)
			return null;

		map<string, string> translations = GetAllItemTranslations(item);
		if (!translations)
			return null;

		string text;
		bool isUpdate = true;
		if (translations.Find(SOURCE_EDITED_ID, text) && !text.IsEmpty())
		{
			if (m_eEditedEntriesMode == ETranslationPlugin_EditedRowsMode.PROCESS_UNEDITED_ONLY)
				return null;
		}
		else
		{
			if (m_eEditedEntriesMode == ETranslationPlugin_EditedRowsMode.PROCESS_EDITED_ONLY)
				return null;

			isUpdate = false;
			if (!translations.Find(SOURCE_ID, text) || text.IsEmpty())
				return null; // no source = no translation
		}

		array<string> locales = {};
		foreach (string key, string value : translations)
		{
			if (key == SOURCE_ID || key == SOURCE_EDITED_ID)
				continue;

			if (isUpdate || m_bOverwriteExistingTranslations || value.IsEmpty())
				locales.Insert(key);
		}

		if (locales.IsEmpty()) // all filled, no translation needed
			return null;

		TranslationPlugin_DataWrapper result = new TranslationPlugin_DataWrapper();
		result.m_bIsUpdate = isUpdate;
		result.m_Item = item;

		result.m_Query = new TranslationPluginQuery();
		result.m_Query.id = id;
		result.m_Query.text = text;

		result.m_Query.locales.InsertAll(locales);

		TranslationRequestMeta meta = new TranslationRequestMeta();

		int gender;
		if (item.Get(m_MatchConfig.m_sGenderField, gender))
		{
			if (gender == GENDER_MALE_INDEX)
				meta.gender = GENDER_MALE_VALUE;
			else
			if (gender == GENDER_FEMALE_INDEX)
				meta.gender = GENDER_FEMALE_VALUE;
			else
			if (gender == GENDER_OTHER_INDEX)
				meta.gender = GENDER_OTHER_VALUE;
		}

		item.Get(m_MatchConfig.m_sCommentField, meta.comment);
		item.Get(m_MatchConfig.m_sMaxLengthField, meta.maxLength);

		if (!meta.IsEmpty())
			result.m_Query.meta = meta;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \return locale-translation map + SOURCE_ID-translation / SOURCE_EDITED_ID-translation, never null
	protected map<string, string> GetAllItemTranslations(notnull BaseContainer item)
	{
		string prefix = m_MatchConfig.m_sLocaleFieldsPrefix;
		string suffix = m_MatchConfig.m_sLocaleFieldsSuffix;

		int prefixLength = prefix.Length();
		int suffixLength = suffix.Length();
		map<string, string> result = new map<string, string>();

		for (int i, count = item.GetNumVars(); i < count; ++i)
		{
			string varName = item.GetVarName(i);
			string translation;
			if (!item.Get(varName, translation))
				continue;

			// direct varName match - the source field may be formatted different than normal locale fields
			// e.g m_sSource vs m_sTarget_it_it
			if (varName == m_MatchConfig.m_sDefaultSource)
			{
				result.Insert(SOURCE_ID, translation);
				continue;
			}
			else
			if (varName == m_MatchConfig.m_sDefaultSourceEdited)
			{
				result.Insert(SOURCE_EDITED_ID, translation);
				continue;
			}

			string locale;
			if (prefixLength > 0 && varName != prefix && varName.StartsWith(prefix))
				locale = varName.Substring(prefixLength, varName.Length() - prefixLength);

			if (suffixLength > 0 && varName != suffix && varName.EndsWith(prefix))
				locale = varName.Substring(0, varName.Length() - suffixLength);

			if (locale.IsEmpty())
				continue;

			foreach (TranslationPluginConfigLocaleMatch localeMatch : m_MatchConfig.m_aLocalesDictionary)
			{
				if (localeMatch.m_sFileLocale == locale)
				{
					locale = localeMatch.m_sServerLocale;
					break;
				}
			}

			result.Insert(locale, translation);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Apply translations to the stringtable
	//! \param[in] stringEditor required to set the edits
	//! \param[in] response the server's response
	//! \return number of changed rows
	protected int ApplyTranslations(notnull LocalizationEditor stringEditor, notnull TranslationPluginResponse response)
	{
		int modifiedRows;
		bool hasBegunModifying;

		string prefix = m_MatchConfig.m_sLocaleFieldsPrefix;
		string suffix = m_MatchConfig.m_sLocaleFieldsSuffix;

		foreach (TranslationPluginResult result : response.results)
		{
			// we use the idWrapperMap to only pick what was requested (do not trust the server blindly)
			TranslationPlugin_DataWrapper wrapper = m_mIdDataMap.Get(result.id);
			if (!wrapper)
			{
				if (m_bLogNetwork)
					PrintFormat("Did NOT ask for %1, received it anyway", result.id, level: LogLevel.NORMAL);

				continue;
			}

			array<string> languagesInResponse = {};

			int modifiedLanguages;
			array<string> localesToUse;
			if (m_aGlobalLocales)
				localesToUse = m_aGlobalLocales;
			else
				localesToUse = wrapper.m_Query.locales;

			foreach (string locale : localesToUse)
			{
				string translation = GetTranslationFromLocale(locale, result.translations);
				int length = translation.Length();

				if (length < 1)
				{
					if (m_bLogNetwork)
						PrintFormat("Asked for %1's %2 translation, did not obtain it", wrapper.m_Query.id, locale, level: LogLevel.WARNING);

					continue;
				}

				if (wrapper.m_Query.meta && wrapper.m_Query.meta.maxLength > 0 && length > wrapper.m_Query.meta.maxLength)
				{
					if (m_bLogChanges)
						PrintFormat("%1's %2 translation is too long: %3 chars > max %4 chars", result.id, locale, length, wrapper.m_Query.meta.maxLength, level: LogLevel.WARNING);
				}

				if (!hasBegunModifying)
				{
					stringEditor.BeginModify(PLUGIN_NAME);
					hasBegunModifying = true;
				}

				stringEditor.ModifyProperty(wrapper.m_Item, wrapper.m_Item.GetVarIndex(prefix + locale + suffix), translation);
				if (m_bLogChanges)
					PrintFormat("Updating %1's %2 locale", result.id, locale, level: LogLevel.NORMAL);

				++modifiedLanguages;
			}

			if (m_bSetEditedAsOriginal && wrapper.m_bIsUpdate && !m_MatchConfig.m_sDefaultSourceEdited.IsEmpty())
			{
				if (!hasBegunModifying)
				{
					stringEditor.BeginModify(PLUGIN_NAME);
					hasBegunModifying = true;
				}

				string updatedTranslation;
				if (wrapper.m_Item.Get(m_MatchConfig.m_sDefaultSourceEdited, updatedTranslation) && !updatedTranslation.IsEmpty())
				{
					stringEditor.ModifyProperty(wrapper.m_Item, wrapper.m_Item.GetVarIndex(m_MatchConfig.m_sDefaultSource), updatedTranslation);
					stringEditor.ModifyProperty(wrapper.m_Item, wrapper.m_Item.GetVarIndex(m_MatchConfig.m_sDefaultSourceEdited), "");

					if (m_bLogChanges)
						PrintFormat(
							"Updating %1's %2 locale to %3 locale's value",
							result.id,
							m_MatchConfig.m_sDefaultSource,
							m_MatchConfig.m_sDefaultSourceEdited,
							level: LogLevel.NORMAL);

					++modifiedLanguages;
				}
			}

			if (modifiedLanguages > 0)
				++modifiedRows;
		}

		if (hasBegunModifying)
			stringEditor.EndModify();

		return modifiedRows;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the holder's translation corresponding to the locale
	//! \param[in] locale e.g fr_fr
	//! \param[in] holder the translation holder to parse
	//! \return the translation, empty string if locale is not found or on error
	protected static string GetTranslationFromLocale(string locale, notnull TranslationPluginResultHolder holder)
	{
		Resource resource = BaseContainerTools.CreateContainerFromInstance(holder);
		if (resource && resource.IsValid())
		{
			BaseContainer baseContainer = resource.GetResource().ToBaseContainer();
			if (baseContainer)
			{
				string result;
				if (baseContainer.Get(locale, result))
					return result;
			}
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Reset variables to brand new
	protected void ResetRequestState()
	{
		m_mIdDataMap = null;
		m_iTranslationQueriesCount = 0;
		m_aGlobalLocales = null;

		m_bWaitingOnRestAPI = false;
		m_iLastUsage = 0;
		m_RestCallback = null;
		m_ProgressBar = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a fake response based on requested ids and locales
	//! \return a response that could come from JSON
	protected TranslationPluginResponse GetFakeResponse()
	{
		TranslationPluginResponse response = new TranslationPluginResponse();
		response.results = {};

		foreach (string id, TranslationPlugin_DataWrapper wrapper : m_mIdDataMap)
		{
			TranslationPluginResultHolder translations = new TranslationPluginResultHolder();
			Resource resource = BaseContainerTools.CreateContainerFromInstance(translations);
			if (!resource || !resource.IsValid())
				continue;

			BaseContainer container = resource.GetResource().ToBaseContainer();
			if (!container)
				continue;

			for (int i, count = container.GetNumVars(); i < count; ++i)
			{
				string varName = container.GetVarName(i);
				string varNameUpper = varName;
				varNameUpper.ToUpper();
				container.Set(varName, varNameUpper + " translation");
			}

			BaseContainerTools.WriteToInstance(translations, container);

			TranslationPluginResult result = new TranslationPluginResult();
			result.id = id;
			result.translations = translations;

			response.results.Insert(result);
		}

		return response;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] url in format http(s)://anything(/)(?xyz), e.g:
	//! - 127.0.0.1						becomes https://127.0.0.1
	//! - localhost						becomes https://localhost
	//! - ://localhost					becomes https://localhost
	//! - http://localhost
	//! - http://localhost:80
	//! - http://localhost?query
	//! - http://localhost/?query
	//! - https://localhost/?query
	//! - https://localhost/test?query	becomes https:// + localhost/test + ?query
	//! - http:/localhost				= error
	//! - ftps://localhost				= error
	//! \param[out] protocol http:// or https://
	//! \see ACCEPTED_PROTOCOLS
	//! \param[out] address the API's address e.g https://localhost/path/to/api, NOT server root address
	//! \param[out] query what comes after the question mark in the URL (e.g mode=translate&version=1.5, without question mark)
	//! \return true on successfully splitting address and query (even if query is empty), false otherwise
	protected static bool SplitURL(string url, out string protocol, out string address, out string query)
	{
		url.TrimInPlace();
		if (url.IsEmpty())
			return false;

		int protocolSeparatorIndex = url.IndexOf(PROTOCOL_SEPARATOR);
		if (protocolSeparatorIndex < 0)
		{
			url = DEFAULT_PROTOCOL + PROTOCOL_SEPARATOR + url;
			protocolSeparatorIndex = 5;
		}
		else
		if (url.StartsWith(PROTOCOL_SEPARATOR))
		{
			url = DEFAULT_PROTOCOL + url;
			protocolSeparatorIndex = 5;
		}

		bool isProtocolValid;
		foreach (string acceptedProtocol : ACCEPTED_PROTOCOLS)
		{
			if (url.StartsWith(acceptedProtocol))
			{
				isProtocolValid = true;
				break;
			}
		}

		if (!isProtocolValid)
			return false;

		int addressIndex = protocolSeparatorIndex + PROTOCOL_SEPARATOR.Length();
		int urlLength = url.Length();

		if (urlLength == addressIndex)
			return false;

		int qIndex = url.IndexOfFrom(addressIndex, "?");
		if (qIndex > -1 && qIndex < addressIndex)
			return false;

		protocol = url.Substring(0, addressIndex);

		if (qIndex < 0)
		{
			address = url.Substring(addressIndex, urlLength - addressIndex);
			query = "";
		}
		else
		{
			address = url.Substring(addressIndex, qIndex - addressIndex);
			if (qIndex + 1 == urlLength) // ending with "?"
				query = "";
			else
				query = url.Substring(qIndex + 1, urlLength - qIndex - 1);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sanitise the provided token to prevent a potential headers exploit
	//! \param[in] token the token to sanitise
	//! \return the sanitised token format ^[a-zA-Z0-9-._~+/=]$, or empty string on bad/empty token
	static string SanitizeToken(string token)
	{
		string result;
		for (int i, length = token.Length(); i < length; ++i)
		{
			if (TOKEN_WHITELIST.Contains(token[i]))
				result += token[i];
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] localesA
	//! \param[in] localesB
	//! \return true if arrays are identical, false otherwise (casing and order matter)
	protected static bool AreLocalesEqual(notnull array<string> localesA, notnull array<string> localesB)
	{
		if (localesA.Count() != localesB.Count())
			return false;

		foreach (int i, string localeA : localesA)
		{
			if (localeA != localesB[i])
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Set a config with the attributes's default values
	protected void ResetConfig()
	{
		typename configTypeName = TranslationPluginMatchConfig;
		Resource resource = BaseContainerTools.CreateContainer(configTypeName.ToString());
		BaseContainer baseContainer = resource.GetResource().ToBaseContainer();
		Managed managed = BaseContainerTools.CreateInstanceFromContainer(baseContainer);
		m_MatchConfig = TranslationPluginMatchConfig.Cast(managed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceName the config to load
	//! \return true on success, false otherwise
	protected bool LoadConfig(ResourceName resourceName)
	{
		if (resourceName.IsEmpty())
		{
			PrintDialog("Please provide the Config File Path field in order to load something.", PLUGIN_NAME, LogLevel.WARNING);
			return false;
		}

		Resource resource = Resource.Load(resourceName);
		if (!resource.IsValid())
		{
			PrintDialog("The provided config is not a valid resource.", PLUGIN_NAME, LogLevel.WARNING);
			return false;
		}

		BaseContainer baseContainer = resource.GetResource().ToBaseContainer();
		if (!baseContainer)
		{
			PrintDialog("The provided config does not have a base container.", PLUGIN_NAME, LogLevel.WARNING);
			return false;
		}

		Managed managed = BaseContainerTools.CreateInstanceFromContainer(baseContainer);
		if (!managed)
		{
			PrintDialog("The provided config cannot be instanciated.", PLUGIN_NAME, LogLevel.WARNING);
			return false;
		}

		TranslationPluginMatchConfig result = TranslationPluginMatchConfig.Cast(managed);
		if (!result)
		{
			PrintDialog("The provided config is not a valid TranslationPluginMatchConfig.", PLUGIN_NAME, LogLevel.WARNING);
			return false;
		}

		m_MatchConfig = result;

		// do not PrintDialog loading confirmation
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		if (Workbench.ScriptDialog(
			PLUGIN_NAME,
			"Set the file parsing and communication options here."
			+ "\nMandatory fields are Server URL, Match Config, and in Match Config: Id and Default Source.",
			this) == 1)
			Configure(); // if Configure* methods return false, reopen the Configuration panel
	}

	//------------------------------------------------------------------------------------------------
	//! \return always true
	protected bool ConfigureClose()
	{
		if (!m_MatchConfig) // quit deleting it, dangit
			ResetConfig();

		string sanitisedToken = SanitizeToken(m_sServerToken);
		if (m_sServerToken != sanitisedToken)
		{
			PrintDialog("The token you provided was sanitised.", PLUGIN_NAME, LogLevel.WARNING);
			m_sServerToken = sanitisedToken;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if everything went well, false otherwise (and need to reopen Configure())
	protected bool ConfigureValidate()
	{
		array<string> errors = {};

		m_sServerURL.TrimInPlace();
		if (m_sServerURL.IsEmpty())
			errors.Insert("the Server URL field is empty");

		if (m_sServerToken != SanitizeToken(m_sServerToken))
			errors.Insert("the Server Token value is invalid; be sure to remove any invalid characters (allowed: a-zA-Z0-9-._~+/=)");

		if (m_MatchConfig)
		{
			m_MatchConfig.m_sIdField.TrimInPlace();
			if (m_MatchConfig.m_sIdField.IsEmpty())
				errors.Insert("the Id field is empty");

			m_MatchConfig.m_sDefaultSource.TrimInPlace();
			if (m_MatchConfig.m_sDefaultSource.IsEmpty())
				errors.Insert("the Default Source field is empty");
		}
		else
		{
			errors.Insert("the Match Config field is null");
			ResetConfig();
		}

		if (!errors.IsEmpty())
		{
			string message = "Mandatory fields are missing:";
			foreach (string error : errors)
			{
				message += "\n- " + error;
			}

			PrintDialog(message, PLUGIN_NAME + " configuration", LogLevel.WARNING);
			return false;
		}

		PrintDialog("Configuration seems correct.", PLUGIN_NAME + " configuration", LogLevel.NORMAL);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return always true
	protected bool ConfigureLoadConfig()
	{
		TranslationPlugin_LoadConfigUI loadConfigUI = new TranslationPlugin_LoadConfigUI();
		while (true) // ugh
		{
			if (Workbench.ScriptDialog(PLUGIN_NAME, "Load a string table match config", loadConfigUI) == 0) // Cancel
				break;

			if (loadConfigUI.m_sConfigFilePath.IsEmpty())
				PrintDialog("Please provide a Config file path (or press Cancel to abort)", PLUGIN_NAME, LogLevel.WARNING);
			else
			if (LoadConfig(loadConfigUI.m_sConfigFilePath)) // LoadConfig has its own PrintDialog
				break;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Load Config")]
	protected int ButtonLoadConfig()
	{
		ConfigureLoadConfig();
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Validate/Close")]
	protected int ButtonValidateClose()
	{
		if (ConfigureValidate())
			return 0;
		else
			return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected int ButtonClose()
	{
		ConfigureClose();
		return 0;
	}

	//
	//	helpful methods
	//

	//------------------------------------------------------------------------------------------------
	//! Print and show a Workbench dialog
	//! \param[in] message
	//! \param[in] caption
	//! \param[in] level
	protected static void PrintDialog(string message, string caption, LogLevel level)
	{
		PrintFormat("[%1] %2", caption, message, level: LogLevel.NORMAL);
		Workbench.Dialog(caption, message);
	}

	//------------------------------------------------------------------------------------------------
	//! Print and show a Workbench dialog
	//! \param[in] message
	//! \param[in] param1
	//! \param[in] param2
	//! \param[in] param3
	//! \param[in] caption
	//! \param[in] level
	protected static void PrintFormatDialog(string message, string param1, string param2 = "", string param3 = "", string caption = "", LogLevel level = LogLevel.WARNING)
	{
		message = string.Format(message, param1, param2, param3);
		PrintFormat("[%1] %2", caption, message, level: level);
		Workbench.Dialog(caption, message);
	}

	//------------------------------------------------------------------------------------------------
	//! Format provided milliseconds into user-friendly time
	//! \param[in] milliSeconds the absolute value is taken
	//! \return a string in format "00:00:00", "00:00", "x0.0s" or "no time" depending on time (respectively >= 60 min, >= 60s, anything else but 0, 0)
	protected static string FormatDurationMs(int milliSeconds)
	{
		if (milliSeconds == 0)
			return "no time";

		if (milliSeconds < 0)
			milliSeconds = -milliSeconds;

		if (milliSeconds < 60000)
			return string.Format("%1s", (milliSeconds * 0.001).ToString(lenDec: 1));

		int totalSeconds = Math.Round(milliSeconds * 0.001);

		int hours = totalSeconds / 3600;
		int minutes = (totalSeconds - hours * 3600) / 60;
		int seconds = (totalSeconds - hours * 3600 - minutes * 60);

		if (hours < 1)
			return string.Format("%1:%2", minutes.ToString(2), seconds.ToString(2));

		return string.Format("%1:%2:%3", hours, minutes.ToString(2), seconds.ToString(2));
	}

	//------------------------------------------------------------------------------------------------
	override void OnStringTableItemContextMenu()
	{
		Run();
	}
}

class TranslationPlugin_LoadConfigUI
{
	[Attribute(
		defvalue: DEFAULT_MATCH_CONFIG,
		desc: "Config to load into the Match Config field using the Load Config button - overrides any existing config",
		category: "Advanced",
		params: "conf class=TranslationPluginMatchConfig")]
	ResourceName m_sConfigFilePath;

	protected static const ResourceName DEFAULT_MATCH_CONFIG = "{9414DBD68A4429DB}Configs/Workbench/LocalizationEditor/TranslatePlugin/BaseStringTableItemConfig.conf";

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Load", true)]
	protected int ButtonLoad()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}

	void TranslationPlugin_LoadConfigUI()
	{
		if (m_sConfigFilePath.IsEmpty())
			m_sConfigFilePath = DEFAULT_MATCH_CONFIG;
	}
}

class TranslationPlugin_DataWrapper
{
	BaseContainer m_Item;
	bool m_bIsUpdate;

	ref TranslationPluginQuery m_Query;
}

enum ETranslationPlugin_EditedRowsMode
{
	PROCESS_ALL,
	PROCESS_UNEDITED_ONLY,
	PROCESS_EDITED_ONLY,
}

enum ETranslationPlugin_ProcessMode
{
	PRODUCTION,			//!< normal process
	SIMULATE_SUCCESS,	//!< simulate proper communication - provides translation results!
	SIMULATE_ERROR_403,	//!< Forbidden
	SIMULATE_ERROR_404,	//!< Not Found
	SIMULATE_ERROR_408,	//!< Request Timeout
	SIMULATE_ERROR_418,	//!< he he
	SIMULATE_ERROR_500,	//!< Internal Server Error
	SIMULATE_ERROR_501,	//!< Not Implemented
}
#endif
