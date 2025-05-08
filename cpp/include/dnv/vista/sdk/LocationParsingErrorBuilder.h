/**
 * @file LocationParsingErrorBuilder.h
 * @brief Defines the LocationParsingErrorBuilder class for collecting Location parsing errors.
 * @details This class provides a mechanism to accumulate errors encountered during the
 *          parsing of a Location string.
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class ParsingErrors;

	//=====================================================================
	// Enumerations
	//=====================================================================

	/**
	 * @brief Enumeration of location validation results.
	 *
	 * Indicates the outcome of a location string validation attempt.
	 */
	enum class LocationValidationResult
	{
		Invalid,
		InvalidCode,
		InvalidOrder,
		NullOrWhiteSpace,
		Valid
	};

	//=====================================================================
	// LocationParsingErrorBuilder Class
	//=====================================================================

	/**
	 * @class LocationParsingErrorBuilder
	 * @brief A builder class for accumulating errors encountered during Location string parsing.
	 *
	 * @details This class provides methods to add errors associated with specific validation results
	 *          (defined by `LocationValidationResult`) and finally builds a `ParsingErrors` object
	 *          containing the collected issues. It is used internally by the `Locations` parsing logic.
	 *          This class is designed to be non-copyable but movable to manage resource ownership clearly.
	 */
	class LocationParsingErrorBuilder final
	{
	public:
		//=====================================================================
		// Construction / Destruction
		//=====================================================================

		/** @brief Default constructor */
		LocationParsingErrorBuilder() = default;

		/** @brief Copy constructor */
		LocationParsingErrorBuilder( const LocationParsingErrorBuilder& ) = delete;

		/** @brief Move constructor */
		LocationParsingErrorBuilder( LocationParsingErrorBuilder&& ) noexcept = default;

		/** @brief Destructor */
		~LocationParsingErrorBuilder() = default;

		//=====================================================================
		// Assignment Operators
		//=====================================================================

		/** @brief Copy assignment operator */
		LocationParsingErrorBuilder& operator=( const LocationParsingErrorBuilder& ) = delete;

		/** @brief Move assignment operator */
		LocationParsingErrorBuilder& operator=( LocationParsingErrorBuilder&& ) noexcept = default;

		//=====================================================================
		// Static Factory Method
		//=====================================================================

		/**
		 * @brief Creates a new, empty LocationParsingErrorBuilder instance.
		 * @details Provides a clear entry point for creating a builder.
		 * @return A new instance of `LocationParsingErrorBuilder`.
		 */
		[[nodiscard]] static LocationParsingErrorBuilder create();

		//=====================================================================
		// Public Methods
		//=====================================================================

		/**
		 * @brief Adds an error with a specific validation result and message.
		 * @param[in] validationResult The `LocationValidationResult` indicating the type of error.
		 * @param[in] message The custom error message describing the issue.
		 * @return A reference to this builder instance for method chaining.
		 */
		LocationParsingErrorBuilder& addError( LocationValidationResult validationResult, const std::string& message );

		/**
		 * @brief Checks if any errors have been added to the builder.
		 * @return `true` if at least one error has been added, `false` otherwise.
		 */
		[[nodiscard]] bool hasError() const noexcept;

		//=====================================================================
		// Build Method
		//=====================================================================

		/**
		 * @brief Constructs a `ParsingErrors` object from the errors collected by this builder.
		 * @details Formats the collected validation result/message pairs into a final `ParsingErrors` object.
		 * @return A `ParsingErrors` object containing the formatted error messages.
		 *         Returns `ParsingErrors::Empty` if `hasError()` is false.
		 */
		[[nodiscard]] ParsingErrors build() const;

	private:
		//=====================================================================
		// Private Member Variables
		//=====================================================================

		/**
		 * @brief Internal storage for the collected parsing errors.
		 * @details Each pair contains the `LocationValidationResult` and the associated error message string.
		 */
		std::vector<std::pair<LocationValidationResult, std::string>> m_errors;
	};
}
