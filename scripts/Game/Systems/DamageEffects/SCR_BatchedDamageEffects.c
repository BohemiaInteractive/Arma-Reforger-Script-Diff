class SCR_BatchedDamageEffects
{
	//------------------------------------------------------------------------------------------------
	//! Method called locally by SCR_DamageSufferingSystem to transfer newly batched data from shared space to this container
	//! \param[in,out] newBatchedDataContainer to transfer from
	void CopyBatchedData(notnull SCR_BatchedDamageEffects newBatchedDataContainer);
}
