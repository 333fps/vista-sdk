/**
 * @file LocationParsingErrorBuilder.inl
 * @brief Inline implementations for performance-critical LocationParsingErrorBuilder operations
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// LocationParsingErrorBuilder class
	//=====================================================================

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	inline bool LocationParsingErrorBuilder::hasError() const
	{
		return !m_errors.empty();
	}
}
