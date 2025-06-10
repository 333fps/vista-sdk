/**
 * @file Gmod.inl
 * @brief Inline implementations for performance-critical Gmod operations
 */

namespace dnv::vista::sdk
{
	namespace internal
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr std::string_view NODE_CATEGORY_PRODUCT = "PRODUCT";
		static constexpr std::string_view NODE_CATEGORY_ASSET = "ASSET";
		static constexpr std::string_view NODE_CATEGORY_ASSET_FUNCTION = "ASSET FUNCTION";
		static constexpr std::string_view NODE_CATEGORY_PRODUCT_FUNCTION = "PRODUCT FUNCTION";

		static constexpr std::string_view NODE_TYPE_SELECTION = "SELECTION";
		static constexpr std::string_view NODE_TYPE_GROUP = "GROUP";
		static constexpr std::string_view NODE_TYPE_LEAF = "LEAF";
		static constexpr std::string_view NODE_TYPE_TYPE = "TYPE";
		static constexpr std::string_view NODE_TYPE_COMPOSITION = "COMPOSITION";

		static constexpr std::string_view NODE_FULLTYPE_ASSET_FUNCTION_LEAF = "ASSET FUNCTION LEAF";
		static constexpr std::string_view NODE_FULLTYPE_PRODUCT_FUNCTION_LEAF = "PRODUCT FUNCTION LEAF";

		static constexpr std::string_view KEYWORD_FUNCTION = "FUNCTION";
		static constexpr std::string_view KEYWORD_PRODUCT = "PRODUCT";
	}

	//=====================================================================
	// Gmod class
	//=====================================================================

	//----------------------------------------------
	// Lookup operators
	//----------------------------------------------

	inline const GmodNode& Gmod::operator[]( std::string_view key ) const
	{
		const GmodNode* nodePtr = nullptr;
		bool found = m_nodeMap.tryGetValue( key, nodePtr );
		if ( found && nodePtr != nullptr )
		{
			return *nodePtr;
		}

		throw std::out_of_range( fmt::format( "Key not found in Gmod node map: {}", key ) );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline VisVersion Gmod::visVersion() const
	{
		return m_visVersion;
	}

	inline const GmodNode& Gmod::rootNode() const
	{
		if ( !m_rootNode )
		{
			throw std::runtime_error( "Root node is not initialized or 'VE' was not found." );
		}

		return *m_rootNode;
	}

	//----------------------------------------------
	// Static state inspection methods
	//----------------------------------------------

	inline bool Gmod::isPotentialParent( const std::string& type )
	{

		return type == internal::NODE_TYPE_SELECTION ||
			   type == internal::NODE_TYPE_GROUP ||
			   type == internal::NODE_TYPE_LEAF;
	}

	inline bool Gmod::isLeafNode( const GmodNodeMetadata& metadata )
	{
		const auto& fullType = metadata.fullType();

		return fullType == internal::NODE_FULLTYPE_ASSET_FUNCTION_LEAF ||
			   fullType == internal::NODE_FULLTYPE_PRODUCT_FUNCTION_LEAF;
	}

	inline bool Gmod::isFunctionNode( const GmodNodeMetadata& metadata )
	{
		const auto& category = metadata.category();

		return category != internal::NODE_CATEGORY_PRODUCT &&
			   category != internal::NODE_CATEGORY_ASSET;
	}

	inline bool Gmod::isProductSelection( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == internal::NODE_CATEGORY_PRODUCT &&
			   metadata.type() == internal::NODE_TYPE_SELECTION;
	}

	inline bool Gmod::isProductType( const GmodNodeMetadata& metadata )
	{

		return metadata.category() == internal::NODE_CATEGORY_PRODUCT &&
			   metadata.type() == internal::NODE_TYPE_TYPE;
	}

	inline bool Gmod::isAsset( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == internal::NODE_CATEGORY_ASSET;
	}

	inline bool Gmod::isAssetFunctionNode( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == internal::NODE_CATEGORY_ASSET_FUNCTION;
	}

	inline bool Gmod::isProductTypeAssignment( const GmodNode* parent, const GmodNode* child ) noexcept
	{
		if ( !parent || !child )
		{
			return false;
		}

		const auto& parentCategory = parent->metadata().category();
		const auto& childCategory = child->metadata().category();
		const auto& childType = child->metadata().type();

		if ( parentCategory.find( internal::KEYWORD_FUNCTION ) == std::string_view::npos )
		{
			return false;
		}

		return childCategory == internal::NODE_CATEGORY_PRODUCT &&
			   childType == internal::NODE_TYPE_TYPE;
	}

	inline bool Gmod::isProductSelectionAssignment( const GmodNode* parent, const GmodNode* child )
	{
		if ( !parent || !child ) [[unlikely]]
		{
			return false;
		}

		const auto& parentCategory = parent->metadata().category();
		const auto& childCategory = child->metadata().category();
		const auto& childType = child->metadata().type();

		if ( parentCategory.find( internal::KEYWORD_FUNCTION ) == std::string_view::npos )
		{
			return false;
		}

		return childCategory.find( internal::KEYWORD_PRODUCT ) != std::string_view::npos &&
			   childType == internal::NODE_TYPE_SELECTION;
	}
}
