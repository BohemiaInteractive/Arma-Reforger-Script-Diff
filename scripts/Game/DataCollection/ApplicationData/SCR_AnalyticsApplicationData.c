// These are structs which are sent as tables to analytics. The attributes are reflected as columns in 
// said tables. The column name is the same as the attribute name, therefore we need to name our attributes
// the same as we want them in our analytical tables.
//
// SCR_AnalyticsData
// |- SCR_InventoryData: cnt_inv_interactions_vertical, cnt_inv_interactions_character_storage, ...
// |- SCR_VotingResultData: flag_vote_success
// |- SCR_TimeSpentData: amt_time_spent
//    |- SCR_CourseEndData: amt_course_name, amt_course_end_reason
//    |- SCR_SettingsTimeSpentData: array_settings_changed
//    |- SCR_MenuTimeSpentData: cnt_menu_reopen
//       |- SCR_FieldManualTimeSpentData: amt_name_location
//       |- SCR_MenuInteractedData: array_menu_interactions
class SCR_AnalyticsData : Managed
{
	//------------------------------------------------------------------------------------------------
	//! \return formatted string that contains class name
	string ToPrettyString()
	{
		return string.Format("%1{%2}", ClassName(), GetData());
	}

	//------------------------------------------------------------------------------------------------
	//! \return data only without class name, can be overloaded to include childrens' fields.
	string GetData();
}

class SCR_InventoryData : SCR_AnalyticsData
{
	int cnt_inv_interactions_vertical;
	int cnt_inv_interactions_character_storage;
	int cnt_inv_interactions_horizontal;
	int cnt_inv_interactions_vicinity;
	int cnt_inv_examination;
	int cnt_inv_healing;
	string array_inv_weight_on_open;
	string array_inv_weight_on_close;
	int amt_inv_time_spent;
	
	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		string result = "cnt_inv_interactions_vertical=" + cnt_inv_interactions_vertical.ToString();
		result += ",cnt_inv_interactions_character_storage=" + cnt_inv_interactions_character_storage.ToString();
		result += ",cnt_inv_interactions_horizontal=" + cnt_inv_interactions_horizontal.ToString();
		result += ",cnt_inv_interactions_vicinity=" + cnt_inv_interactions_vicinity.ToString();
		result += ",cnt_inv_examination=" + cnt_inv_examination.ToString();
		result += ",cnt_inv_healing=" + cnt_inv_healing.ToString();
		result += ",array_inv_weight_on_open=" + array_inv_weight_on_open;
		result += ",array_inv_weight_on_close=" + array_inv_weight_on_close;
		result += ",amt_inv_time_spent=" + amt_inv_time_spent.ToString();
		return result;
	}
}

class SCR_VotingResultData : SCR_AnalyticsData
{
	bool flag_vote_success;
	
	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		return string.Format("flag_vote_success=%1", flag_vote_success);
	} 
}

class SCR_TimeSpentData : SCR_AnalyticsData
{
	int amt_time_spent;
	
	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		return string.Format("amt_time_spend=%1", amt_time_spent);
	}
}

class SCR_CourseEndData : SCR_TimeSpentData
{
	string amt_course_name;
	string amt_course_end_reason;

	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		return string.Format("%1, amt_course_name=%2, amt_course_end_reason=%3", super.GetData(), amt_course_name, amt_course_end_reason);
	}
}

class SCR_SettingsTimeSpentData : SCR_TimeSpentData
{
	string array_settings_changed;

	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		return string.Format("%1, array_settings_changed=%2", super.GetData(), array_settings_changed);
	}
}

class SCR_MenuTimeSpentData : SCR_TimeSpentData
{
	int cnt_menu_reopen;
	
	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		return string.Format("%1, cnt_menu_reopen=%2", super.GetData(), cnt_menu_reopen);
	}
}

class SCR_FieldManualTimeSpentData : SCR_MenuTimeSpentData
{
	string amt_name_location;

	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		return string.Format("%1, amt_name_location='%2'", super.GetData(), amt_name_location);
	}
}

class SCR_MenuInteractedData : SCR_MenuTimeSpentData
{
	string array_menu_interactions;

	//------------------------------------------------------------------------------------------------
	protected override string GetData()
	{
		return string.Format("%1, array_menu_interactions=%2", super.GetData(), array_menu_interactions);
	}
}

