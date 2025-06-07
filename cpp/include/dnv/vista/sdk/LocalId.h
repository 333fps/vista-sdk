/**
 * @file LocalId.h
 * @brief High-performance, immutable Local ID implementation.
 * @details Concrete implementation of a VIS Local ID with direct value storage
 *          for optimal performance and zero-overhead property access.
 */

#pragma once

#include "LocalIdBuilder.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class GmodPath;
	class MetadataTag;
	class ParsingErrors;
	enum class VisVersion;

	//=====================================================================
	// LocalId class
	//=====================================================================

	/**
	 * @class LocalId
	 * @brief Immutable representation of a VIS Local ID with optimal performance.
	 *
	 * @details Represents a validated Local ID according to VIS standard (ISO 19848).
	 * Uses direct value storage for zero-overhead access. All instances are immutable
	 * and thread-safe for concurrent read access.
	 *
	 * Construction via LocalIdBuilder or static parse() methods.
	 * All property accessors are inline and noexcept for maximum performance.
	 */
	class LocalId final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructs LocalId from validated LocalIdBuilder.
		 * @param[in] builder Valid LocalIdBuilder instance (moved).
		 * @throws std::invalid_argument If builder is invalid or empty.
		 */
		explicit LocalId( LocalIdBuilder builder );

		/** @brief Copy constructor */
		LocalId( const LocalId& other ) = default;

		/** @brief Move constructor */
		LocalId( LocalId&& other ) noexcept = default;

		/** @brief Destructor */
		~LocalId() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocalId& operator=( const LocalId& other ) = default;

		/** @brief Move assignment operator */
		LocalId& operator=( LocalId&& other ) noexcept = default;

		//----------------------------------------------
		// Lookup operators
		//----------------------------------------------

		/**
		 * @brief Performs deep equality comparison.
		 * @param[in] other LocalId to compare against.
		 * @return true if semantically equal.
		 */
		[[nodiscard]] bool equals( const LocalId& other ) const
		{
			return m_builder == other.m_builder;
		}

		//----------------------------------------------
		// Core property accessors
		//----------------------------------------------

		/**
		 * @brief Gets direct access to underlying LocalIdBuilder.
		 * @return Const reference to internal builder.
		 * @note Zero-overhead access via direct reference.
		 */
		[[nodiscard]] const LocalIdBuilder& builder() const noexcept
		{
			return m_builder;
		}

		/**
		 * @brief Gets VIS version of this Local ID.
		 * @return VIS version enum value.
		 * @note Guaranteed to contain value for valid LocalId.
		 */
		[[nodiscard]] VisVersion visVersion() const noexcept
		{
			return m_builder.visVersion().value();
		}

		/**
		 * @brief Gets primary GMOD path item.
		 * @return Const reference to optional containing primary item.
		 * @note Zero-copy access. Guaranteed to contain value for valid LocalId.
		 */
		[[nodiscard]] const std::optional<GmodPath>& primaryItem() const noexcept
		{
			return m_builder.primaryItem();
		}

		/**
		 * @brief Gets optional secondary GMOD path item.
		 * @return Const reference to optional containing secondary item.
		 * @note Zero-copy access. May be empty if no secondary item specified.
		 */
		[[nodiscard]] const std::optional<GmodPath>& secondaryItem() const noexcept
		{
			return m_builder.secondaryItem();
		}

		/**
		 * @brief Gets all metadata tags as collection.
		 * @return Vector containing copies of all present metadata tags.
		 */
		[[nodiscard]] std::vector<MetadataTag> metadataTags() const noexcept
		{
			return m_builder.metadataTags();
		}

		/**
		 * @brief Calculates hash code for container use.
		 * @return Hash value suitable for std::unordered_set/map.
		 */
		[[nodiscard]] size_t hashCode() const noexcept
		{
			return m_builder.hashCode();
		}

		//----------------------------------------------
		// Metadata accessors
		//----------------------------------------------

		/**
		 * @brief Gets quantity metadata tag.
		 * @return Const reference to optional quantity metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& quantity() const noexcept
		{
			return m_builder.quantity();
		}

		/**
		 * @brief Gets content metadata tag.
		 * @return Const reference to optional content metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& content() const noexcept
		{
			return m_builder.content();
		}

		/**
		 * @brief Gets calculation metadata tag.
		 * @return Const reference to optional calculation metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& calculation() const noexcept
		{
			return m_builder.calculation();
		}

		/**
		 * @brief Gets state metadata tag.
		 * @return Const reference to optional state metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& state() const noexcept
		{
			return m_builder.state();
		}

		/**
		 * @brief Gets command metadata tag.
		 * @return Const reference to optional command metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& command() const noexcept
		{
			return m_builder.command();
		}

		/**
		 * @brief Gets type metadata tag.
		 * @return Const reference to optional type metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& type() const noexcept
		{
			return m_builder.type();
		}

		/**
		 * @brief Gets position metadata tag.
		 * @return Const reference to optional position metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& position() const noexcept
		{
			return m_builder.position();
		}

		/**
		 * @brief Gets detail metadata tag.
		 * @return Const reference to optional detail metadata.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& detail() const noexcept
		{
			return m_builder.detail();
		}

		//----------------------------------------------
		// State inspection
		//----------------------------------------------

		/**
		 * @brief Checks if Local ID is in verbose mode.
		 * @return true if verbose mode enabled.
		 */
		[[nodiscard]] bool isVerboseMode() const noexcept
		{
			return m_builder.isVerboseMode();
		}

		/**
		 * @brief Checks if Local ID contains custom metadata tags.
		 * @return true if any custom tags present.
		 */
		[[nodiscard]] bool hasCustomTag() const noexcept
		{
			return m_builder.hasCustomTag();
		}

		//----------------------------------------------
		// String conversion
		//----------------------------------------------

		/**
		 * @brief Converts LocalId to canonical string representation.
		 * @return VIS-compliant Local ID string.
		 * @throws std::invalid_argument If conversion fails.
		 */
		[[nodiscard]] std::string toString() const
		{
			return m_builder.toString();
		}

		//----------------------------------------------
		// Static parsing methods
		//----------------------------------------------

		/**
		 * @brief Parses Local ID string into LocalId object.
		 * @param[in] localIdStr VIS Local ID string to parse.
		 * @return Parsed LocalId object.
		 * @throws std::invalid_argument If parsing fails.
		 */
		[[nodiscard]] static LocalId parse( std::string_view localIdStr )
		{
			return LocalId( LocalIdBuilder::parse( localIdStr ) );
		}

		/**
		 * @brief Attempts to parse Local ID string with error reporting.
		 * @param[in] localIdStr String to parse.
		 * @param[out] errors Detailed parsing errors.
		 * @param[out] localId Parsed result on success.
		 * @return true if parsing succeeded.
		 */
		[[nodiscard]] static bool tryParse( std::string_view localIdStr, ParsingErrors& errors, std::optional<LocalId>& localId )
		{
			std::optional<LocalIdBuilder> builder;
			bool success = LocalIdBuilder::tryParse( localIdStr, errors, builder );
			if ( success && builder.has_value() )
			{
				localId = LocalId( std::move( builder.value() ) );
			}
			return success;
		}

		/**
		 * @brief Attempts to parse Local ID string.
		 * @param[in] localIdStr String to parse.
		 * @param[out] localId Parsed result on success.
		 * @return true if parsing succeeded.
		 */
		[[nodiscard]] static bool tryParse( std::string_view localIdStr, std::optional<LocalId>& localId )
		{
			std::optional<LocalIdBuilder> builder;
			bool success = LocalIdBuilder::tryParse( localIdStr, builder );
			if ( success && builder.has_value() )
			{
				localId = LocalId( std::move( builder.value() ) );
			}
			return success;
		}

	private:
		//----------------------------------------------
		// Private members variables
		//----------------------------------------------

		/**
		 * @brief Direct value storage of LocalIdBuilder state.
		 * @details Uses direct storage (not shared_ptr) for optimal performance
		 *          with zero indirection overhead.
		 */
		LocalIdBuilder m_builder;
	};
}
