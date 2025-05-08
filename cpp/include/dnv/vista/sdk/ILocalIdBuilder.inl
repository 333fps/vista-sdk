/**
 * @file ILocalIdBuilder.inl
 * @brief Inline definitions for the ILocalIdBuilder template class.
 * @details This file contains the implementation of template methods declared in ILocalIdBuilder.h.
 *          It should only be included at the end of ILocalIdBuilder.h.
 */

namespace dnv::vista::sdk
{
	//=========================================================================
	// Inline Operator Definitions
	//=========================================================================

	template <typename TBuilder, typename TResult>
	inline bool ILocalIdBuilder<TBuilder, TResult>::operator==( const TBuilder& other ) const noexcept
	{
		return equals( other );
	}

	template <typename TBuilder, typename TResult>
	inline bool ILocalIdBuilder<TBuilder, TResult>::operator!=( const TBuilder& other ) const noexcept
	{
		return !equals( other );
	}

	//=========================================================================
	// Inline Static Parsing Method Definitions
	//=========================================================================

	template <typename TBuilder, typename TResult>
	inline TBuilder ILocalIdBuilder<TBuilder, TResult>::parse( std::string_view localIdStr ) // Changed
	{
		SPDLOG_TRACE( "ILocalIdBuilder::parse delegating to {}: {}", typeid( TBuilder ).name(), std::string( localIdStr ) ); // May need explicit conversion for SPDLOG if it doesn't handle string_view directly

		return TBuilder::parse( localIdStr );
	}

	template <typename TBuilder, typename TResult>
	inline bool ILocalIdBuilder<TBuilder, TResult>::tryParse( std::string_view localIdStr, std::optional<TBuilder>& builder ) // Changed
	{
		SPDLOG_TRACE( "ILocalIdBuilder::tryParse (no errors) delegating to {}: {}", typeid( TBuilder ).name(), std::string( localIdStr ) ); // May need explicit conversion for SPDLOG

		return TBuilder::tryParse( localIdStr, builder );
	}

	template <typename TBuilder, typename TResult>
	inline bool ILocalIdBuilder<TBuilder, TResult>::tryParse( std::string_view localIdStr, ParsingErrors& errors, std::optional<TBuilder>& builder ) // Changed
	{
		SPDLOG_TRACE( "ILocalIdBuilder::tryParse (with errors) delegating to {}: {}", typeid( TBuilder ).name(), std::string( localIdStr ) ); // May need explicit conversion for SPDLOG

		return TBuilder::tryParse( localIdStr, errors, builder );
	}
}
