/**
 * @file Codebook.inl
 * @brief Inline implementations for performance-critical codebook operations
 * @details Contains performance-critical inline implementations for
 *          fast codebook access operations.
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// CodebookStandardValues class
	//=====================================================================

	//----------------------------------------------
	// Public methods
	//----------------------------------------------

	inline size_t CodebookStandardValues::count() const noexcept
	{
		return m_standardValues.size();
	}

	//----------------------------------------------
	// Iteration
	//----------------------------------------------

	inline CodebookStandardValues::Iterator CodebookStandardValues::begin() const noexcept
	{
		return m_standardValues.begin();
	}

	inline CodebookStandardValues::Iterator CodebookStandardValues::end() const noexcept
	{
		return m_standardValues.end();
	}

	//=====================================================================
	// CodebookGroups class
	//=====================================================================

	//----------------------------------------------
	// Public methods
	//----------------------------------------------

	inline size_t CodebookGroups::count() const noexcept
	{
		return m_groups.size();
	}

	inline bool CodebookGroups::contains( std::string_view group ) const noexcept
	{
		return m_groups.contains( group );
	}

	//----------------------------------------------
	// Iteration
	//----------------------------------------------

	inline CodebookGroups::Iterator CodebookGroups::begin() const noexcept
	{
		return m_groups.begin();
	}

	inline CodebookGroups::Iterator CodebookGroups::end() const noexcept
	{
		return m_groups.end();
	}

	//=====================================================================
	// Codebook class
	//=====================================================================

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline const CodebookGroups& Codebook::groups() const noexcept
	{
		return m_groups;
	}

	inline const CodebookStandardValues& Codebook::standardValues() const noexcept
	{
		return m_standardValues;
	}

	inline const std::unordered_map<std::string, std::vector<std::string>>& Codebook::rawData() const noexcept
	{
		return m_rawData;
	}

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	inline bool Codebook::hasGroup( std::string_view group ) const noexcept
	{
		return m_groups.contains( group );
	}

	inline bool Codebook::hasStandardValue( std::string_view value ) const noexcept
	{
		return m_standardValues.contains( value );
	}
}
