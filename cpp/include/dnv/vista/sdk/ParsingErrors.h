/**
 * @file ParsingErrors.h
 * @brief Defines the ParsingErrors class for managing parsing error collections.
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// ParsingErrors class
	//=====================================================================

	/**
	 * @brief Represents a collection of parsing errors.
	 *
	 * This class is used to store and manage errors encountered during parsing operations.
	 */
	class ParsingErrors final
	{
	public:
		//----------------------------------------------
		// Forward declarations
		//----------------------------------------------

		class Enumerator;
		struct ErrorEntry;

		//----------------------------------------------
		// Friends access
		//----------------------------------------------

		friend class LocalIdParsingErrorBuilder;
		friend class LocationParsingErrorBuilder;

		//----------------------------------------------
		// Public static members
		//----------------------------------------------

		/** @brief A static instance representing an empty set of parsing errors. */
		static const ParsingErrors Empty;

		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Internal constructor for creating ParsingErrors with error entries (copy).
		 * @param errors A vector of error entries to copy.
		 */
		explicit ParsingErrors( const std::vector<ErrorEntry>& errors );

		/**
		 * @brief Internal constructor for creating ParsingErrors with error entries (move).
		 * @param errors A vector of error entries to move from.
		 * @note This constructor takes ownership of the provided vector for optimal performance.
		 */
		explicit ParsingErrors( std::vector<ErrorEntry>&& errors ) noexcept;

	public:
		/** @brief Default constructor */
		ParsingErrors();

		/** @brief Copy constructor */
		ParsingErrors( const ParsingErrors& ) = default;

		/** @brief Move constructor */
		ParsingErrors( ParsingErrors&& errors ) noexcept;

		/** @brief Destructor */
		~ParsingErrors() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		ParsingErrors& operator=( const ParsingErrors& ) = default;

		/** @brief Move assignment operator */
		ParsingErrors& operator=( ParsingErrors&& ) noexcept = default;

		//----------------------------------------------
		// Operators
		//----------------------------------------------

		/**
		 * @brief Equality operator.
		 * @param other The other ParsingErrors object to compare.
		 * @return True if the two ParsingErrors objects are equal, false otherwise.
		 */
		[[nodiscard]] bool operator==( const ParsingErrors& other ) const noexcept;

		/**
		 * @brief Inequality operator.
		 * @param other The other ParsingErrors object to compare.
		 * @return True if the two ParsingErrors objects are not equal, false otherwise.
		 */
		[[nodiscard]] bool operator!=( const ParsingErrors& other ) const noexcept;

		/**
		 * @brief Checks if this ParsingErrors object is equal to another.
		 * @param other The other ParsingErrors object to compare.
		 * @return True if the two ParsingErrors objects are equal, false otherwise.
		 */
		[[nodiscard]] bool equals( const ParsingErrors& other ) const noexcept;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the number of error entries.
		 * @return The count of error entries.
		 */
		[[nodiscard]] size_t count() const noexcept;

		/**
		 * @brief Gets the hash code for this ParsingErrors object.
		 * @return The hash code as an unsigned integer.
		 */
		[[nodiscard]] size_t hashCode() const noexcept;

		//----------------------------------------------
		// State inspection methods
		//----------------------------------------------

		/**
		 * @brief Checks if there are any errors.
		 * @return True if there are errors, false otherwise.
		 */
		[[nodiscard]] bool hasErrors() const noexcept;

		/**
		 * @brief Checks if there is an error of a specific type.
		 * @param type The type of error to check for.
		 * @return True if an error of the specified type exists, false otherwise.
		 */
		[[nodiscard]] bool hasErrorType( std::string_view type ) const noexcept;

		//----------------------------------------------
		// String conversion methods
		//----------------------------------------------

		/**
		 * @brief Converts the parsing errors to a string representation.
		 * @return A string representation of the parsing errors.
		 */
		[[nodiscard]] std::string toString() const;

		//----------------------------------------------
		// Enumeration
		//----------------------------------------------

		/**
		 * @brief Gets an enumerator for the error entries.
		 * @return An enumerator for iterating through the errors.
		 */
		[[nodiscard]] Enumerator enumerator() const;

		//----------------------------------------------
		// ParsingErrors::enumerator class
		//----------------------------------------------

		/**
		 * @brief Enumerator for traversing parsing errors.
		 *
		 * Provides enumeration functionality for iterating through error entries.
		 * The enumerator starts positioned before the first element.
		 */
		class Enumerator final
		{
		public:
			//----------------------------
			// Construction / destruction
			//----------------------------

			/**
			 * @brief Constructs an enumerator for the given error data.
			 * @param data Pointer to the vector of error entries to enumerate.
			 */
			Enumerator( const std::vector<ErrorEntry>* data );

			/** @brief Default constructor */
			Enumerator() = delete;

			/** @brief Copy constructor */
			Enumerator( const Enumerator& ) = default;

			/** @brief Move constructor */
			Enumerator( Enumerator&& ) noexcept = default;

			/** @brief Destructor */
			~Enumerator() = default;

			//----------------------------
			// Assignment operators
			//----------------------------

			/** @brief Copy assignment operator */
			Enumerator& operator=( const Enumerator& ) = default;

			/** @brief Move assignment operator */
			Enumerator& operator=( Enumerator&& ) noexcept = default;

			//----------------------------
			// Enumeration methods
			//----------------------------

			/**
			 * @brief Advances the enumerator to the next element.
			 * @return True if the enumerator successfully moved to the next element;
			 *         false if the enumerator has passed the end of the collection.
			 */
			[[nodiscard]] bool next() noexcept;

			/**
			 * @brief Gets the current element.
			 * @return The current error entry.
			 * @throws std::out_of_range if enumerator is not positioned on a valid element
			 */
			[[nodiscard]] const ErrorEntry& current() const;

			/**
			 * @brief Resets the enumerator to its initial position.
			 */
			void reset() noexcept;

		private:
			//----------------------------
			// Private member variables
			//----------------------------

			const std::vector<ErrorEntry>* m_data;
			size_t m_index;
		};

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		std::vector<ErrorEntry> m_errors;

	public:
		//----------------------------------------------
		// ParsingErrors::ErrorEntry struct
		//----------------------------------------------

		struct ErrorEntry
		{
			std::string type;
			std::string message;

			ErrorEntry() = default;
			ErrorEntry( std::string_view type, std::string_view message );
			ErrorEntry( std::string&& type, std::string&& message );

			[[nodiscard]] bool operator==( const ErrorEntry& other ) const noexcept;
			[[nodiscard]] bool operator!=( const ErrorEntry& other ) const noexcept;
		};
	};
}
