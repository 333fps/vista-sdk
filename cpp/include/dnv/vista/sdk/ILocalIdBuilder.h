/**
 * @file ILocalIdBuilder.h
 * @brief Interface template for Local ID builders.
 *
 * Defines the interface for builder classes that create Local IDs,
 * following the fluent builder pattern.
 */

#pragma once

#include "ParsingErrors.h"
#include "VisVersion.h"
#include "GmodPath.h"
#include "MetadataTag.h"
#include "CodebookName.h"

namespace dnv::vista::sdk
{
	//---------------------------------------------------------------------
	// Forward declarations
	//---------------------------------------------------------------------
	class LocalIdBuilder;

	/**
	 * @brief Interface for building Local IDs.
	 *
	 * This template interface provides the builder pattern for creating
	 * Local ID objects with a fluent interface. Template parameters allow
	 * for proper return type of builder methods in derived classes.
	 *
	 * @tparam TBuilder The concrete builder type (for method chaining).
	 * @tparam TResult The result type that the builder produces.
	 */
	template <typename TBuilder, typename TResult>
	class ILocalIdBuilder
	{
	public:
		//---------------------------------------------------------------------
		// Lifecycle
		//---------------------------------------------------------------------

		/** @brief Default constructor. */
		ILocalIdBuilder() = default;

		/** @brief Virtual destructor. */
		virtual ~ILocalIdBuilder() = default;

		/** @brief Delete copy constructor - interfaces shouldn't be copied. */
		ILocalIdBuilder( const ILocalIdBuilder& ) = default;

		/** @brief Delete copy assignment - interfaces shouldn't be assigned. */
		ILocalIdBuilder& operator=( const ILocalIdBuilder& ) = default;

		/** @brief Allow move constructor. */
		ILocalIdBuilder( ILocalIdBuilder&& ) noexcept = default;

		/** @brief Allow move assignment. */
		ILocalIdBuilder& operator=( ILocalIdBuilder&& ) noexcept = default;

		//---------------------------------------------------------------------
		// Core Build Method
		//---------------------------------------------------------------------

		/**
		 * @brief Creates the final Local ID object.
		 * @return A new instance of the Local ID.
		 */
		[[nodiscard]] virtual TResult build() const = 0;

		//---------------------------------------------------------------------
		// State Inspection Methods
		//---------------------------------------------------------------------

		/**
		 * @brief Gets the VIS version.
		 * @return The VIS version, if specified.
		 */
		[[nodiscard]] virtual std::optional<VisVersion> visVersion() const = 0;

		/**
		 * @brief Checks if verbose mode is enabled.
		 * @return True if verbose mode is enabled.
		 */
		[[nodiscard]] virtual bool isVerboseMode() const = 0;

		/**
		 * @brief Gets the primary item path.
		 * @return The primary item path, if specified.
		 */
		[[nodiscard]] virtual const GmodPath& primaryItem() const = 0;

		/**
		 * @brief Gets the secondary item path.
		 * @return The secondary item path, if specified.
		 */
		[[nodiscard]] virtual const std::optional<GmodPath>& secondaryItem() const = 0;

		/**
		 * @brief Checks if the builder has a custom tag.
		 * @return True if a custom tag exists.
		 */
		[[nodiscard]] virtual bool hasCustomTag() const = 0;

		/**
		 * @brief Gets all metadata tags.
		 * @return A const reference to the vector of metadata tags.
		 */
		[[nodiscard]] virtual const std::vector<MetadataTag>& metadataTags() const = 0;

		/**
		 * @brief Gets quantity metadata tag if present.
		 * @return The quantity tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& quantity() const = 0;

		/**
		 * @brief Gets content metadata tag if present.
		 * @return The content tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& content() const = 0;

		/**
		 * @brief Gets calculation metadata tag if present.
		 * @return The calculation tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& calculation() const = 0;

		/**
		 * @brief Gets state metadata tag if present.
		 * @return The state tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& state() const = 0;

		/**
		 * @brief Gets command metadata tag if present.
		 * @return The command tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& command() const = 0;

		/**
		 * @brief Gets type metadata tag if present.
		 * @return The type tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& type() const = 0;

		/**
		 * @brief Gets position metadata tag if present.
		 * @return The position tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& position() const = 0;

		/**
		 * @brief Gets detail metadata tag if present.
		 * @return The detail tag, if present.
		 */
		[[nodiscard]] virtual const std::optional<MetadataTag>& detail() const = 0;

		/**
		 * @brief Checks if the builder contains valid data for building a Local ID.
		 * @return True if valid, false otherwise.
		 */
		[[nodiscard]] virtual bool isValid() const = 0;

		/**
		 * @brief Checks if the builder has no data.
		 * @return True if empty, false otherwise.
		 */
		[[nodiscard]] virtual bool isEmpty() const = 0;

		/**
		 * @brief Gets the string representation of the builder state.
		 * @return String representation.
		 */
		[[nodiscard]] virtual std::string toString() const = 0;

		//---------------------------------------------------------------------
		// VIS Version Builder Methods
		//---------------------------------------------------------------------

		/**
		 * @brief Sets the VIS version from a string.
		 * @param visVersion The VIS version string.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withVisVersion( const std::string& visVersion ) = 0;

		/**
		 * @brief Sets the VIS version.
		 * @param version The VIS version.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withVisVersion( VisVersion version ) = 0;

		/**
		 * @brief Tries to set the VIS version.
		 * @param version The optional VIS version.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithVisVersion( const std::optional<VisVersion>& version ) = 0;

		/**
		 * @brief Tries to set the VIS version from a string.
		 * @param visVersionStr The optional VIS version string.
		 * @param succeeded Output parameter indicating success.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithVisVersion(
			const std::optional<std::string>& visVersionStr,
			bool& succeeded ) = 0;

		/**
		 * @brief Removes the VIS version.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withoutVisVersion() = 0;

		//---------------------------------------------------------------------
		// Verbose Mode Builder Methods
		//---------------------------------------------------------------------

		/**
		 * @brief Sets the verbose mode.
		 * @param verboseMode True to enable verbose mode.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withVerboseMode( bool verboseMode ) = 0;

		//---------------------------------------------------------------------
		// Primary Item Builder Methods
		//---------------------------------------------------------------------

		/**
		 * @brief Sets the primary item.
		 * @param item The primary item path.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withPrimaryItem( GmodPath&& item ) = 0;

		/**
		 * @brief Tries to set the primary item.
		 * @param item The optional primary item path.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithPrimaryItem( GmodPath&& item ) = 0;

		/**
		 * @brief Tries to set the primary item.
		 * @param item The optional primary item path.
		 * @param succeeded Output parameter indicating success.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithPrimaryItem( GmodPath&& item, bool& succeeded ) = 0;

		/**
		 * @brief Removes the primary item.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withoutPrimaryItem() = 0;

		//---------------------------------------------------------------------
		// Secondary Item Builder Methods
		//---------------------------------------------------------------------

		/**
		 * @brief Sets the secondary item.
		 * @param item The secondary item path.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withSecondaryItem( GmodPath&& item ) = 0;

		/**
		 * @brief Tries to set the secondary item.
		 * @param item The optional secondary item path.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( GmodPath&& item ) = 0;

		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item, bool& succeeded ) = 0;
		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item ) = 0;

		/**
		 * @brief Tries to set the secondary item.
		 * @param item The optional secondary item path.
		 * @param succeeded Output parameter indicating success.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( GmodPath&& item, bool& succeeded ) = 0;

		/**
		 * @brief Removes the secondary item.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withoutSecondaryItem() = 0;

		//---------------------------------------------------------------------
		// Metadata Tag Builder Methods
		//---------------------------------------------------------------------

		/**
		 * @brief Adds a metadata tag.
		 * @param metadataTag The metadata tag to add.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withMetadataTag( const MetadataTag& metadataTag ) = 0;

		/**
		 * @brief Tries to add a metadata tag.
		 * @param metadataTag The optional metadata tag.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag ) = 0;

		/**
		 * @brief Tries to add a metadata tag.
		 * @param metadataTag The optional metadata tag.
		 * @param succeeded Output parameter indicating success.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag, bool& succeeded ) = 0;

		/**
		 * @brief Removes a metadata tag by codebook name.
		 * @param name The codebook name of the tag to remove.
		 * @return Builder instance for method chaining.
		 */
		[[nodiscard]] virtual TBuilder withoutMetadataTag( CodebookName name ) = 0;

		//---------------------------------------------------------------------
		// Static Factory Methods
		//---------------------------------------------------------------------

		/**
		 * @brief Parses a string into a builder.
		 * @param localIdStr The string to parse.
		 * @return A new builder instance.
		 * @throws std::invalid_argument If parsing fails.
		 */
		static TBuilder parse( const std::string& localIdStr );

		/**
		 * @brief Tries to parse a string into a builder.
		 * @param localIdStr The string to parse.
		 * @param localId Output parameter for the parsed builder.
		 * @return True if parsing succeeded, false otherwise.
		 */
		static bool tryParse( const std::string& localIdStr, std::optional<TBuilder>& localId );

		/**
		 * @brief Tries to parse a string into a builder with error information.
		 * @param localIdStr The string to parse.
		 * @param errors Output parameter for parsing errors.
		 * @param localId Output parameter for the parsed builder.
		 * @return True if parsing succeeded, false otherwise.
		 */
		static bool tryParse(
			const std::string& localIdStr,
			ParsingErrors& errors,
			std::optional<TBuilder>& localId );
	};
}
