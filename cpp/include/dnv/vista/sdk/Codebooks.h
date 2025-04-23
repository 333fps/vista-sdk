#pragma once

#include "Codebook.h"

namespace dnv::vista::sdk
{
	class MetadataTag;
	struct CodebooksDto;
	enum class CodebookName;
	enum class VisVersion;

	/**
	 * @brief Container for all codebooks in a specific VIS version
	 */
	class Codebooks final
	{
	public:
		/**
		 * @brief Iterator for codebooks
		 */
		class Iterator final
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::tuple<CodebookName, std::reference_wrapper<const Codebook>>;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type*;
			using reference = const value_type&;

			Iterator( const std::vector<Codebook>* codebooks, size_t index );

			reference operator*() const;
			pointer operator->() const;

			Iterator& operator++();
			Iterator operator++( int );

			bool operator==( const Iterator& other ) const;
			bool operator!=( const Iterator& other ) const;

			void reset();

		private:
			const std::vector<Codebook>* m_codebooks;
			size_t m_index;
			mutable std::optional<value_type> m_current;
		};

		using Enumerator = Iterator;

		/**
		 * @brief Default constructor
		 */
		Codebooks() = default;

		/**
		 * @brief Construct from DTO
		 * @param version The VIS version
		 * @param dto The codebooks data transfer object
		 */
		Codebooks( VisVersion version, const CodebooksDto& dto );

		/**
		 * @brief Access a codebook by name
		 * @param name The codebook name
		 * @return The codebook
		 * @throws std::invalid_argument If the name is invalid
		 */
		const Codebook& operator[]( CodebookName name ) const;

		/**
		 * @brief Get the VIS version
		 * @return The VIS version
		 */
		VisVersion visVersion() const;

		/**
		 * @brief Get a codebook by name
		 * @param name The codebook name
		 * @return The codebook
		 */
		const Codebook& codebook( CodebookName name ) const;

		/**
		 * @brief Try to create a metadata tag
		 * @param name The codebook name
		 * @param value The tag value
		 * @return The metadata tag, or nullopt if invalid
		 */
		std::optional<MetadataTag> tryCreateTag( CodebookName name, const std::string_view value ) const;

		/**
		 * @brief Create a metadata tag
		 * @param name The codebook name
		 * @param value The tag value
		 * @return The metadata tag
		 * @throws std::invalid_argument If the value is invalid
		 */
		MetadataTag createTag( CodebookName name, const std::string& value ) const;

		/**
		 * @brief Get iterator to the beginning
		 * @return Iterator to the first codebook
		 */
		Iterator begin() const;

		/**
		 * @brief Get iterator to the end
		 * @return Iterator past the last codebook
		 */
		Iterator end() const;

		/**
		 * @brief Get enumerator
		 * @return Iterator for enumerating codebooks
		 */
		Enumerator enumerator() const;

	private:
		VisVersion m_visVersion;
		std::vector<Codebook> m_codebooks;
	};
}
