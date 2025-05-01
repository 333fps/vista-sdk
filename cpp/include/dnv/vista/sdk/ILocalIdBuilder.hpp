/**
 * @file ILocalIdBuilder.hpp
 * @brief Template implementation helpers for ILocalIdBuilder.
 */

#pragma once

#include "ILocalIdBuilder.h"

namespace dnv::vista::sdk::detail
{
	/**
	 * @brief Helper struct to check if a class has static parse method.
	 */
	template <typename T>
	class HasStaticParse
	{
		template <typename C>
		static auto test( int ) -> decltype( C::parse( std::declval<const std::string&>() ),
			std::true_type{} );

		template <typename>
		static auto test( ... ) -> std::false_type;

	public:
		static constexpr bool value = decltype( test<T>( 0 ) )::value;
	};

	/**
	 * @brief Helper struct to check if a class has static tryParse method.
	 */
	template <typename T>
	class HasStaticTryParse
	{
		template <typename C>
		static auto test( int ) -> decltype( C::tryParse( std::declval<const std::string&>(),
												 std::declval<std::optional<T>&>() ),
			std::true_type{} );

		template <typename>
		static auto test( ... ) -> std::false_type;

	public:
		static constexpr bool value = decltype( test<T>( 0 ) )::value;
	};

	/**
	 * @brief Helper struct to check if a class has extended static tryParse method.
	 */
	template <typename T>
	class HasStaticExtendedTryParse
	{
		template <typename C>
		static auto test( int ) -> decltype( C::tryParse( std::declval<const std::string&>(),
												 std::declval<ParsingErrors&>(),
												 std::declval<std::optional<T>&>() ),
			std::true_type{} );

		template <typename>
		static auto test( ... ) -> std::false_type;

	public:
		static constexpr bool value = decltype( test<T>( 0 ) )::value;
	};
}

namespace dnv::vista::sdk
{
	template <typename TBuilder, typename TResult>
	TBuilder ILocalIdBuilder<TBuilder, TResult>::parse( const std::string& localIdStr )
	{
		static_assert( detail::HasStaticParse<TBuilder>::value,
			"Derived class must implement static TBuilder parse(const std::string&)" );

		auto startTime = std::chrono::high_resolution_clock::now();
		SPDLOG_INFO( "Parsing LocalId from string: {}", localIdStr );

		auto result = TBuilder::parse( localIdStr );

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
		SPDLOG_INFO( "Parsing completed in {}μs", duration );

		return std::move( result );
	}

	template <typename TBuilder, typename TResult>
	bool ILocalIdBuilder<TBuilder, TResult>::tryParse(
		const std::string& localIdStr,
		std::optional<TBuilder>& localId )
	{
		static_assert( detail::HasStaticTryParse<TBuilder>::value,
			"Derived class must implement static bool tryParse(const std::string&, std::optional<TBuilder>&)" );

		auto startTime = std::chrono::high_resolution_clock::now();
		SPDLOG_INFO( "Attempting to parse LocalId from string: {}", localIdStr );

		bool result = TBuilder::tryParse( localIdStr, localId );

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( endTime - startTime )
							.count();
		SPDLOG_INFO( "Parse attempt completed in {}μs, success: {}", duration, result );

		return result;
	}

	template <typename TBuilder, typename TResult>
	bool ILocalIdBuilder<TBuilder, TResult>::tryParse(
		const std::string& localIdStr,
		ParsingErrors& errors,
		std::optional<TBuilder>& localId )
	{
		static_assert( detail::HasStaticExtendedTryParse<TBuilder>::value,
			"Derived class must implement static bool tryParse(const std::string&, ParsingErrors&, std::optional<TBuilder>&)" );

		auto startTime = std::chrono::high_resolution_clock::now();
		SPDLOG_INFO( "Attempting to parse LocalId from string with error handling: {}", localIdStr );

		bool result = TBuilder::tryParse( localIdStr, errors, localId );

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
			endTime - startTime )
							.count();

		if ( result )
		{
			SPDLOG_DEBUG( "Parsing succeeded in {}μs", duration );
		}
		else
		{
			SPDLOG_ERROR( "Parsing failed in {}μs with errors: {}",
				duration, errors.toString() );
		}

		return result;
	}
}
