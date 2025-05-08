/**
 * @file ILocalIdBuilder.h
 * @brief Defines the abstract interface for Local ID builders.
 * @details This file contains the definition of the `ILocalIdBuilder` template class,
 *          which serves as the contract for classes responsible for constructing
 *          Local ID objects using an immutable fluent builder pattern.
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class GmodPath;
	class MetadataTag;
	class ParsingErrors;
	enum class VisVersion;
	enum class CodebookName;

	/**
	 * @interface ILocalIdBuilder
	 * @brief Abstract interface for building Local IDs using an immutable fluent pattern.
	 *
	 * @details This template interface defines the contract for creating Local ID objects (`TResult`).
	 * It employs the Curiously Recurring Template Pattern (CRTP) where `TBuilder` is the
	 * concrete derived builder class. This pattern enables builder methods (`with...`, `tryWith...`, `without...`)
	 * to return the concrete type `TBuilder`, facilitating method chaining while maintaining immutability
	 * (each builder method returns a new, modified instance).
	 *
	 * Implementations of this interface are expected to be immutable; calling a builder
	 * method should not modify the current instance but return a new instance reflecting
	 * the requested change.
	 *
	 * @tparam TBuilder The concrete builder type that derives from this interface (CRTP).
	 * @tparam TResult The type of the Local ID object produced by the `build()` method.
	 */
	template <typename TBuilder, typename TResult>
	class ILocalIdBuilder
	{
		//=====================================================================
		// Construction / Destruction
		//=====================================================================

	protected:
		/** @brief Default constructor. */
		ILocalIdBuilder() = default;

	public:
		/** @brief Copy constructor */
		ILocalIdBuilder( const ILocalIdBuilder& ) = delete;

		/** @brief Move constructor */
		ILocalIdBuilder( ILocalIdBuilder&& ) noexcept = default;

		/** @brief Destructor */
		virtual ~ILocalIdBuilder() = default;

		//=====================================================================
		// Assignment Operators
		//=====================================================================

		/** @brief Copy assignment operator */
		ILocalIdBuilder& operator=( const ILocalIdBuilder& ) = delete;

		/** @brief Move assignment operator */
		ILocalIdBuilder& operator=( ILocalIdBuilder&& ) noexcept = default;

		//=====================================================================
		// Operators
		//=====================================================================

		/**
		 * @brief Equality comparison operator.
		 * @details Delegates to the virtual `equals` method for state comparison.
		 * @param[in] other The builder instance to compare against.
		 * @return True if the builders represent the same state, false otherwise.
		 * @see equals
		 */
		[[nodiscard]] bool operator==( const TBuilder& other ) const noexcept;

		/**
		 * @brief Inequality comparison operator.
		 * @details Returns the negation of the `equals` method result.
		 * @param[in] other The builder instance to compare against.
		 * @return True if the builders represent different states, false otherwise.
		 * @see equals
		 */
		[[nodiscard]] bool operator!=( const TBuilder& other ) const noexcept;

		//=====================================================================
		// Core Build Method
		//=====================================================================

		/**
		 * @brief Creates the final Local ID object from the current builder state.
		 * @details This method constructs and returns the target `TResult` object based
		 *          on the configuration accumulated in the builder.
		 * @return A new instance of the Local ID (`TResult`).
		 * @throws std::invalid_argument If the builder state is invalid (`isValid()` returns false).
		 *         Implementations should throw if building is attempted on an invalid state.
		 */
		[[nodiscard]] virtual TResult build() = 0;

		//=====================================================================
		// State Inspection Methods
		//=====================================================================

		/**
		 * @brief Checks if the builder state is valid to build a Local ID.
		 * @details Validity typically requires at least a VIS version, a primary item,
		 *          and one or more metadata tags, depending on the specific `TResult` rules.
		 * @return True if the current state allows for a successful `build()`, false otherwise.
		 */
		[[nodiscard]] virtual bool isValid() const noexcept = 0;

		/**
		 * @brief Checks if the builder is in its initial, empty state.
		 * @details An empty builder typically has no VIS version, no items, and no metadata tags set.
		 * @return True if the builder holds no configuration data, false otherwise.
		 */
		[[nodiscard]] virtual bool isEmpty() const noexcept = 0;

		/**
		 * @brief Gets the VIS version currently set in the builder, if any.
		 * @return An `std::optional<VisVersion>` containing the VIS version if set,
		 *         or `std::nullopt` if no version is set.
		 */
		[[nodiscard]] virtual std::optional<VisVersion> visVersion() const = 0;

		/**
		 * @brief Checks if verbose mode is enabled for the `toString()` representation.
		 * @details Verbose mode typically includes descriptive text alongside codes in the string output.
		 * @return True if verbose mode is enabled, false otherwise.
		 */
		[[nodiscard]] virtual bool isVerboseMode() const noexcept = 0;

		/**
		 * @brief Gets the primary item path.
		 * @details The primary item is usually mandatory for a valid Local ID.
		 *          Accessing this when no primary item is set might lead to undefined behavior
		 *          or exceptions in the concrete implementation, although `isValid()` should be false.
		 * @return A const reference to an `std::optional<GmodPath>` containing the
		 *         primary item path if set, or `std::nullopt` if no primary item is set.
		 * @warning Calling this when `isValid()` is false or no primary item is set might be unsafe,
		 *          though the returned optional would be empty in such cases.
		 */
		[[nodiscard]] virtual const std::optional<GmodPath>& primaryItem() const = 0;

		/**
		 * @brief Gets the secondary item path, if one is set.
		 * @return A const reference to an `std::optional<GmodPath>` containing the
		 *         secondary item path if set, or `std::nullopt` otherwise.
		 */
		[[nodiscard]] virtual const std::optional<GmodPath>& secondaryItem() const = 0;

		/**
		 * @brief Gets all metadata tags currently set in the builder.
		 * @details Returns a collection of the `MetadataTag` objects configured.
		 *          The order within the vector might correspond to the standard Local ID format
		 *          or be implementation-defined.
		 * @return A vector containing copies of the MetadataTag objects currently set.
		 */
		[[nodiscard]] virtual std::vector<MetadataTag> metadataTags() const = 0;

		/**
		 * @brief Checks if the builder has a custom metadata tag defined.
		 * @details A custom tag is typically one that isn't part of the standard VIS structure
		 *          but is user-defined.
		 * @return True if a custom tag is set, false otherwise.
		 */
		[[nodiscard]] virtual bool hasCustomTag() const noexcept = 0;

		//=====================================================================
		// Conversion and Comparison
		//=====================================================================

		/**
		 * @brief Generates the string representation of the Local ID based on the current builder state.
		 * @details The format should follow the standard Local ID string conventions.
		 *          The output might be affected by the `isVerboseMode()` setting.
		 * @return A `std::string` representing the configured Local ID.
		 *         Returns an empty or specific "invalid" string if the state is not valid.
		 */
		[[nodiscard]] virtual std::string toString() const = 0;

		/**
		 * @brief Checks for logical equality between this builder's state and another's.
		 * @details Compares all relevant configuration aspects (VIS version, items, tags, modes)
		 *          to determine if two builders would produce equivalent Local IDs.
		 * @param[in] other The other `TBuilder` instance to compare against.
		 * @return True if the builders represent the same logical state, false otherwise.
		 */
		[[nodiscard]] virtual bool equals( const TBuilder& other ) const = 0;

		//=====================================================================
		// Builder Methods (Immutable Fluent Interface)
		//=====================================================================

		//----------------------------------------------
		// Verbose Mode
		//----------------------------------------------

		/**
		 * @brief Returns a new builder with the specified verbose mode setting.
		 * @param[in] verboseMode True to enable verbose mode for `toString()`, false to disable.
		 * @return A new `TBuilder` instance with the updated verbose mode setting.
		 */
		[[nodiscard]] virtual TBuilder withVerboseMode( bool verboseMode ) = 0;

		//----------------------------------------------
		// VIS Version
		//----------------------------------------------

		/**
		 * @brief Returns a new builder with the VIS version set from a string.
		 * @details Parses the string (e.g., "vis-3.8") and sets the corresponding version.
		 * @param[in] visVersionStr The VIS version string to parse and set.
		 * @return A new `TBuilder` instance with the updated VIS version.
		 * @throws std::invalid_argument If the `visVersionStr` format is invalid or unrecognized.
		 */
		[[nodiscard]] virtual TBuilder withVisVersion( const std::string& visVersionStr ) = 0;

		/**
		 * @brief Returns a new builder with the VIS version set from an enum value.
		 * @param[in] version The `VisVersion` enum value to set.
		 * @return A new `TBuilder` instance with the updated VIS version.
		 */
		[[nodiscard]] virtual TBuilder withVisVersion( VisVersion version ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the VIS version set from an optional enum.
		 * @details If `version` contains a value, the returned builder has that version set.
		 *          Otherwise, returns a builder identical to the current one.
		 * @param[in] version An `std::optional<VisVersion>` containing the version to set, if present.
		 * @return A new `TBuilder` instance, potentially updated.
		 */
		[[nodiscard]] virtual TBuilder tryWithVisVersion( const std::optional<VisVersion>& version ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the VIS version set from an optional string.
		 * @details If `visVersionStr` contains a value, attempts to parse and set it.
		 *          Reports success or failure via the `succeeded` output parameter.
		 * @param[in] visVersionStr An `std::optional<std::string>` containing the version string to set, if present.
		 * @param[out] succeeded Set to true if the version was successfully set (i.e., optional had a value
		 *                       and the string was valid), false otherwise.
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithVisVersion( const std::optional<std::string>& visVersionStr, bool& succeeded ) = 0;

		/**
		 * @brief Returns a new builder with the VIS version removed.
		 * @return A new `TBuilder` instance without any VIS version set.
		 */
		[[nodiscard]] virtual TBuilder withoutVisVersion() = 0;

		//----------------------------------------------
		// Primary Item
		//----------------------------------------------

		/**
		 * @brief Returns a new builder with the primary item set (moves the provided path).
		 * @details Sets the primary item, taking ownership of the `item` via move semantics.
		 * @param[in] item The `GmodPath` to set as the primary item (rvalue reference, will be moved).
		 * @return A new `TBuilder` instance with the updated primary item.
		 * @throws std::invalid_argument If setting the primary item fails (e.g., path validation).
		 */
		[[nodiscard]] virtual TBuilder withPrimaryItem( GmodPath&& item ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the primary item set (moves the provided path). Does not throw.
		 * @details Attempts to set the primary item via move semantics. If setting fails (e.g., validation),
		 *          it returns a builder identical to the current one.
		 * @param[in] item The `GmodPath` to attempt to set as primary (rvalue reference, will be moved).
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithPrimaryItem( GmodPath&& item ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the primary item set (moves the provided path). Reports success.
		 * @details Attempts to set the primary item via move semantics. Reports success or failure via `succeeded`.
		 * @param[in] item The `GmodPath` to attempt to set as primary (rvalue reference, will be moved).
		 * @param[out] succeeded Set to true if the primary item was successfully set, false otherwise.
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithPrimaryItem( GmodPath&& item, bool& succeeded ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the primary item set from an optional (moves if present).
		 * @details If `item` contains a value, attempts to set it as the primary item via move semantics.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @return A new `TBuilder` instance, potentially updated.
		 */
		[[nodiscard]] virtual TBuilder tryWithPrimaryItem( std::optional<GmodPath>&& item ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the primary item set from an optional (moves if present). Reports success.
		 * @details If `item` contains a value, attempts to set it as the primary item via move semantics. Reports success.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @param[out] succeeded Set to true if the item was present and successfully set, false otherwise.
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithPrimaryItem( std::optional<GmodPath>&& item, bool& succeeded ) = 0;

		/**
		 * @brief Returns a new builder with the primary item removed.
		 * @return A new `TBuilder` instance without a primary item set.
		 */
		[[nodiscard]] virtual TBuilder withoutPrimaryItem() = 0;

		//----------------------------------------------
		// Secondary Item
		//----------------------------------------------

		/**
		 * @brief Returns a new builder with the secondary item set (moves the provided path).
		 * @details Sets the secondary item, taking ownership of the `item` via move semantics.
		 * @param[in] item The `GmodPath` to set as the secondary item (rvalue reference, will be moved).
		 * @return A new `TBuilder` instance with the updated secondary item.
		 * @throws std::invalid_argument If setting the secondary item fails (e.g., path validation).
		 */
		[[nodiscard]] virtual TBuilder withSecondaryItem( GmodPath&& item ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the secondary item set (moves the provided path). Does not throw.
		 * @details Attempts to set the secondary item via move semantics. If setting fails,
		 *          it returns a builder identical to the current one.
		 * @param[in] item The `GmodPath` to attempt to set as secondary (rvalue reference, will be moved).
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( GmodPath&& item ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the secondary item set (moves the provided path). Reports success.
		 * @details Attempts to set the secondary item via move semantics. Reports success or failure via `succeeded`.
		 * @param[in] item The `GmodPath` to attempt to set as secondary (rvalue reference, will be moved).
		 * @param[out] succeeded Set to true if the secondary item was successfully set, false otherwise.
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( GmodPath&& item, bool& succeeded ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the secondary item set from an optional (moves if present).
		 * @details If `item` contains a value, attempts to set it as the secondary item via move semantics.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @return A new `TBuilder` instance, potentially updated.
		 */
		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the secondary item set from an optional (moves if present). Reports success.
		 * @details If `item` contains a value, attempts to set it as the secondary item via move semantics. Reports success.
		 * @param[in] item An `std::optional<GmodPath>` containing the item to set, if present (rvalue reference).
		 * @param[out] succeeded Set to true if the item was present and successfully set, false otherwise.
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithSecondaryItem( std::optional<GmodPath>&& item, bool& succeeded ) = 0;

		/**
		 * @brief Returns a new builder with the secondary item removed.
		 * @return A new `TBuilder` instance without a secondary item set.
		 */
		[[nodiscard]] virtual TBuilder withoutSecondaryItem() = 0;

		//----------------------------------------------
		// Metadata Tags
		//----------------------------------------------

		/**
		 * @brief Returns a new builder with the specified metadata tag added or replaced.
		 * @details Adds the `metadataTag`. If a tag with the same `CodebookName` already exists,
		 *          it is replaced.
		 * @param[in] metadataTag The `MetadataTag` to add or replace.
		 * @return A new `TBuilder` instance with the added or updated tag.
		 * @throws std::invalid_argument If the tag's `CodebookName` is not one that is directly
		 *         supported as part of the Local ID structure (e.g., if it represents items).
		 */
		[[nodiscard]] virtual TBuilder withMetadataTag( const MetadataTag& metadataTag ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the specified metadata tag added or replaced.
		 * @details If `metadataTag` contains a value, adds or replaces it in the builder.
		 *          Otherwise, returns a builder identical to the current one.
		 * @param[in] metadataTag An `std::optional<MetadataTag>` containing the tag to add/replace, if present.
		 * @return A new `TBuilder` instance, potentially updated.
		 */
		[[nodiscard]] virtual TBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag ) = 0;

		/**
		 * @brief Returns a new builder, potentially with the specified metadata tag added or replaced. Reports success.
		 * @details If `metadataTag` contains a value, attempts to add or replace it. Reports success via `succeeded`.
		 * @param[in] metadataTag An `std::optional<MetadataTag>` containing the tag to add/replace, if present.
		 * @param[out] succeeded Set to true if the tag was present and successfully added/replaced, false otherwise.
		 * @return A new `TBuilder` instance, updated if successful, otherwise identical to the current one.
		 */
		[[nodiscard]] virtual TBuilder tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag, bool& succeeded ) = 0;

		/**
		 * @brief Returns a new builder with the specified metadata tag removed.
		 * @details Removes the metadata tag identified by its `CodebookName`.
		 * @param[in] name The `CodebookName` of the tag to remove.
		 * @return A new `TBuilder` instance without the specified metadata tag.
		 * @throws std::invalid_argument If the `name` is not one that is directly supported as part
		 *         of the Local ID structure.
		 */
		[[nodiscard]] virtual TBuilder withoutMetadataTag( CodebookName name ) = 0;

		//=====================================================================
		// Static Parsing Methods
		//=====================================================================

		/**
		 * @brief Parses a string representation into a builder instance.
		 * @details Creates a `TBuilder` configured according to the parsed `localIdStr`.
		 *          Delegates parsing logic to the concrete `TBuilder` implementation.
		 * @param[in] localIdStr The Local ID string to parse.
		 * @return A new `TBuilder` instance representing the parsed string.
		 * @throws std::invalid_argument If parsing fails due to invalid format or content.
		 */
		[[nodiscard]] static TBuilder parse( std::string_view localIdStr );

		/**
		 * @brief Attempts to parse a string representation into a builder instance. Does not throw.
		 * @details Tries to create a `TBuilder` from `localIdStr`. If successful, the result
		 *          is placed in the `builder` output parameter.
		 *          Delegates parsing logic to the concrete `TBuilder` implementation.
		 * @param[in] localIdStr The Local ID string to parse.
		 * @param[out] builder An `std::optional<TBuilder>` that will contain the resulting builder
		 *                     if parsing succeeds, or `std::nullopt` otherwise.
		 * @return True if parsing was successful, false otherwise.
		 */
		[[nodiscard]] static bool tryParse( std::string_view localIdStr, std::optional<TBuilder>& builder );

		/**
		 * @brief Attempts to parse a string representation into a builder instance, providing error details.
		 * @details Tries to create a `TBuilder` from `localIdStr`. If successful, the result
		 *          is placed in `builder`. If parsing fails, error information is added to `errors`.
		 *          Delegates parsing logic to the concrete `TBuilder` implementation.
		 * @param[in] localIdStr The Local ID string to parse.
		 * @param[out] errors A `ParsingErrors` object to collect detailed error information if parsing fails.
		 * @param[out] builder An `std::optional<TBuilder>` that will contain the resulting builder
		 *                     if parsing succeeds, or `std::nullopt` otherwise.
		 * @return True if parsing was successful, false otherwise.
		 */
		[[nodiscard]] static bool tryParse( std::string_view localIdStr, ParsingErrors& errors, std::optional<TBuilder>& builder );
	};
}

#include "ILocalIdBuilder.inl"
