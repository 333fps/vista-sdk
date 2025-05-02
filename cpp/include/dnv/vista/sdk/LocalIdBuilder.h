/**
 * @file LocalIdBuilder.h
 * @brief Concrete implementation of the LocalId builder
 *
 * This class provides the concrete implementation of the ILocalIdBuilder interface
 * for creating LocalId objects following the fluent builder pattern.
 */
#pragma once

#include "ILocalIdBuilder.h"
#include "LocalId.h"
#include "LocalIdItems.h"
#include "Codebooks.h"
#include "MetadataTag.h"
#include "GmodPath.h"
#include "ParsingErrors.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Forward declarations
	//-------------------------------------------------------------------------
	class LocalIdParsingErrorBuilder;

	/**
	 * @brief Concrete builder class for LocalId objects
	 *
	 * This class implements the ILocalIdBuilder interface for LocalId objects.
	 * It follows the fluent builder pattern and provides methods for setting
	 * all properties required to build a valid LocalId.
	 */
	class LocalIdBuilder final : public ILocalIdBuilder<LocalIdBuilder, LocalId>
	{
	public:
		//-------------------------------------------------------------------------
		// Constants and Type Definitions
		//-------------------------------------------------------------------------

		/** @brief Standard naming rule prefix for Local IDs */
		static const std::string namingRule;

		/** @brief List of codebook names used in LocalId */
		static const std::vector<CodebookName> usedCodebooks;

		//-------------------------------------------------------------------------
		// Constructors and Factory Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		LocalIdBuilder() = default;

		/**
		 * @brief Create a builder with the specified VIS version
		 * @param version The VIS version
		 * @return A new builder instance
		 */
		static LocalIdBuilder create( VisVersion version );

		/** @brief Deleted copy constructor. */
		LocalIdBuilder( const LocalIdBuilder& ) = delete;

		/** @brief Deleted copy assignment operator. */
		LocalIdBuilder& operator=( const LocalIdBuilder& ) = delete;

		/** @brief Allow move constructor. */
		LocalIdBuilder( LocalIdBuilder&& ) noexcept = default;

		/** @brief Allow move assignment. */
		LocalIdBuilder& operator=( LocalIdBuilder&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Core Build Method (ILocalIdBuilder Implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Creates the final LocalId object
		 * @return A new LocalId instance
		 * @throws std::invalid_argument If the builder state is invalid
		 */
		[[nodiscard]] LocalId build() const override;

		//-------------------------------------------------------------------------
		// State Inspection Methods (ILocalIdBuilder Implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Check if the builder state is valid for creating a LocalId
		 * @return True if valid, false otherwise
		 */
		[[nodiscard]] bool isValid() const override;

		/**
		 * @brief Check if the builder is empty
		 * @return True if empty, false otherwise
		 */
		[[nodiscard]] bool isEmpty() const override;

		/**
		 * @brief Check if the builder has no metadata
		 * @return True if no metadata, false otherwise
		 */
		[[nodiscard]] bool isEmptyMetadata() const;

		/**
		 * @brief Check if the builder has a custom tag
		 * @return True if has custom tag, false otherwise
		 */
		[[nodiscard]] bool hasCustomTag() const override;

		/**
		 * @brief Get the VIS version
		 * @return The VIS version, if set
		 */
		[[nodiscard]] std::optional<VisVersion> visVersion() const override;

		/**
		 * @brief Check if verbose mode is enabled
		 * @return True if verbose mode enabled
		 */
		[[nodiscard]] bool isVerboseMode() const override;

		/**
		 * @brief Get the items (primary and secondary)
		 * @return Reference to the LocalIdItems
		 */
		[[nodiscard]] const LocalIdItems& items() const;

		/**
		 * @brief Get the primary item
		 * @return The primary item path
		 */
		[[nodiscard]] const GmodPath& primaryItem() const override;

		/**
		 * @brief Get the secondary item
		 * @return The secondary item path, if set
		 */
		[[nodiscard]] const std::optional<GmodPath>& secondaryItem() const override;

		/**
		 * @brief Get all metadata tags
		 * @return Vector of metadata tags
		 */
		[[nodiscard]] const std::vector<MetadataTag>& metadataTags() const override;

		/**
		 * @brief Get quantity metadata tag
		 * @return The quantity tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& quantity() const override;

		/**
		 * @brief Get content metadata tag
		 * @return The content tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& content() const override;

		/**
		 * @brief Get calculation metadata tag
		 * @return The calculation tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& calculation() const override;

		/**
		 * @brief Get state metadata tag
		 * @return The state tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& state() const override;

		/**
		 * @brief Get command metadata tag
		 * @return The command tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& command() const override;

		/**
		 * @brief Get type metadata tag
		 * @return The type tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& type() const override;

		/**
		 * @brief Get position metadata tag
		 * @return The position tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& position() const override;

		/**
		 * @brief Get detail metadata tag
		 * @return The detail tag, if present
		 */
		[[nodiscard]] const std::optional<MetadataTag>& detail() const override;

		/**
		 * @brief Get string representation of the builder state
		 * @return String representation
		 */
		[[nodiscard]] std::string toString() const override;

		/**
		 * @brief Write string representation to a stringstream
		 * @param builder The stringstream to write to
		 */
		void toString( std::stringstream& builder ) const;

		//-------------------------------------------------------------------------
		// VIS Version Builder Methods (ILocalIdBuilder Implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Set the VIS version from a string
		 * @param visVersion The VIS version string
		 * @return New builder instance with the updated VIS version
		 */
		[[nodiscard]] LocalIdBuilder withVisVersion( const std::string& visVersion ) override;

		/**
		 * @brief Set the VIS version
		 * @param version The VIS version
		 * @return New builder instance with the updated VIS version
		 */
		[[nodiscard]] LocalIdBuilder withVisVersion( VisVersion version ) override;

		/**
		 * @brief Try to set the VIS version
		 * @param version The optional VIS version
		 * @return New builder instance with the updated VIS version if provided
		 */
		[[nodiscard]] LocalIdBuilder tryWithVisVersion( const std::optional<VisVersion>& version ) override;

		/**
		 * @brief Try to set the VIS version from a string
		 * @param visVersionStr The optional VIS version string
		 * @param succeeded Output parameter indicating success
		 * @return New builder instance with the updated VIS version if successful
		 */
		[[nodiscard]] LocalIdBuilder tryWithVisVersion(
			const std::optional<std::string>& visVersionStr, bool& succeeded ) override;

		[[nodiscard]] LocalIdBuilder tryWithVisVersion( const std::optional<VisVersion>& version, bool& succeeded );

		/**
		 * @brief Remove the VIS version
		 * @return New builder instance without a VIS version
		 */
		[[nodiscard]] LocalIdBuilder withoutVisVersion() override;

		/**
		 * @brief Set the verbose mode
		 * @param verboseMode True to enable verbose mode
		 * @return New builder instance with the updated verbose mode
		 */
		[[nodiscard]] LocalIdBuilder withVerboseMode( bool verboseMode ) override;

		//-------------------------------------------------------------------------
		// Primary Item Builder Methods (ILocalIdBuilder Implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Sets the primary item by moving the provided GmodPath.
		 * @param item The GmodPath to set as primary (moved).
		 * @return A new LocalIdBuilder instance with the updated primary item.
		 * @throws std::invalid_argument if setting fails.
		 */
		[[nodiscard]] LocalIdBuilder withPrimaryItem( GmodPath&& item ) override;

		/**
		 * @brief Tries to set the primary item by moving the provided GmodPath.
		 * @param item The GmodPath to set as primary (moved).
		 * @return A new LocalIdBuilder instance with the updated primary item.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( GmodPath&& item ) override;

		/**
		 * @brief Tries to set the primary item by moving the provided GmodPath.
		 * @param item The GmodPath to set as primary (moved).
		 * @param[out] succeeded True if setting was successful, false otherwise.
		 * @return A new LocalIdBuilder instance with the updated primary item, or the original if failed.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( GmodPath&& item, bool& succeeded ) override;

		/**
		 * @brief Tries to set the primary item by moving the provided optional GmodPath.
		 * If the optional does not contain a value, the builder is returned unchanged.
		 * @param item An optional containing the GmodPath to set as primary (moved if present).
		 * @return A new LocalIdBuilder instance with the updated primary item if the optional had a value, otherwise the original builder.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( std::optional<GmodPath>&& item );

		/**
		 * @brief Tries to set the primary item by moving the provided optional GmodPath.
		 * If the optional does not contain a value, succeeded is set to false and the builder is returned unchanged.
		 * @param item An optional containing the GmodPath to set as primary (moved if present).
		 * @param[out] succeeded True if setting was successful (optional had value), false otherwise.
		 * @return A new LocalIdBuilder instance with the updated primary item if successful, otherwise the original builder.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( std::optional<GmodPath>&& item, bool& succeeded );

		/**
		 * @brief Removes the primary item, resetting it to an empty GmodPath.
		 * @return A new LocalIdBuilder instance without the primary item.
		 */
		[[nodiscard]] LocalIdBuilder withoutPrimaryItem() override;

		//-------------------------------------------------------------------------
		// Secondary Item Builder Methods (ILocalIdBuilder Implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Sets the secondary item by moving the provided GmodPath.
		 * @param item The GmodPath to set as secondary (moved).
		 * @return A new LocalIdBuilder instance with the updated secondary item.
		 * @throws std::invalid_argument if setting fails.
		 */
		[[nodiscard]] LocalIdBuilder withSecondaryItem( GmodPath&& item ) override;

		/**
		 * @brief Tries to set the secondary item by moving the provided optional GmodPath.
		 * If the optional does not contain a value, the builder is returned unchanged.
		 * @param item An optional containing the GmodPath to set as secondary (moved if present).
		 * @return A new LocalIdBuilder instance with the updated secondary item if the optional had a value, otherwise the original builder.
		 */
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( GmodPath&& item ) override;

		/**
		 * @brief Tries to set the secondary item by moving the provided optional GmodPath.
		 * If the optional does not contain a value, succeeded is set to false and the builder is returned unchanged.
		 * @param item An optional containing the GmodPath to set as secondary (moved if present).
		 * @param[out] succeeded True if setting was successful (optional had value), false otherwise.
		 * @return A new LocalIdBuilder instance with the updated secondary item if successful, otherwise the original builder.
		 */
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item, bool& succeeded ) override;
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item ) override;

		/**
		 * @brief Tries to set the secondary item by moving the provided GmodPath.
		 * Internal helper method.
		 * @param item The GmodPath to set as secondary (moved).
		 * @param[out] succeeded True if setting was successful, false otherwise.
		 * @return A new LocalIdBuilder instance with the updated secondary item, or the original if failed.
		 */
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( GmodPath&& item, bool& succeeded ) override;

		/**
		 * @brief Removes the secondary item.
		 * @return A new LocalIdBuilder instance without the secondary item.
		 */
		[[nodiscard]] LocalIdBuilder withoutSecondaryItem() override;

		//-------------------------------------------------------------------------
		// Metadata Tag Builder Methods (ILocalIdBuilder Implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Add a metadata tag
		 * @param metadataTag The metadata tag to add
		 * @return New builder instance with the added metadata tag
		 */
		[[nodiscard]] LocalIdBuilder withMetadataTag( const MetadataTag& metadataTag ) override;

		/**
		 * @brief Try to add a metadata tag
		 * @param metadataTag The optional metadata tag
		 * @return New builder instance with the added metadata tag if provided
		 */
		[[nodiscard]] LocalIdBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag ) override;

		/**
		 * @brief Try to add a metadata tag
		 * @param metadataTag The optional metadata tag
		 * @param succeeded Output parameter indicating success
		 * @return New builder instance with the added metadata tag if successful
		 */
		[[nodiscard]] LocalIdBuilder tryWithMetadataTag(
			const std::optional<MetadataTag>& metadataTag, bool& succeeded ) override;

		/**
		 * @brief Remove a metadata tag by codebook name
		 * @param name The codebook name of the tag to remove
		 * @return New builder instance without the specified metadata tag
		 */
		[[nodiscard]] LocalIdBuilder withoutMetadataTag( CodebookName name ) override;

		//-------------------------------------------------------------------------
		// Specific Metadata Tag Builder Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Set the quantity metadata tag
		 * @param quantity The quantity tag
		 * @return New builder instance with the updated quantity tag
		 */
		[[nodiscard]] LocalIdBuilder withQuantity( const MetadataTag& quantity );

		/**
		 * @brief Set the content metadata tag
		 * @param content The content tag
		 * @return New builder instance with the updated content tag
		 */
		[[nodiscard]] LocalIdBuilder withContent( const MetadataTag& content );

		/**
		 * @brief Set the calculation metadata tag
		 * @param calculation The calculation tag
		 * @return New builder instance with the updated calculation tag
		 */
		[[nodiscard]] LocalIdBuilder withCalculation( const MetadataTag& calculation );

		/**
		 * @brief Set the state metadata tag
		 * @param state The state tag
		 * @return New builder instance with the updated state tag
		 */
		[[nodiscard]] LocalIdBuilder withState( const MetadataTag& state );

		/**
		 * @brief Set the command metadata tag
		 * @param command The command tag
		 * @return New builder instance with the updated command tag
		 */
		[[nodiscard]] LocalIdBuilder withCommand( const MetadataTag& command );

		/**
		 * @brief Set the type metadata tag
		 * @param type The type tag
		 * @return New builder instance with the updated type tag
		 */
		[[nodiscard]] LocalIdBuilder withType( const MetadataTag& type );

		/**
		 * @brief Set the position metadata tag
		 * @param position The position tag
		 * @return New builder instance with the updated position tag
		 */
		[[nodiscard]] LocalIdBuilder withPosition( const MetadataTag& position );

		/**
		 * @brief Set the detail metadata tag
		 * @param detail The detail tag
		 * @return New builder instance with the updated detail tag
		 */
		[[nodiscard]] LocalIdBuilder withDetail( const MetadataTag& detail );

		/**
		 * @brief Remove the quantity metadata tag
		 * @return New builder instance without a quantity tag
		 */
		[[nodiscard]] LocalIdBuilder withoutQuantity();

		/**
		 * @brief Remove the content metadata tag
		 * @return New builder instance without a content tag
		 */
		[[nodiscard]] LocalIdBuilder withoutContent();

		/**
		 * @brief Remove the calculation metadata tag
		 * @return New builder instance without a calculation tag
		 */
		[[nodiscard]] LocalIdBuilder withoutCalculation();

		/**
		 * @brief Remove the state metadata tag
		 * @return New builder instance without a state tag
		 */
		[[nodiscard]] LocalIdBuilder withoutState();

		/**
		 * @brief Remove the command metadata tag
		 * @return New builder instance without a command tag
		 */
		[[nodiscard]] LocalIdBuilder withoutCommand();

		/**
		 * @brief Remove the type metadata tag
		 * @return New builder instance without a type tag
		 */
		[[nodiscard]] LocalIdBuilder withoutType();

		/**
		 * @brief Remove the position metadata tag
		 * @return New builder instance without a position tag
		 */
		[[nodiscard]] LocalIdBuilder withoutPosition();

		/**
		 * @brief Remove the detail metadata tag
		 * @return New builder instance without a detail tag
		 */
		[[nodiscard]] LocalIdBuilder withoutDetail();

		//-------------------------------------------------------------------------
		// Static Parse Methods (Required by ILocalIdBuilder)
		//-------------------------------------------------------------------------

		/**
		 * @brief Parse a string into a LocalIdBuilder
		 * @param localIdStr The string to parse
		 * @return A new LocalIdBuilder instance
		 * @throws std::invalid_argument If parsing fails
		 */
		[[nodiscard]] static LocalIdBuilder parse( const std::string& localIdStr );

		/**
		 * @brief Try to parse a string into a LocalIdBuilder
		 * @param localIdStr The string to parse
		 * @param localId Output parameter for the parsed builder
		 * @return True if parsing succeeded, false otherwise
		 */
		[[nodiscard]] static bool tryParse(
			const std::string& localIdStr,
			std::optional<LocalIdBuilder>& localId );

		/**
		 * @brief Try to parse a string into a LocalIdBuilder with error information
		 * @param localIdStr The string to parse
		 * @param errors Output parameter for parsing errors
		 * @param localId Output parameter for the parsed builder
		 * @return True if parsing succeeded, false otherwise
		 */
		[[nodiscard]] static bool tryParse(
			const std::string& localIdStr,
			ParsingErrors& errors,
			std::optional<LocalIdBuilder>& localId );

		/**
		 * @brief Internal parsing implementation
		 * @param localIdStr The string to parse
		 * @param errorBuilder Error builder for collecting parsing errors
		 * @param localIdBuilder Output parameter for the parsed builder
		 * @return True if parsing succeeded, false otherwise
		 */
		[[nodiscard]] static bool tryParseInternal(
			const std::string& localIdStr,
			LocalIdParsingErrorBuilder& errorBuilder,
			std::optional<LocalIdBuilder>& localIdBuilder );

		//-------------------------------------------------------------------------
		// Comparison Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality comparison operator
		 * @param other The builder to compare with
		 * @return True if equal, false otherwise
		 */
		bool operator==( const LocalIdBuilder& other ) const;

		/**
		 * @brief Inequality comparison operator
		 * @param other The builder to compare with
		 * @return True if not equal, false otherwise
		 */
		bool operator!=( const LocalIdBuilder& other ) const;

		/**
		 * @brief Calculate hash code for use in hash-based containers
		 * @return Hash code
		 */
		size_t hashCode() const;

	private:
		//-------------------------------------------------------------------------
		// Private Static Helper Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Add an error to the error builder
		 * @param errorBuilder The error builder to add to
		 * @param state The parsing state where the error occurred
		 * @param message Optional custom error message
		 */
		static void addError(
			LocalIdParsingErrorBuilder& errorBuilder,
			LocalIdParsingState state,
			const std::string& message = "" );

		/**
		 * @brief Advance the parser position
		 * @param i The index to advance
		 * @param segment The segment being parsed
		 * @param state The current parsing state
		 */
		static void advanceParser(
			size_t& i,
			const std::string& segment,
			LocalIdParsingState& state );

		/**
		 * @brief Advance the parser position
		 * @param i The index to advance
		 * @param segment The segment being parsed
		 */
		static void advanceParser(
			size_t& i,
			const std::string& segment );

		/**
		 * @brief Advance the parser to a new state
		 * @param state The current state to update
		 * @param to The new state
		 */
		static void advanceParser(
			LocalIdParsingState& state,
			LocalIdParsingState to );

		/**
		 * @brief Advance the parser position and state
		 * @param i The index to advance
		 * @param segment The segment being parsed
		 * @param state The current parsing state
		 * @param to The new state
		 */
		static void advanceParser(
			size_t& i,
			const std::string& segment,
			LocalIdParsingState& state,
			LocalIdParsingState to );

		/**
		 * @brief Advance the parser position
		 * @param i The index to advance
		 * @param segment The segment being parsed
		 * @param state The current parsing state
		 */
		static void advanceParser(
			size_t& i,
			const std::string_view& segment,
			LocalIdParsingState& state );

		/**
		 * @brief Advance the parser position
		 * @param i The index to advance
		 * @param segment The segment being parsed
		 */
		static void advanceParser(
			size_t& i,
			const std::string_view& segment );

		/**
		 * @brief Advance the parser position and state
		 * @param i The index to advance
		 * @param segment The segment being parsed
		 * @param state The current parsing state
		 * @param to The new state
		 */
		static void advanceParser(
			size_t& i,
			const std::string_view& segment,
			LocalIdParsingState& state,
			LocalIdParsingState to );

		/**
		 * @brief Find the indexes of the next state in a string
		 * @param str The string to search
		 * @param state The current parsing state
		 * @return Pair of start and end indexes
		 */
		static std::pair<size_t, size_t> nextStateIndexes(
			const std::string& str,
			LocalIdParsingState state );

		/**
		 * @brief Convert a metadata prefix to a parsing state
		 * @param prefix The prefix to convert
		 * @return The corresponding parsing state, if recognized
		 */
		static std::optional<LocalIdParsingState> metaPrefixToState(
			const std::string_view& prefix );

		/**
		 * @brief Get the next parsing state in the sequence
		 * @param prev The previous parsing state
		 * @return The next parsing state, if there is one
		 */
		static std::optional<LocalIdParsingState> nextParsingState(
			LocalIdParsingState prev );

		/**
		 * @brief Parse a metadata tag
		 * @param codebookName The codebook name for the tag
		 * @param state The current parsing state
		 * @param i The current position in the input
		 * @param segment The segment being parsed
		 * @param tag Output parameter for the parsed tag
		 * @param codebooks Codebooks for validation
		 * @param errorBuilder Error builder for collecting errors
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool parseMetaTag(
			CodebookName codebookName,
			LocalIdParsingState& state,
			size_t& i,
			const std::string_view& segment,
			std::optional<MetadataTag>& tag,
			const std::shared_ptr<Codebooks>& codebooks,
			LocalIdParsingErrorBuilder& errorBuilder );

		//-------------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------------

		/** @brief The VIS version */
		std::optional<VisVersion> m_visVersion;

		/** @brief Flag indicating verbose mode */
		bool m_verboseMode = false;

		/** @brief Contains primary and secondary items */
		LocalIdItems m_items;

		/** @brief Quantity metadata tag */
		std::optional<MetadataTag> m_quantity;

		/** @brief Content metadata tag */
		std::optional<MetadataTag> m_content;

		/** @brief Calculation metadata tag */
		std::optional<MetadataTag> m_calculation;

		/** @brief State metadata tag */
		std::optional<MetadataTag> m_state;

		/** @brief Command metadata tag */
		std::optional<MetadataTag> m_command;

		/** @brief Type metadata tag */
		std::optional<MetadataTag> m_type;

		/** @brief Position metadata tag */
		std::optional<MetadataTag> m_position;

		/** @brief Detail metadata tag */
		std::optional<MetadataTag> m_detail;
	};
}
