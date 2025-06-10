/**
 * @file LocalIdBuilder.h
 * @brief High-performance fluent builder for LocalId objects.
 * @details This file contains the definition of the LocalIdBuilder class,
 *          which provides a concrete implementation using direct value storage
 *          and move semantics for creating LocalId objects with an immutable
 *          fluent builder pattern.
 */

#pragma once

#include "LocalIdItems.h"
#include "MetadataTag.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class Codebooks;
	class LocalId;
	class LocalIdParsingErrorBuilder;
	class ParsingErrors;

	enum class CodebookName;
	enum class LocalIdParsingState;
	enum class VisVersion;

	//=====================================================================
	// LocalIdBuilder class
	//=====================================================================

	/**
	 * @class LocalIdBuilder
	 * @brief High-performance fluent builder for LocalId objects.
	 *
	 * @details Concrete implementation using immutable fluent pattern with direct value storage.
	 * Provides methods for setting all properties required to build a valid LocalId, plus
	 * parsing capabilities to create builder instances from string representations.
	 */
	class LocalIdBuilder final
	{
	public:
		//----------------------------------------------
		// Constants
		//----------------------------------------------

		/**
		 * @brief Standard naming rule prefix expected for Local IDs.
		 * @details Defines the standard prefix "/dnv-v2" used in VIS Local ID format.
		 * @note Used during parsing and string generation for validation.
		 */
		static const std::string namingRule;

		/** @brief List of standard `CodebookName` values used directly within the LocalId structure. */
		static const std::vector<CodebookName> usedCodebooks;

		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

	protected:
		/** @brief Default constructor. */
		LocalIdBuilder() = default;

	public:
		/** @brief Copy constructor */
		LocalIdBuilder( const LocalIdBuilder& ) = default;

		/** @brief Move constructor */
		LocalIdBuilder( LocalIdBuilder&& ) noexcept = default;

		/** @brief Destructor */
		~LocalIdBuilder() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocalIdBuilder& operator=( const LocalIdBuilder& ) = delete;

		/** @brief Move assignment operator */
		inline LocalIdBuilder& operator=( LocalIdBuilder&& ) noexcept;

		//----------------------------------------------
		// Operators
		//----------------------------------------------

		/**
		 * @brief Equality comparison operator.
		 * @details Delegates to the `equals` method for state comparison.
		 * @param[in] other The builder instance to compare against.
		 * @return True if the builders represent the same state, false otherwise.
		 * @see equals
		 */
		inline bool operator==( const LocalIdBuilder& other ) const;

		/**
		 * @brief Inequality comparison operator.
		 * @details Returns the negation of the `equals` method result.
		 * @param[in] other The builder instance to compare against.
		 * @return True if the builders represent different states, false otherwise.
		 * @see equals
		 */
		inline bool operator!=( const LocalIdBuilder& other ) const;

		/**
		 * @brief Checks for logical equality between this builder's state and another's.
		 * @details Compares all relevant configuration aspects (VIS version, items, tags, modes)
		 *          to determine if two builders would produce equivalent `LocalId` objects.
		 * @param[in] other The other `LocalIdBuilder` instance to compare against.
		 * @return True if the builders represent the same logical state, false otherwise.
		 */
		inline bool equals( const LocalIdBuilder& other ) const;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the VIS version currently set in the builder, if any.
		 * @return An `std::optional<VisVersion>` containing the VIS version if set,
		 *         or `std::nullopt` if no version is set.
		 */
		[[nodiscard]] inline std::optional<VisVersion> visVersion() const noexcept;

		/**
		 * @brief Gets the primary item path.
		 * @details The primary item is mandatory for a valid `LocalId`.
		 *          This implementation returns a reference to the internal path.
		 * @return A const reference to the primary item's `GmodPath`.
		 * @warning Behavior is undefined if called when no primary item is set (`isValid()` is false).
		 */
		[[nodiscard]] inline const std::optional<GmodPath>& primaryItem() const noexcept;

		/**
		 * @brief Gets the secondary item path, if one is set.
		 * @return A const reference to an `std::optional<GmodPath>` containing the
		 *         secondary item path if set, or `std::nullopt` otherwise.
		 */
		[[nodiscard]] inline const std::optional<GmodPath>& secondaryItem() const noexcept;

		/**
		 * @brief Gets all metadata tags currently set in the builder.
		 * @details Returns a collection of the `MetadataTag` objects configured.
		 *          The order within the vector corresponds to the standard Local ID format.
		 * @return A vector containing copies of the MetadataTag objects currently set.
		 */
		[[nodiscard]] inline std::vector<MetadataTag> metadataTags() const noexcept;

		/**
		 * @brief Calculate hash code based on builder content.
		 * @details Computes a hash value representing the current state of the builder,
		 *          suitable for use in hash-based containers like `std::unordered_set`.
		 *          Builders that are equal according to `equals()` must produce the same hash code.
		 * @return A `size_t` hash code.
		 * @see equals
		 */
		[[nodiscard]] inline size_t hashCode() const noexcept;

		//----------------------------------------------
		// State inspection methods
		//----------------------------------------------

		/**
		 * @brief Checks if the builder state is valid to build a `LocalId`.
		 * @details Validity typically requires at least a VIS version and a primary item.
		 *          Specific rules depend on the `LocalId` definition.
		 * @return True if the current state allows for a successful `build()`, false otherwise.
		 */
		[[nodiscard]] inline bool isValid() const noexcept;

		/**
		 * @brief Checks if the builder is in its initial, empty state.
		 * @details An empty builder has no VIS version, no items, and no metadata tags set.
		 * @return True if the builder holds no configuration data, false otherwise.
		 */
		[[nodiscard]] inline bool isEmpty() const noexcept;

		/**
		 * @brief Checks if verbose mode is enabled for the `toString()` representation.
		 * @details Verbose mode typically includes descriptive text alongside codes in the string output.
		 * @return True if verbose mode is enabled, false otherwise.
		 */
		[[nodiscard]] inline bool isVerboseMode() const noexcept;

		//----------------------------------------------
		// Metadata inspection methods
		//----------------------------------------------

		/**
		 * @brief Checks if the builder has any custom (non-standard) metadata tags.
		 * @return True if the builder contains custom tags, false otherwise.
		 */
		[[nodiscard]] inline bool hasCustomTag() const noexcept;

		/**
		 * @brief Checks if the builder has no metadata tags set.
		 * @return True if no metadata tags (quantity, content, etc.) are present, false otherwise.
		 */
		[[nodiscard]] inline bool isEmptyMetadata() const noexcept;

		/**
		 * @brief Gets the internal `LocalIdItems` object containing primary and secondary items.
		 * @return A const reference to the `LocalIdItems` member.
		 */
		[[nodiscard]] inline const LocalIdItems& items() const noexcept;

		/**
		 * @brief Gets the quantity metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for quantity.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& quantity() const noexcept;

		/**
		 * @brief Gets the content metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for content.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& content() const noexcept;

		/**
		 * @brief Gets the calculation metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for calculation.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& calculation() const noexcept;

		/**
		 * @brief Gets the state metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for state.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& state() const noexcept;

		/**
		 * @brief Gets the command metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for command.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& command() const noexcept;

		/**
		 * @brief Gets the type metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for type.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& type() const noexcept;

		/**
		 * @brief Gets the position metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for position.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& position() const noexcept;

		/**
		 * @brief Gets the detail metadata tag, if present.
		 * @return A const reference to an `std::optional<MetadataTag>` for detail.
		 */
		[[nodiscard]] inline const std::optional<MetadataTag>& detail() const noexcept;

		//----------------------------------------------
		// String conversion
		//----------------------------------------------

		/**
		 * @brief Generates the string representation of the Local ID based on the current builder state.
		 * @details The format follows the standard Local ID string conventions.
		 *          The output is affected by the `isVerboseMode()` setting.
		 * @return A `std::string` representing the configured Local ID.
		 *         Returns an empty string if the state is not valid (`isValid()` is false).
		 */
		[[nodiscard]] std::string toString() const;

		/**
		 * @brief Appends the string representation of the Local ID to a stringstream.
		 * @details Efficiently builds the string representation directly into the provided stream.
		 * @param[in,out] builder The `std::stringstream` to append the representation to.
		 */
		void toString( std::stringstream& builder ) const;

		//----------------------------------------------
		// Static factory methods
		//----------------------------------------------

		/**
		 * @brief Creates a new builder instance initialized with the specified VIS version.
		 * @param[in] version The initial `VisVersion` for the builder.
		 * @return A new `LocalIdBuilder` instance.
		 */
		[[nodiscard]] static LocalIdBuilder create( VisVersion version );

		//----------------------------------------------
		// Build methods (Immutable fluent interface)
		//----------------------------------------------

		//----------------------------
		// Build
		//----------------------------

		/**
		 * @brief Creates the final `LocalId` object from the current builder state.
		 * @details Constructs and returns the target `LocalId` object based
		 *          on the configuration accumulated in the builder.
		 * @return A new instance of `LocalId`.
		 * @throws std::invalid_argument If the builder state is invalid (`isValid()` returns false).
		 */
		[[nodiscard]] LocalId build() const;

		//----------------------------
		// Verbose mode
		//----------------------------

		/**
		 * @brief Returns a new builder with the specified verbose mode setting.
		 * @param[in] verboseMode True to enable verbose mode for `toString()`, false to disable.
		 * @return A new `LocalIdBuilder` instance with the updated verbose mode setting.
		 */
		[[nodiscard]] LocalIdBuilder withVerboseMode( bool verboseMode );

		//----------------------------
		// VIS version
		//----------------------------

		/**
		 * @brief Returns a new builder with the VIS version set from a string.
		 * @details Parses the string (e.g., "vis-3.8") and sets the corresponding version.
		 * @param[in] visVersionStr The VIS version string to parse and set.
		 * @return A new `LocalIdBuilder` instance with the updated VIS version.
		 * @throws std::invalid_argument If the `visVersionStr` format is invalid or unrecognized.
		 */
		[[nodiscard]] LocalIdBuilder withVisVersion( const std::string& visVersionStr );

		/**
		 * @brief Returns a new builder with the VIS version set from an enum value.
		 * @param[in] version The `VisVersion` enum value to set.
		 * @return A new `LocalIdBuilder` instance with the updated VIS version.
		 */
		[[nodiscard]] LocalIdBuilder withVisVersion( VisVersion version );

		/**
		 * @brief Returns a new builder, potentially with the VIS version set from an optional enum.
		 * @details If `version` contains a value, the returned builder has that version set.
		 *          Otherwise, returns a builder identical to the current one (effectively a no-op).
		 * @param[in] version An `std::optional<VisVersion>` containing the version to set, if present.
		 * @return A new `LocalIdBuilder` instance, potentially updated.
		 */
		[[nodiscard]] LocalIdBuilder tryWithVisVersion( const std::optional<VisVersion>& version );

		/**
		 * @brief Returns a new builder with optional VIS version from string.
		 * @details If `visVersionStr` contains a value, attempts to parse and set it.
		 *          Reports success or failure via the `succeeded` output parameter.
		 * @param[in] visVersionStr An `std::optional<std::string>` containing the version string to set, if present.
		 * @param[out] succeeded Set to true if the version was successfully set (i.e., optional had a value
		 *                       and the string was valid), false otherwise.
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithVisVersion( const std::optional<std::string>& visVersionStr, bool& succeeded );

		/**
		 * @brief Returns a new builder, potentially with the VIS version set from an optional enum. Reports success.
		 * @details If `version` contains a value, the returned builder has that version set and `succeeded` is true.
		 *          Otherwise, `succeeded` is false and the builder is identical to the current one.
		 * @param[in] version An `std::optional<VisVersion>` containing the version to set, if present.
		 * @param[out] succeeded Set to true if the version was present and set, false otherwise.
		 * @return A new `LocalIdBuilder` instance, potentially updated.
		 */
		[[nodiscard]] LocalIdBuilder tryWithVisVersion( const std::optional<VisVersion>& version, bool& succeeded );

		/**
		 * @brief Returns a new builder with the VIS version removed.
		 * @return A new `LocalIdBuilder` instance without any VIS version set.
		 */
		[[nodiscard]] LocalIdBuilder withoutVisVersion();

		//----------------------------
		// Primary item
		//----------------------------

		/**
		 * @brief Returns a new builder with the primary item set (moves the provided path).
		 * @details Sets the primary item, taking ownership of the `item` via move semantics.
		 * @param[in] item The `GmodPath` to set as the primary item (rvalue reference, will be moved).
		 * @return A new `LocalIdBuilder` instance with the updated primary item.
		 * @throws std::invalid_argument If setting the primary item fails (e.g., path validation).
		 */
		[[nodiscard]] LocalIdBuilder withPrimaryItem( GmodPath&& item );

		/**
		 * @brief Returns a new builder, potentially with the primary item set (moves the provided path). Does not throw.
		 * @details Attempts to set the primary item via move semantics. If setting fails (e.g., validation),
		 *          it returns a builder identical to the current one.
		 * @param[in] item The `GmodPath` to attempt to set as primary (rvalue reference, will be moved).
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( GmodPath&& item );

		/**
		 * @brief Returns a new builder, potentially with the primary item set (moves the provided path). Reports success.
		 * @details Attempts to set the primary item via move semantics. Reports success or failure via `succeeded`.
		 * @param[in] item The `GmodPath` to attempt to set as primary (rvalue reference, will be moved).
		 * @param[out] succeeded Set to true if the primary item was successfully set, false otherwise.
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( GmodPath&& item, bool& succeeded );

		/**
		 * @brief Returns a new builder, potentially with the primary item set from an optional (moves if present). Does not throw.
		 * @details If `item` contains a value, attempts to set it as the primary item via move semantics.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @return A new `LocalIdBuilder` instance, potentially updated.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( std::optional<GmodPath>&& item );

		/**
		 * @brief Returns a new builder, potentially with the primary item set from an optional (moves if present). Reports success.
		 * @details If `item` contains a value, attempts to set it as the primary item via move semantics. Reports success.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @param[out] succeeded Set to true if the item was present and successfully set, false otherwise.
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithPrimaryItem( std::optional<GmodPath>&& item, bool& succeeded );

		/**
		 * @brief Returns a new builder with the primary item removed (reset to default/empty).
		 * @return A new `LocalIdBuilder` instance without a primary item set.
		 */
		[[nodiscard]] LocalIdBuilder withoutPrimaryItem();

		//----------------------------
		// Secondary item
		//----------------------------

		/**
		 * @brief Returns a new builder with the secondary item set (moves the provided path).
		 * @details Sets the secondary item, taking ownership of the `item` via move semantics.
		 * @param[in] item The `GmodPath` to set as the secondary item (rvalue reference, will be moved).
		 * @return A new `LocalIdBuilder` instance with the updated secondary item.
		 * @throws std::invalid_argument If setting the secondary item fails (e.g., path validation).
		 */
		[[nodiscard]] LocalIdBuilder withSecondaryItem( GmodPath&& item );

		/**
		 * @brief Returns a new builder, potentially with the secondary item set (moves the provided path). Does not throw.
		 * @details Attempts to set the secondary item via move semantics. If setting fails,
		 *          it returns a builder identical to the current one.
		 * @param[in] item The `GmodPath` to attempt to set as secondary (rvalue reference, will be moved).
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( GmodPath&& item );

		/**
		 * @brief Returns a new builder, potentially with the secondary item set (moves the provided path). Reports success.
		 * @details Attempts to set the secondary item via move semantics. Reports success or failure via `succeeded`.
		 * @param[in] item The `GmodPath` to attempt to set as secondary (rvalue reference, will be moved).
		 * @param[out] succeeded Set to true if the secondary item was successfully set, false otherwise.
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( GmodPath&& item, bool& succeeded );

		/**
		 * @brief Returns a new builder, potentially with the secondary item set from an optional (moves if present). Does not throw.
		 * @details If `item` contains a value, attempts to set it as the secondary item via move semantics.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @return A new `LocalIdBuilder` instance, potentially updated.
		 */
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item );

		/**
		 * @brief Returns a new builder, potentially with the secondary item set from an optional (moves if present). Reports success.
		 * @details If `item` contains a value, attempts to set it as the secondary item via move semantics. Reports success.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @param[out] succeeded Set to true if the item was present and successfully set, false otherwise.
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item, bool& succeeded );

		/**
		 * @brief Returns a new builder with the secondary item removed.
		 * @return A new `LocalIdBuilder` instance without a secondary item set.
		 */
		[[nodiscard]] LocalIdBuilder withoutSecondaryItem();

		//----------------------------
		// Metadata tags
		//----------------------------

		/**
		 * @brief Returns a new builder with the specified metadata tag added or replaced.
		 * @details Adds the `metadataTag`. If a tag with the same `CodebookName` already exists,
		 *          it is replaced. Delegates to the appropriate specific `with...` method (e.g., `withQuantity`).
		 * @param[in] metadataTag The `MetadataTag` to add or replace.
		 * @return A new `LocalIdBuilder` instance with the added or updated tag.
		 * @throws std::invalid_argument If the tag's `CodebookName` is not one of the standard metadata types
		 *         supported directly by `LocalId` (Quantity, Content, etc.).
		 */
		[[nodiscard]] LocalIdBuilder withMetadataTag( const MetadataTag& metadataTag );

		/**
		 * @brief Returns a new builder, potentially with the specified metadata tag added or replaced. Does not throw.
		 * @details If `metadataTag` contains a value, attempts to add or replace it using `withMetadataTag`.
		 *          Otherwise, returns a builder identical to the current one.
		 * @param[in] metadataTag An `std::optional<MetadataTag>` containing the tag to add/replace, if present.
		 * @return A new `LocalIdBuilder` instance, potentially updated.
		 */
		[[nodiscard]] LocalIdBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag );

		/**
		 * @brief Returns a new builder, potentially with the specified metadata tag added or replaced. Reports success.
		 * @details If `metadataTag` contains a value, attempts to add or replace it using `withMetadataTag`. Reports success via `succeeded`.
		 * @param[in] metadataTag An `std::optional<MetadataTag>` containing the tag to add/replace, if present.
		 * @param[out] succeeded Set to true if the tag was present and successfully added/replaced (and was a valid standard tag), false otherwise.
		 * @return A new `LocalIdBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] LocalIdBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag, bool& succeeded );

		/**
		 * @brief Returns a new builder with the specified metadata tag removed.
		 * @details Removes the metadata tag identified by its `CodebookName`. Delegates to the appropriate
		 *          specific `without...` method (e.g., `withoutQuantity`).
		 * @param[in] name The `CodebookName` of the tag to remove.
		 * @return A new `LocalIdBuilder` instance without the specified metadata tag.
		 * @throws std::invalid_argument If the `name` is not one of the standard metadata types supported by `LocalId`.
		 */
		[[nodiscard]] LocalIdBuilder withoutMetadataTag( CodebookName name );

		//----------------------------------------------
		// Specific metadata tag builder methods
		//----------------------------------------------

		//----------------------------
		// Quantity
		//----------------------------

		/**
		 * @brief Returns a new builder with the quantity metadata tag set.
		 * @param[in] quantity The quantity tag. Must have `CodebookName::Quantity`.
		 * @return A new `LocalIdBuilder` instance with the updated quantity tag.
		 * @throws std::invalid_argument If `quantity.name()` is not `CodebookName::Quantity`.
		 */
		[[nodiscard]] LocalIdBuilder withQuantity( const MetadataTag& quantity );

		/**
		 * @brief Returns a new builder with the quantity metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the quantity tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutQuantity();

		//----------------------------
		// Content
		//----------------------------

		/**
		 * @brief Returns a new builder with the content metadata tag set.
		 * @param[in] content The content tag. Must have `CodebookName::Content`.
		 * @return A new `LocalIdBuilder` instance with the updated content tag.
		 * @throws std::invalid_argument If `content.name()` is not `CodebookName::Content`.
		 */
		[[nodiscard]] LocalIdBuilder withContent( const MetadataTag& content );

		/**
		 * @brief Returns a new builder with the content metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the content tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutContent();

		//----------------------------
		// Calculation
		//----------------------------

		/**
		 * @brief Returns a new builder with the calculation metadata tag set.
		 * @param[in] calculation The calculation tag. Must have `CodebookName::Calculation`.
		 * @return A new `LocalIdBuilder` instance with the updated calculation tag.
		 * @throws std::invalid_argument If `calculation.name()` is not `CodebookName::Calculation`.
		 */
		[[nodiscard]] LocalIdBuilder withCalculation( const MetadataTag& calculation );

		/**
		 * @brief Returns a new builder with the calculation metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the calculation tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutCalculation();

		//----------------------------
		// State
		//----------------------------

		/**
		 * @brief Returns a new builder with the state metadata tag set.
		 * @param[in] state The state tag. Must have `CodebookName::State`.
		 * @return A new `LocalIdBuilder` instance with the updated state tag.
		 * @throws std::invalid_argument If `state.name()` is not `CodebookName::State`.
		 */
		[[nodiscard]] LocalIdBuilder withState( const MetadataTag& state );

		/**
		 * @brief Returns a new builder with the state metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the state tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutState();

		//----------------------------
		// Command
		//----------------------------

		/**
		 * @brief Returns a new builder with the command metadata tag set.
		 * @param[in] command The command tag. Must have `CodebookName::Command`.
		 * @return A new `LocalIdBuilder` instance with the updated command tag.
		 * @throws std::invalid_argument If `command.name()` is not `CodebookName::Command`.
		 */
		[[nodiscard]] LocalIdBuilder withCommand( const MetadataTag& command );

		/**
		 * @brief Returns a new builder with the command metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the command tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutCommand();

		//----------------------------
		// Type
		//----------------------------

		/**
		 * @brief Returns a new builder with the type metadata tag set.
		 * @param[in] type The type tag. Must have `CodebookName::Type`.
		 * @return A new `LocalIdBuilder` instance with the updated type tag.
		 * @throws std::invalid_argument If `type.name()` is not `CodebookName::Type`.
		 */
		[[nodiscard]] LocalIdBuilder withType( const MetadataTag& type );

		/**
		 * @brief Returns a new builder with the type metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the type tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutType();

		//----------------------------
		// Position
		//----------------------------

		/**
		 * @brief Returns a new builder with the position metadata tag set.
		 * @param[in] position The position tag. Must have `CodebookName::Position`.
		 * @return A new `LocalIdBuilder` instance with the updated position tag.
		 * @throws std::invalid_argument If `position.name()` is not `CodebookName::Position`.
		 */
		[[nodiscard]] LocalIdBuilder withPosition( const MetadataTag& position );

		/**
		 * @brief Returns a new builder with the position metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the position tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutPosition();

		//----------------------------
		// Detail
		//----------------------------

		/**
		 * @brief Returns a new builder with the detail metadata tag set.
		 * @param[in] detail The detail tag. Must have `CodebookName::Detail`.
		 * @return A new `LocalIdBuilder` instance with the updated detail tag.
		 * @throws std::invalid_argument If `detail.name()` is not `CodebookName::Detail`.
		 */
		[[nodiscard]] LocalIdBuilder withDetail( const MetadataTag& detail );

		/**
		 * @brief Returns a new builder with the detail metadata tag removed.
		 * @return A new `LocalIdBuilder` instance without the detail tag.
		 */
		[[nodiscard]] LocalIdBuilder withoutDetail();

		//----------------------------------------------
		// Static parsing methods
		//----------------------------------------------

		/**
		 * @brief Parses a string representation into a `LocalIdBuilder` instance.
		 * @details Creates a `LocalIdBuilder` configured according to the parsed `localIdStr`.
		 *          Expects the standard Local ID format (e.g., "/dnv-v2/vis-3.0/...").
		 * @param[in] localIdStr The Local ID string to parse.
		 * @return A new `LocalIdBuilder` instance representing the parsed string.
		 * @throws std::invalid_argument If parsing fails due to invalid format or content.
		 */
		[[nodiscard]] static LocalIdBuilder parse( std::string_view localIdStr );

		/**
		 * @brief Attempts to parse a string representation into a `LocalIdBuilder` instance. Does not throw.
		 * @details Tries to create a `LocalIdBuilder` from `localIdStr`. If successful, the result
		 *          is placed in the `localId` output parameter. No detailed error information is provided on failure.
		 * @param[in] localIdStr The Local ID string to parse.
		 * @param[out] localId An `std::optional<LocalIdBuilder>` that will contain the resulting builder
		 *                     if parsing succeeds, or `std::nullopt` otherwise.
		 * @return True if parsing was successful, false otherwise.
		 */
		[[nodiscard]] static bool tryParse( std::string_view localIdStr, std::optional<LocalIdBuilder>& localId );

		/**
		 * @brief Attempts to parse a string representation into a `LocalIdBuilder` instance, providing error details.
		 * @details Tries to create a `LocalIdBuilder` from `localIdStr`. If successful, the result
		 *          is placed in `localId`. If parsing fails, detailed error information is added to `errors`.
		 * @param[in] localIdStr The Local ID string to parse.
		 * @param[out] errors A `ParsingErrors` object to collect detailed error information if parsing fails.
		 * @param[out] localId An `std::optional<LocalIdBuilder>` that will contain the resulting builder
		 *                     if parsing succeeds, or `std::nullopt` otherwise.
		 * @return True if parsing was successful, false otherwise.
		 */
		[[nodiscard]] static bool tryParse( std::string_view localIdStr, ParsingErrors& errors, std::optional<LocalIdBuilder>& localId );

	private:
		//----------------------------------------------
		// Private static helper parsing methods
		//----------------------------------------------

		/**
		 * @brief Internal core parsing logic used by public `tryParse` methods.
		 * @param[in] localIdStr The complete Local ID string to parse.
		 * @param[in,out] errorBuilder A helper object to accumulate parsing errors.
		 * @param[out] localIdBuilder Output parameter where the successfully parsed builder is placed.
		 * @return True if parsing succeeded (potentially with non-critical errors recorded), false if a critical error occurred.
		 */
		[[nodiscard]] static bool tryParseInternal(
			std::string_view localIdStr, LocalIdParsingErrorBuilder& errorBuilder, std::optional<LocalIdBuilder>& localIdBuilder );

		/**
		 * @brief Advances the parsing index `i` past the current `segment` and the following separator '/'.
		 * @param[in,out] i The current parsing index within the input string.
		 * @param[in] segment The string view representing the segment just processed.
		 */
		static void advanceParser( size_t& i, std::string_view segment );

		/**
		 * @brief Advances the parsing index `i` and updates the parsing `state`.
		 * @param[in,out] i The current parsing index.
		 * @param[in] segment The segment just processed.
		 * @param[in,out] state The current parsing state (will be updated based on standard progression).
		 */
		static void advanceParser( size_t& i, std::string_view segment, LocalIdParsingState& state );

		/**
		 * @brief Advances the parsing index `i` and explicitly sets the parsing `state` to `to`.
		 * @param[in,out] i The current parsing index.
		 * @param[in] segment The segment just processed.
		 * @param[in,out] state The current parsing state (will be set to `to`).
		 * @param[in] to The target `LocalIdParsingState` to transition to.
		 */
		static void advanceParser( size_t& i, std::string_view segment, LocalIdParsingState& state, LocalIdParsingState to );

		/**
		 * @brief Explicitly sets the parsing `state` to `to`.
		 * @param[in,out] state The current parsing state (will be set to `to`).
		 * @param[in] to The target `LocalIdParsingState` to transition to.
		 */
		static void advanceParser( LocalIdParsingState& state, LocalIdParsingState to );

		/**
		 * @brief Converts a metadata prefix string (e.g., "q", "qty") to its corresponding `LocalIdParsingState`.
		 * @param[in] prefix The string view representing the metadata prefix.
		 * @return An `std::optional<LocalIdParsingState>` containing the state if the prefix is recognized,
		 *         or `std::nullopt` otherwise.
		 */
		static std::optional<LocalIdParsingState> metaPrefixToState( std::string_view prefix );

		/**
		 * @brief Determines the expected next parsing state in the standard metadata sequence.
		 * @param[in] prev The previous `LocalIdParsingState` (must be a metadata state).
		 * @return An `std::optional<LocalIdParsingState>` containing the next expected state,
		 *         or `std::nullopt` if `prev` is the last state in the sequence (e.g., MetaDetail).
		 */
		static std::optional<LocalIdParsingState> nextParsingState( LocalIdParsingState prev );

		/**
		 * @brief Parses a single metadata tag segment (e.g., "q-value" or "qty-value").
		 * @param[in] codebookName The expected `CodebookName` for this tag.
		 * @param[in,out] state The current parsing state (updated on success).
		 * @param[in,out] i The current parsing index (updated on success).
		 * @param[in] segment The string view representing the full metadata segment (e.g., "q-value").
		 * @param[out] tag Output parameter where the successfully parsed `MetadataTag` is placed.
		 * @param[in] codebooks A pointer to the loaded codebooks, used for validation.
		 * @param[in,out] errorBuilder Used to record errors encountered during parsing.
		 * @return True if the segment was successfully parsed as the expected tag, false otherwise.
		 */
		static bool parseMetaTag( CodebookName codebookName, LocalIdParsingState& state,
			size_t& i, std::string_view segment, std::optional<MetadataTag>& tag,
			const Codebooks* codebooks,
			LocalIdParsingErrorBuilder& errorBuilder );

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The VIS version, if set. */
		std::optional<VisVersion> m_visVersion;

		/** @brief Flag indicating verbose mode for `toString()`. Defaults to false. */
		bool m_verboseMode = false;

		/** @brief Holds the primary and optional secondary GmodPath items. */
		LocalIdItems m_items;

		/** @brief Quantity metadata tag, if set. */
		std::optional<MetadataTag> m_quantity;

		/** @brief Content metadata tag, if set. */
		std::optional<MetadataTag> m_content;

		/** @brief Calculation metadata tag, if set. */
		std::optional<MetadataTag> m_calculation;

		/** @brief State metadata tag, if set. */
		std::optional<MetadataTag> m_state;

		/** @brief Command metadata tag, if set. */
		std::optional<MetadataTag> m_command;

		/** @brief Type metadata tag, if set. */
		std::optional<MetadataTag> m_type;

		/** @brief Position metadata tag, if set. */
		std::optional<MetadataTag> m_position;

		/** @brief Detail metadata tag, if set. */
		std::optional<MetadataTag> m_detail;
	};
}

#include "LocalIdBuilder.inl"
