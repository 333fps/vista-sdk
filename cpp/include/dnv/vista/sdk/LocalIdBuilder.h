#pragma once

#include "ILocalIdBuilder.h"
#include "LocalIdItems.h"
#include "MetadataTag.h"

namespace dnv::vista::sdk
{
	enum class VisVersion;
	enum class CodebookName;
	enum class LocalIdParsingState;
	class Codebooks;
	class GmodPath;
	class ParsingErrors;
	class LocalId;
	class LocalIdParsingErrorBuilder;

	/**
	 * @brief Builder class for LocalId objects, implementing the ILocalIdBuilder interface.
	 */
	class LocalIdBuilder final : public ILocalIdBuilder<LocalIdBuilder, LocalId>
	{
	public:
		//-------------------------------------------------------------------------
		// Constants
		//-------------------------------------------------------------------------

		/** @brief Naming rule constant for local IDs.*/
		static const std::string namingRule;

		/** @brief Used codebooks array */
		static const std::vector<CodebookName> usedCodebooks;

		//-------------------------------------------------------------------------
		// Constructors and Static Factories
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		LocalIdBuilder();

		/**
		 * @brief Copy constructor
		 */
		LocalIdBuilder( const LocalIdBuilder& other );

		/**
		 * @brief Copy assignment operator
		 * @param other The object to copy from
		 * @return Reference to this object
		 */
		LocalIdBuilder& operator=( const LocalIdBuilder& other );

		/**
		 * @brief Create a new LocalIdBuilder with the specified VIS version
		 * @param version The VIS version to use
		 * @return A new LocalIdBuilder instance
		 */
		static LocalIdBuilder create( VisVersion version );

		/**
		 * @brief Build a LocalId from this builder
		 * @return The built LocalId
		 * @throws std::invalid_argument If the builder is in an invalid state
		 */
		virtual LocalId build() const override;

		//-------------------------------------------------------------------------
		// State Inspection Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Check if valid
		 * @return true if valid
		 */
		virtual bool isValid() const override;

		/**
		 * @brief Check if empty
		 * @return true if empty
		 */
		virtual bool isEmpty() const override;

		/**
		 * @brief Check if metadata is empty
		 * @return true if metadata is empty
		 */
		bool isEmptyMetadata() const;

		/**
		 * @brief Check if has custom tag
		 * @return true if has custom tag
		 */
		virtual bool hasCustomTag() const override;

		//-------------------------------------------------------------------------
		// Core Property Getters
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the VIS version
		 * @return Optional VIS version
		 */
		virtual std::optional<VisVersion> visVersion() const override;

		/**
		 * @brief Get verbose mode flag
		 * @return true if verbose mode is enabled
		 */
		virtual bool isVerboseMode() const override;

		//-------------------------------------------------------------------------
		// Item Getters
		//-------------------------------------------------------------------------

		/**
		 * @brief Get items
		 * @return LocalIdItems
		 */
		const LocalIdItems& items() const;

		/**
		 * @brief Get primary item
		 * @return Optional primary item GmodPath
		 */
		virtual const std::optional<GmodPath>& primaryItem() const override;

		/**
		 * @brief Get secondary item
		 * @return Optional secondary item GmodPath
		 */
		virtual const std::optional<GmodPath>& secondaryItem() const override;

		//-------------------------------------------------------------------------
		// Metadata Tag Getters
		//-------------------------------------------------------------------------

		/**
		 * @brief Get metadata tags
		 * @return List of metadata tags
		 */
		virtual const std::vector<MetadataTag> metadataTags() const override;

		/**
		 * @brief Get quantity tag
		 * @return Optional quantity metadata tag
		 */
		const std::optional<MetadataTag>& quantity() const;

		/**
		 * @brief Get content tag
		 * @return Optional content metadata tag
		 */
		const std::optional<MetadataTag>& content() const;

		/**
		 * @brief Get calculation tag
		 * @return Optional calculation metadata tag
		 */
		const std::optional<MetadataTag>& calculation() const;

		/**
		 * @brief Get state tag
		 * @return Optional state metadata tag
		 */
		const std::optional<MetadataTag>& state() const;

		/**
		 * @brief Get command tag
		 * @return Optional command metadata tag
		 */
		const std::optional<MetadataTag>& command() const;

		/**
		 * @brief Get type tag
		 * @return Optional type metadata tag
		 */
		const std::optional<MetadataTag>& type() const;

		/**
		 * @brief Get position tag
		 * @return Optional position metadata tag
		 */
		const std::optional<MetadataTag>& position() const;

		/**
		 * @brief Get detail tag
		 * @return Optional detail metadata tag
		 */
		const std::optional<MetadataTag>& detail() const;

		//-------------------------------------------------------------------------
		// Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert to string
		 * @return String representation
		 */
		virtual std::string toString() const override;

		/**
		 * @brief Convert to string
		 * @param builder Stream to write to
		 */
		void toString( std::stringstream& builder ) const;

		//-------------------------------------------------------------------------
		// Builder Methods - Core Properties
		//-------------------------------------------------------------------------

		/**
		 * @brief Set VIS version from string
		 * @param visVersion VIS version string
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withVisVersion( const std::string& visVersion ) override;

		/**
		 * @brief Set VIS version from enum
		 * @param version VIS version enum
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withVisVersion( VisVersion version ) override;

		/**
		 * @brief Try to set VIS version from optional
		 * @param version Optional VIS version
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithVisVersion( const std::optional<VisVersion>& version ) override;

		/**
		 * @brief Try to set VIS version from string
		 * @param visVersionStr Optional VIS version string
		 * @param[out] succeeded true if operation succeeded
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithVisVersion( const std::optional<std::string>& visVersionStr, bool& succeeded ) override;

		/**
		 * @brief Remove VIS version
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withoutVisVersion() override;

		/**
		 * @brief Set verbose mode
		 * @param verboseMode Verbose mode flag
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withVerboseMode( bool verboseMode ) override;

		//-------------------------------------------------------------------------
		// Builder Methods - Items
		//-------------------------------------------------------------------------

		/**
		 * @brief Set primary item
		 * @param item GmodPath item
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withPrimaryItem( const GmodPath& item ) override;

		/**
		 * @brief Try to set primary item from optional
		 * @param item Optional GmodPath
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithPrimaryItem( const std::optional<GmodPath>& item ) override;

		/**
		 * @brief Try to set primary item from optional
		 * @param item Optional GmodPath
		 * @param[out] succeeded true if operation succeeded
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithPrimaryItem( const std::optional<GmodPath>& item, bool& succeeded ) override;

		/**
		 * @brief Remove primary item
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withoutPrimaryItem() override;

		/**
		 * @brief Set secondary item
		 * @param item GmodPath item
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withSecondaryItem( const GmodPath& item ) override;

		/**
		 * @brief Try to set secondary item from optional
		 * @param item Optional GmodPath
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithSecondaryItem( const std::optional<GmodPath>& item ) override;

		/**
		 * @brief Try to set secondary item from optional
		 * @param item Optional GmodPath
		 * @param[out] succeeded true if operation succeeded
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithSecondaryItem( const std::optional<GmodPath>& item, bool& succeeded ) override;

		/**
		 * @brief Remove secondary item
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withoutSecondaryItem() override;

		//-------------------------------------------------------------------------
		// Builder Methods - Metadata Tags
		//-------------------------------------------------------------------------

		/**
		 * @brief Add metadata tag
		 * @param metadataTag MetadataTag to add
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withMetadataTag( const MetadataTag& metadataTag ) override;

		/**
		 * @brief Try to add metadata tag from optional
		 * @param metadataTag Optional MetadataTag
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag ) override;

		/**
		 * @brief Try to add metadata tag from optional
		 * @param metadataTag Optional MetadataTag
		 * @param[out] succeeded true if operation succeeded
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag, bool& succeeded ) override;

		/**
		 * @brief Remove metadata tag by name
		 * @param name CodebookName of tag to remove
		 * @return Builder for method chaining
		 */
		virtual LocalIdBuilder withoutMetadataTag( CodebookName name ) override;

		LocalIdBuilder withQuantity( const MetadataTag& quantity );
		LocalIdBuilder withContent( const MetadataTag& content );
		LocalIdBuilder withCalculation( const MetadataTag& calculation );
		LocalIdBuilder withState( const MetadataTag& state );
		LocalIdBuilder withCommand( const MetadataTag& command );
		LocalIdBuilder withType( const MetadataTag& type );
		LocalIdBuilder withPosition( const MetadataTag& position );
		LocalIdBuilder withDetail( const MetadataTag& detail );

		LocalIdBuilder withoutQuantity();
		LocalIdBuilder withoutContent();
		LocalIdBuilder withoutCalculation();
		LocalIdBuilder withoutState();
		LocalIdBuilder withoutCommand();
		LocalIdBuilder withoutType();
		LocalIdBuilder withoutPosition();
		LocalIdBuilder withoutDetail();

		//-------------------------------------------------------------------------
		// Static Parsing Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Static method to parse a LocalId string
		 * @param localIdStr String to parse
		 * @return LocalIdBuilder representing the parsed string
		 * @throws std::invalid_argument If parsing fails
		 */
		static LocalIdBuilder parse( const std::string& localIdStr );

		/**
		 * @brief Static method to try parsing a LocalId string
		 * @param localIdStr String to parse
		 * @param[out] errors Object to receive parsing errors
		 * @param[out] localId Optional to receive the parsed builder
		 * @return true if parsing succeeded
		 */
		static bool tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<LocalIdBuilder>& localId );

		/**
		 * @brief Static method to try parsing a LocalId string with detailed errors
		 * @param localIdStr String to parse
		 * @param[out] errorBuilder Object to receive detailed parsing errors
		 * @param[out] localIdBuilder Optional to receive the parsed builder
		 * @return true if parsing succeeded
		 */
		static bool tryParseInternal( const std::string& localIdStr, LocalIdParsingErrorBuilder& errorBuilder, std::optional<LocalIdBuilder>& localIdBuilder );

		//-------------------------------------------------------------------------
		// Operators and Utilities
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality operator
		 * @param other Builder to compare with
		 * @return true if equal
		 */
		bool operator==( const LocalIdBuilder& other ) const;

		/**
		 * @brief Inequality operator
		 * @param other Builder to compare with
		 * @return true if not equal
		 */
		bool operator!=( const LocalIdBuilder& other ) const;

		/**
		 * @brief Get hash code
		 * @return Hash code for this builder
		 */
		size_t hashCode() const;

	private:
		//-------------------------------------------------------------------------
		// Private Helper Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Adds a parsing error to the error builder
		 * @param errorBuilder Error builder to add the error to
		 * @param state State where the error occurred
		 * @param message Optional error message (empty for default message)
		 */
		static void addError( LocalIdParsingErrorBuilder& errorBuilder, LocalIdParsingState state, const std::string& message = "" );

		/**
		 * @brief Advances parser position and state
		 * @param i Current position index
		 * @param segment Current segment being parsed
		 * @param state Current parsing state to update
		 */
		static void advanceParser( size_t& i, const std::string& segment, LocalIdParsingState& state );

		/**
		 * @brief Advances parser position only
		 * @param i Current position index
		 * @param segment Current segment being parsed
		 */
		static void advanceParser( size_t& i, const std::string& segment );

		/**
		 * @brief Advances parser state only
		 * @param state Current parsing state
		 * @param to Target state to transition to
		 */
		static void advanceParser( LocalIdParsingState& state, LocalIdParsingState to );

		/**
		 * @brief Advances both parser position and state
		 * @param i Current position index
		 * @param segment Current segment being parsed
		 * @param state Current parsing state
		 * @param to Target state to transition to
		 */
		static void advanceParser( size_t& i, const std::string& segment, LocalIdParsingState& state, LocalIdParsingState to );

		/**
		 * @brief Advances parser position and state using string_view
		 * @param i Current position index
		 * @param segment Current segment being parsed as string_view
		 * @param state Current parsing state to update
		 */
		static void advanceParser( size_t& i, const std::string_view& segment, LocalIdParsingState& state );

		/**
		 * @brief Advances parser position only using string_view
		 * @param i Current position index
		 * @param segment Current segment being parsed as string_view
		 */
		static void advanceParser( size_t& i, const std::string_view& segment );

		/**
		 * @brief Advances both parser position and state using string_view
		 * @param i Current position index
		 * @param segment Current segment being parsed as string_view
		 * @param state Current parsing state
		 * @param to Target state to transition to
		 */
		static void advanceParser( size_t& i, const std::string_view& segment, LocalIdParsingState& state, LocalIdParsingState to );

		/**
		 * @brief Gets the indexes for transitioning to the next state
		 * @param str String being parsed
		 * @param state Current parsing state
		 * @return Pair of (start index, end index) for the next segment
		 */
		static std::pair<size_t, size_t> nextStateIndexes( const std::string& str, LocalIdParsingState state );

		/**
		 * @brief Converts a metadata prefix string to its corresponding parsing state
		 * @param prefix String prefix to convert (e.g., "qty", "q", "cnt", etc.)
		 * @return The corresponding LocalIdParsingState if the prefix is valid, nullopt otherwise
		 */
		static std::optional<LocalIdParsingState> metaPrefixToState( const std::string_view& prefix );

		/**
		 * @brief Determines the next parsing state in the metadata state machine sequence
		 * @param prev Current parsing state
		 * @return The next state in the sequence if one exists, nullopt if at the end of the sequence
		 */
		static std::optional<LocalIdParsingState> nextParsingState( LocalIdParsingState prev );

		/**
		 * @brief Parses a metadata tag from the string
		 * @param codebookName Codebook name for the tag
		 * @param state Current parsing state
		 * @param i Current position index
		 * @param segment Current segment being parsed
		 * @param tag Optional MetadataTag to fill
		 * @param codebooks Codebooks object for validation
		 * @param errorBuilder Error builder to report errors
		 * @return true if parsing succeeded, false otherwise
		 */
		static bool parseMetaTag( CodebookName codebookName, LocalIdParsingState& state,
			size_t& i, const std::string_view& segment, std::optional<MetadataTag>& tag,
			const std::shared_ptr<Codebooks>& codebooks, LocalIdParsingErrorBuilder& errorBuilder );

		//-------------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------------

		std::optional<VisVersion> m_visVersion;
		bool m_verboseMode = false;
		LocalIdItems m_items;
		std::optional<MetadataTag> m_quantity;
		std::optional<MetadataTag> m_content;
		std::optional<MetadataTag> m_calculation;
		std::optional<MetadataTag> m_state;
		std::optional<MetadataTag> m_command;
		std::optional<MetadataTag> m_type;
		std::optional<MetadataTag> m_position;
		std::optional<MetadataTag> m_detail;
	};
}
