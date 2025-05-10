/**
 * @file GmodPath.h
 * @brief Declarations for GmodPath and related classes for representing paths in the Generic Product Model (GMOD).
 * @details Defines the `GmodPath` class for representing hierarchical paths according to ISO 19848,
 *          along with helper classes for parsing, validation, iteration, and modification of path segments.
 */

#pragma once

#include "GmodNode.h"
#include "Locations.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward Declarations
	//=====================================================================

	enum class VisVersion;
	class Gmod;
	class Locations;
	class GmodPath;
	class GmodIndividualizableSet;
	class GmodParsePathResult;

	//=====================================================================
	// Internal Helper Classes
	//=====================================================================

	//----------------------------------------------
	// LocationSetsVisitor Struct
	//----------------------------------------------

	/**
	 * @brief Helper struct for identifying individualizable node sets within a GmodPath during traversal.
	 * @details Used internally by `GmodPath` methods to track consecutive individualizable nodes
	 *          and determine their common location according to GMOD rules. It operates on nodes
	 *          from a `GmodPath` instance.
	 */
	struct LocationSetsVisitor final
	{
		/** @brief Starting index of the current potential individualizable parent sequence within the path.
		 *         Initialized to a value indicating "not started" (e.g., std::numeric_limits<size_t>::max()). */
		size_t m_currentParentStart;

		/**
		 * @brief Default constructor. Initializes internal state.
		 */
		LocationSetsVisitor();

		/**
		 * @brief Visits a node during path traversal to identify potential individualizable sets.
		 * @param visitedNodeInstance The specific GmodNode instance from the path currently being evaluated by the caller's loop.
		 * @param visitedNodeIndex The 0-based index of `visitedNodeInstance` within the full path.
		 * @param allPathParentInstances A const reference to the vector of all parent GmodNode instances in the GmodPath being analyzed.
		 * @param pathTargetInstance A const reference to the target GmodNode instance of the GmodPath being analyzed.
		 * @return Optional tuple containing:
		 *         - size_t: Starting index (inclusive) of the identified individualizable set within the path.
		 *         - size_t: Ending index (inclusive) of the identified individualizable set within the path.
		 *         - std::optional<Location>: The common location associated with the set, if any.
		 *         Returns std::nullopt if the current node doesn't complete or form part of an individualizable set
		 *         according to the GMOD rules.
		 */
		std::optional<std::tuple<size_t, size_t, std::optional<Location>>> visit(
			const GmodNode& visitedNodeInstance,
			size_t visitedNodeIndex,
			const std::vector<GmodNode>& allPathParentInstances,
			const GmodNode& pathTargetInstance );
	};

	//=====================================================================
	// GmodPath Class
	//=====================================================================

	/**
	 * @class GmodPath
	 * @brief Represents a hierarchical path in the Generic Product Model (GMOD).
	 * @details A GmodPath consists of a sequence of owned parent GmodNode objects (`m_ownedParentNodes`)
	 *          and an owned target GmodNode (`m_ownedTargetNode`), forming a path through the
	 *          GMOD structure as defined in ISO 19848. These nodes are typically initialized from
	 *          GmodNode templates but are managed as distinct instances by the GmodPath. Each node
	 *          in the path may have an optional Location. This class uses move-only semantics for transfer
	 *          of ownership and manages its own node data.
	 */
	class GmodPath final
	{
		friend class GmodIndividualizableSet;

	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/** @brief Default constructor. */
		GmodPath();

		/**
		 * @brief Construct a path from a vector of parent node pointers and an owned target node.
		 * @details This is the primary constructor for creating valid GmodPath instances.
		 * @param parents Vector of const pointers to parent nodes, ordered from root towards the target.
		 *                The pointers themselves are copied, but the pointed-to nodes are not owned.
		 * @param target The target node at the end of the path (ownership is taken via move).
		 * @param visVersion The VIS version associated with this path and the GMOD it relates to.
		 * @param skipVerify If true (default), skips validation of parent-child relationships.
		 *                   If false, validates the path structure upon construction using `isValid()`.
		 * @throws std::invalid_argument If `skipVerify` is false and the provided path structure is invalid.
		 */
		GmodPath( const std::vector<const GmodNode*>& initialParentNodeTemplates,
			const GmodNode& initialTargetNodeTemplate,
			VisVersion visVersion,
			bool skipVerify = true );

		/** @brief Copy constructor */
		GmodPath( const GmodPath& );

		/** @brief Move constructor */
		GmodPath( GmodPath&& other ) noexcept = default;

		/** @brief Default destructor. */
		~GmodPath() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodPath& operator=( const GmodPath& );

		/** @brief Move assignment operator */
		GmodPath& operator=( GmodPath&& other ) noexcept = default;

		//----------------------------------------------
		// Operators
		//----------------------------------------------

		/**
		 * @brief Access a node pointer at a specific depth within the path (const version).
		 * @param depth Zero-based depth index (0 is the first parent/root, `length()-1` is the target node).
		 * @return Const pointer (`const GmodNode*`) to the node at the specified depth.
		 *         Returns a pointer to the parent from `m_nodes` or the address of `m_targetNode`.
		 * @throws std::out_of_range If the `depth` index is invalid for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		[[nodiscard]] const GmodNode* operator[]( size_t depth ) const;

		/**
		 * @brief Equality operator. Equivalent to calling `equals(other)`.
		 * @param other The GmodPath to compare with.
		 * @return True if the paths are equal, false otherwise.
		 * @see equals
		 */
		bool operator==( const GmodPath& other ) const;

		/**
		 * @brief Inequality operator. Equivalent to `!equals(other)`.
		 * @param other The GmodPath to compare with.
		 * @return True if the paths are not equal, false otherwise.
		 * @see equals
		 */
		bool operator!=( const GmodPath& other ) const;

		//----------------------------------------------
		// Public Methods
		//----------------------------------------------

		/**
		 * @brief Get the total number of nodes in the path (parents + target node).
		 * @return The length of the path (number of nodes). Returns 0 for an empty path.
		 */
		[[nodiscard]] size_t length() const noexcept;

		/**
		 * @brief Check if the path is empty (default constructed).
		 * @return True if the path is empty, false otherwise.
		 */
		[[nodiscard]] bool isEmpty() const noexcept;

		/**
		 * @brief Check if the target node of this path is designated as mappable in the GMOD.
		 * @return True if the target node is mappable, false otherwise. Returns false for an empty path.
		 *         Let's assume it returns false for empty path based on implementation.
		 */
		[[nodiscard]] bool isMappable() const noexcept;

		/**
		 * @brief Creates a new GmodPath instance identical to this one but with locations removed from all its nodes.
		 * @details Creates a new path by making new instances of all parent nodes and the target node,
		 *          each created using their respective `withoutLocation()` method.
		 * @return A new GmodPath instance where all constituent nodes are copies without locations.
		 *         Returns an empty path if called on an empty path.
		 */
		[[nodiscard]] GmodPath withoutLocations() const;

		/**
		 * @brief Performs deep equality comparison with another GmodPath.
		 * @details Compares VIS version, target node (including location), and all parent node pointers
		 *          by comparing the pointed-to nodes' content (including locations). Order matters.
		 * @param other The GmodPath to compare against.
		 * @return True if both paths represent the exact same sequence of nodes and locations, false otherwise.
		 */
		[[nodiscard]] bool equals( const GmodPath& other ) const;

		/**
		 * @brief Calculate a hash code for this GmodPath.
		 * @details Suitable for use in hash-based containers like `std::unordered_map`.
		 *          The hash considers the VIS version, the target node (including location),
		 *          and the content of all pointed-to parent nodes (including locations).
		 * @return A `size_t` hash code value. Returns 0 for an empty path.
		 */
		[[nodiscard]] size_t hashCode() const noexcept;

		/**
		 * @brief Check if the path contains any nodes marked as individualizable in the GMOD.
		 * @details Checks both the parent nodes (via pointers) and the owned target node.
		 * @return True if at least one node in the path is individualizable, false otherwise or for an empty path.
		 */
		[[nodiscard]] bool isIndividualizable() const;

		/**
		 * @brief Identifies and returns all sets of consecutive individualizable nodes within this path.
		 * @details An individualizable set represents a segment of the path where instance identifiers
		 *          can be applied, potentially sharing a common location. Uses `LocationSetsVisitor`.
		 * @warning The implementation of this method and `LocationSetsVisitor` needs review/update
		 *          to correctly handle the `std::vector<const GmodNode*>` structure.
		 * @return A vector of `GmodIndividualizableSet` objects representing the identified sets.
		 *         Returns an empty vector if the path is not individualizable or is empty.
		 *         The returned sets operate on a mutable reference to this path.
		 */
		[[nodiscard]] std::vector<GmodIndividualizableSet> individualizableSets();

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the VIS (Vessel Information Structure) version associated with this path.
		 * @return The `VisVersion` enum value. Returns `VisVersion::Unknown` for an empty path.
		 */
		[[nodiscard]] VisVersion visVersion() const noexcept;

		/**
		 * @brief Get a const reference to the owned target node (the last node) of the path.
		 * @return Const reference to the target `GmodNode`.
		 * @throws std::logic_error If called on an empty path.
		 */
		[[nodiscard]] const GmodNode& targetNode() const;

		//----------------------------------------------
		// Specific Accessors
		//----------------------------------------------

		/**
		 * @brief Get the normal assignment name for the node at a specific depth, if defined in the GMOD.
		 * @details Traverses the path downwards from the target to find the relevant assignment name
		 *          defined in the metadata of the node at `nodeDepth`.
		 * @param nodeDepth The zero-based depth index of the node (0 for the root parent, length()-1 for the target node).
		 * @return An `std::optional<std::string>` containing the normal assignment name if found, otherwise `std::nullopt`.
		 * @throws std::out_of_range If `nodeDepth` is invalid for this path.
		 * @throws std::logic_error If called on an empty path.
		 * @throws std::runtime_error If a null pointer is encountered in the path structure at or below `nodeDepth`.
		 */
		[[nodiscard]] std::optional<std::string> normalAssignmentName( size_t nodeDepth ) const;

		/**
		 * @brief Get all common names defined for the nodes in this path, along with their depths.
		 * @return A vector of pairs, where each pair contains the zero-based depth and the common name string.
		 *         Returns an empty vector for an empty path. Skips nodes if null pointers are encountered.
		 */
		[[nodiscard]] std::vector<std::pair<size_t, std::string>> commonNames() const;

		//----------------------------------------------
		// String Conversions
		//----------------------------------------------

		/**
		 * @brief Convert the path to its standard string representation (e.g., "CODE1/CODE2.LOC/CODE3").
		 * @details Uses '/' as the default separator. Includes location suffixes (`-LOC`) only for nodes
		 *          that actually have a location set. If no node has a location, only the target node code is returned.
		 * @return The string representation of the path. Returns an empty string for an empty path.
		 */
		[[nodiscard]] std::string toString() const;

		/**
		 * @brief Append the string representation of the path to a string stream.
		 * @details See `toString()` for format details.
		 * @param builder The `std::stringstream` to append to.
		 * @param separator The character to use as a separator between node codes (default: '/').
		 *                  Only used if locations are present.
		 */
		void toString( std::stringstream& builder, char separator = '/' ) const;

		/**
		 * @brief Convert the path to its full string representation, including all parent nodes explicitly.
		 * @details Format: "RootCode[-Loc]/ParentCode[-Loc]/.../TargetCode[-Loc]". Uses '/' separator.
		 * @return The full path string representation. Returns empty string for an empty path.
		 *         Represents null pointers as "[null]".
		 */
		[[nodiscard]] std::string toFullPathString() const;

		/**
		 * @brief Append the full path string representation to a string stream.
		 * @details See `toFullPathString()` for format details.
		 * @param builder The `std::stringstream` to append to.
		 */
		void toFullPathString( std::stringstream& builder ) const;

		/**
		 * @brief Generate a detailed string representation suitable for debugging purposes.
		 * @details Includes VIS version, node codes, locations, and mappable status.
		 * @return A detailed debug string. Returns description for empty path.
		 */
		[[nodiscard]] std::string toStringDump() const;

		/**
		 * @brief Append the detailed debug string representation to a string stream.
		 * @param builder The `std::stringstream` to append to.
		 */
		void toStringDump( std::stringstream& builder ) const;

		//----------------------------------------------
		// Static Validation Methods
		//----------------------------------------------

		/**
		 * @brief Statically validate the structural integrity of a potential path.
		 * @details Checks if the parent-child relationships between the provided node pointers and the target node are valid.
		 *          Assumes nodes are ordered root-to-target. Checks `nodes[i]` is parent of `nodes[i+1]`,
		 *          and `nodes.back()` is parent of `targetNode`. Also checks if `nodes[0]` is the root.
		 * @param nodes Vector of const pointers to potential parent nodes, ordered root to leaf.
		 * @param targetNode The potential target node.
		 * @return True if the sequence forms a valid path structure, false otherwise.
		 */
		[[nodiscard]] static bool isValid( const std::vector<const GmodNode*>& nodes, const GmodNode& targetNode );

		/**
		 * @brief Statically validate the structural integrity and identify the point of failure.
		 * @details Checks parent-child relationships as in `isValid`. If invalid, provides the index of the first invalid link.
		 * @param nodes Vector of const pointers to potential parent nodes, ordered root to leaf.
		 * @param targetNode The potential target node.
		 * @param[out] missingLinkAt Output parameter set to the index `i` where the parent-child link is broken.
		 *                         `i=0`: `nodes[0]` is not root.
		 *                         `0 < i < nodes.size()`: `nodes[i-1]` is not parent of `nodes[i]`.
		 *                         `i = nodes.size()`: `nodes.back()` is not parent of `targetNode`.
		 *                         Set to `std::numeric_limits<size_t>::max()` if the path is valid.
		 * @return True if the sequence forms a valid path structure, false otherwise.
		 */
		[[nodiscard]] static bool isValid( const std::vector<const GmodNode*>& nodes, const GmodNode& targetNode, size_t& missingLinkAt );

		//----------------------------------------------
		// Static Parsing Methods
		//----------------------------------------------

		/**
		 * @brief Parse a path string using the default GMOD and Locations for the specified VIS version.
		 * @details Loads required GMOD/Locations data based on `visVersion`.
		 * @param item The path string to parse (e.g., "CODE1/CODE2.LOC", "CODE3").
		 * @param visVersion The VIS version context.
		 * @return The parsed `GmodPath` object.
		 * @throws std::invalid_argument If parsing fails (invalid format, unknown codes/locations).
		 * @throws std::runtime_error If the required GMOD/Locations data for `visVersion` cannot be loaded.
		 */
		[[nodiscard]] static GmodPath parse(
			std::string_view item,
			VisVersion visVersion );

		[[nodiscard]] static GmodPath parse(
			const std::string& item,
			VisVersion visVersion,
			const Gmod& gmod );

		/**
		 * @brief Try to parse a path string using the default GMOD and Locations for the specified VIS version.
		 * @param item The path string to parse.
		 * @param visVersion The VIS version context.
		 * @param[out] path Output parameter where the parsed `GmodPath` will be stored if successful.
		 * @return True if parsing succeeded, false otherwise. Does not throw on parsing errors,
		 *         but may throw `std::runtime_error` if GMOD/Locations data is missing.
		 */
		[[nodiscard]] static bool tryParse(
			std::string_view item,
			VisVersion visVersion,
			GmodPath& path );

		/**
		 * @brief Parse a full path string using the default GMOD and Locations for the specified VIS version.
		 * @details A full path string explicitly includes all nodes from the root (e.g., "VE/HULL/SYS").
		 * @param pathStr The full path string to parse.
		 * @param visVersion The VIS version context.
		 * @return The parsed `GmodPath` object.
		 * @throws std::invalid_argument If parsing fails.
		 * @throws std::runtime_error If GMOD/Locations data is missing.
		 */
		[[nodiscard]] static GmodPath parseFullPath(
			std::string_view pathStr,
			VisVersion visVersion );

		[[nodiscard]] static GmodPath parseFullPath(
			const std::string& item,
			VisVersion visVersion,
			const Gmod& gmod );

		/**
		 * @brief Try to parse a full path string using the default GMOD and Locations for the specified VIS version.
		 * @param pathStr The full path string to parse.
		 * @param visVersion The VIS version context.
		 * @param[out] path Output parameter for the parsed `GmodPath` if successful.
		 * @return True if parsing succeeded, false otherwise. May throw `std::runtime_error` if data is missing.
		 */
		[[nodiscard]] static bool tryParseFullPath(
			const std::string& pathStr,
			VisVersion visVersion,
			GmodPath& path );

		/**
		 * @brief Try to parse a full path string (provided as string_view) using the default GMOD/Locations.
		 * @param pathStr The full path `std::string_view` to parse.
		 * @param visVersion The VIS version context.
		 * @param[out] path Output parameter for the parsed `GmodPath` if successful.
		 * @return True if parsing succeeded, false otherwise. May throw `std::runtime_error` if data is missing.
		 */
		[[nodiscard]] static bool tryParseFullPath(
			std::string_view pathStr,
			VisVersion visVersion,
			GmodPath& path );

		[[nodiscard]] static bool tryParseFullPath(
			const std::string& item,
			VisVersion visVersion,
			const Gmod& gmod,
			std::optional<GmodPath>& path );

		/**
		 * @brief Parse a path string using explicitly provided GMOD and Locations objects.
		 * @param item The path string to parse.
		 * @param gmod The `Gmod` object containing the model structure.
		 * @param locations The `Locations` object for resolving location codes.
		 * @return The parsed `GmodPath` object.
		 * @throws std::invalid_argument If parsing fails.
		 */
		[[nodiscard]] static GmodPath parse(
			const std::string& item,
			const Gmod& gmod,
			const Locations& locations );

		/**
		 * @brief Try to parse a path string using explicitly provided GMOD and Locations objects.
		 * @param item The path string to parse.
		 * @param gmod The `Gmod` object.
		 * @param locations The `Locations` object.
		 * @param[out] path Output parameter for the parsed `GmodPath` if successful.
		 * @return True if parsing succeeded, false otherwise. Does not throw on parsing errors.
		 */
		[[nodiscard]] static bool tryParse(
			const std::string& item,
			const Gmod& gmod,
			const Locations& locations,
			GmodPath& path );

		[[nodiscard]] static bool tryParse(
			const std::string& item,
			VisVersion visVersion,
			const Gmod& gmod,
			std::optional<GmodPath>& path );

		/**
		 * @brief Try to parse a full path string (provided as string_view) using explicit GMOD/Locations.
		 * @param pathStr The full path `std::string_view` to parse.
		 * @param gmod The `Gmod` object.
		 * @param locations The `Locations` object.
		 * @param[out] path Output parameter for the parsed `GmodPath` if successful.
		 * @return True if parsing succeeded, false otherwise. Does not throw on parsing errors.
		 */
		[[nodiscard]] static bool tryParseFullPath(
			std::string_view pathStr,
			const Gmod& gmod,
			const Locations& locations,
			GmodPath& path );

		//----------------------------------------------
		// Enumerator Inner Class
		//----------------------------------------------

		/**
		 * @class Enumerator
		 * @brief Provides forward iteration over the nodes of a GmodPath.
		 * @details Yields the zero-based depth and a const reference to the `GmodNode` at that depth.
		 *          Supports use in range-based for loops via its `begin()` and `end()` methods.
		 */
		class Enumerator final
		{
		public:
			//---------------------------
			// Construction / Reset
			//---------------------------

			/**
			 * @brief Construct a new Enumerator for a given GmodPath.
			 * @param path The `GmodPath` to enumerate over (const reference is stored).
			 * @param fromDepth Optional zero-based depth index to start enumeration from.
			 *                  If `std::nullopt` (default), starts from the beginning (depth 0).
			 * @throws std::out_of_range If `fromDepth` is specified and is >= `path.length()`.
			 * @throws std::logic_error If constructed for an empty path.
			 */
			explicit Enumerator( const GmodPath& path, std::optional<size_t> fromDepth = std::nullopt );

			/** @brief Deleted copy constructor. Enumerator holds reference and state. */
			Enumerator( const Enumerator& ) = delete;

			/** @brief Deleted move constructor. */
			Enumerator( Enumerator&& ) = delete;

			/** @brief Deleted copy assignment operator. */
			Enumerator& operator=( const Enumerator& ) = delete;

			/** @brief Deleted move assignment operator. */
			Enumerator& operator=( Enumerator&& ) = delete;

			/**
			 * @brief Resets the enumerator to its initial state (before the first element or at the specified `fromDepth`).
			 */
			void reset();

			//---------------------------
			// Iteration
			//---------------------------

			/**
			 * @brief Gets the current item in the enumeration.
			 * @return A `std::pair` containing the current zero-based depth and a `std::reference_wrapper<const GmodNode>`.
			 * @throws std::runtime_error If called when the enumerator is not positioned on a valid element
			 *                            (e.g., before first `next()` or after end is reached), or if a null node pointer is encountered.
			 */
			[[nodiscard]] std::pair<size_t, std::reference_wrapper<const GmodNode>> current() const;

			/**
			 * @brief Advances the enumerator to the next node in the path.
			 * @return True if the enumerator was successfully advanced to a valid node,
			 *         false if the end of the path was reached.
			 */
			bool next();

			//---------------------------
			// Enumerator Inner Class
			//---------------------------

			/**
			 * @class Iterator
			 * @brief Input iterator adapter for the `GmodPath::Enumerator`.
			 * @details Enables the use of `GmodPath::Enumerator` with range-based for loops and standard algorithms.
			 *          Acts as a standard input iterator, caching the current value upon dereference.
			 */
			class Iterator final
			{
			public:
				using iterator_category = std::input_iterator_tag;
				using value_type = std::pair<size_t, std::reference_wrapper<const GmodNode>>;
				using difference_type = std::ptrdiff_t;
				using pointer = const value_type*;
				using reference = const value_type&;

				/** @brief Default constructor (creates an end iterator). */
				Iterator() = default;

				/**
				 * @brief Dereference operator. Gets the current element.
				 * @details Calls `current()` on the underlying enumerator and caches the result.
				 * @return Const reference to the cached depth-node pair.
				 * @throws std::runtime_error If the underlying enumerator is invalid or `current()` throws.
				 */
				[[nodiscard]] reference operator*() const;

				/**
				 * @brief Pointer-to-member operator.
				 * @return Const pointer to the cached depth-node pair.
				 * @throws std::runtime_error If the underlying enumerator is invalid or `current()` throws.
				 */
				[[nodiscard]] pointer operator->() const;

				/**
				 * @brief Pre-increment operator. Advances the iterator.
				 * @details Calls `next()` on the underlying enumerator and clears the cache.
				 * @return Reference to this iterator after advancing.
				 */
				Iterator& operator++();

				/**
				 * @brief Post-increment operator. Advances the iterator.
				 * @details Creates a copy, calls `next()` on the underlying enumerator, clears the cache, and returns the copy.
				 * @return A copy of the iterator before advancing.
				 */
				Iterator operator++( int );

				/**
				 * @brief Equality comparison.
				 * @param other The iterator to compare with.
				 * @return True if both iterators are end iterators or if they point to the same underlying enumerator and have the same internal state.
				 */
				bool operator==( const Iterator& other ) const;

				/**
				 * @brief Inequality comparison.
				 * @param other The iterator to compare with.
				 * @return True if the iterators are different.
				 */
				bool operator!=( const Iterator& other ) const;

			private:
				friend class Enumerator;

				/**
				 * @brief Private constructor used by `Enumerator::begin()` and `Enumerator::end()`.
				 * @param enumerator Pointer to the `Enumerator` instance (can be null for end iterator).
				 * @param isEnd Flag indicating if this is an end iterator.
				 */
				Iterator( Enumerator* enumerator, bool isEnd );

				/** @brief Helper to advance the iterator and handle the end state. */
				void advance();

				/** @brief Helper to update the cached value if needed. */
				void updateCache() const;

				Enumerator* m_enumerator{ nullptr };
				bool m_isEnd{ true };
				mutable std::optional<value_type> m_cachedValue;
			};

			/**
			 * @brief Get an iterator pointing to the beginning of the sequence.
			 * @details Advances the enumerator to the first valid element.
			 * @return An `Iterator` positioned at the first node (or `end()` if empty or starting past the end).
			 */
			Iterator begin();

			/**
			 * @brief Get an iterator pointing past the end of the sequence.
			 * @return An end `Iterator`.
			 */
			Iterator end();

		private:
			const GmodPath& m_path;
			int m_currentIndex;
			const size_t m_endIndex;
			const size_t m_startIndex;
		};

		//----------------------------------------------
		// Enumerator Accessors
		//----------------------------------------------

		/**
		 * @brief Get an enumerator to traverse the full path from the beginning (depth 0).
		 * @return An `Enumerator` instance for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		[[nodiscard]] Enumerator fullPath() const;

		/**
		 * @brief Get an enumerator to traverse the path starting from a specific depth.
		 * @param fromDepth The zero-based depth index to start enumeration from.
		 * @return An `Enumerator` instance for this path, starting at the specified depth.
		 * @throws std::out_of_range If `fromDepth` is invalid for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		[[nodiscard]] Enumerator fullPathFrom( size_t fromDepth ) const;

	private:
		//----------------------------------------------
		// Private Static Helper Methods
		//----------------------------------------------

		/**
		 * @brief Internal implementation for parsing a standard path string (e.g., "A/B.L/C").
		 * @details Handles splitting, node/location parsing, node lookup, and path validation/construction.
		 * @param item The path string (as a string_view) to parse.
		 * @param gmod The `Gmod` object to use for resolving nodes.
		 * @param locations The `Locations` object to use for resolving locations.
		 * @return A `GmodParsePathResult` containing either the parsed `GmodPath` (Ok) or an error message (Err).
		 */
		[[nodiscard]] static std::unique_ptr<GmodParsePathResult> parseInternal(
			std::string_view item,
			const Gmod& gmod,
			const Locations& locations );

		/**
		 * @brief Internal implementation for parsing a full path string (e.g., "VE/A/B.L/C").
		 * @details Handles splitting, node/location parsing, node lookup, and path validation/construction,
		 *          expecting the path to start from the root.
		 * @param pathStr The full path string to parse, provided as a `std::string_view`.
		 * @param gmod The `Gmod` object to use for resolving nodes.
		 * @param locations The `Locations` object to use for resolving locations.
		 * @return A `GmodParsePathResult` containing either the parsed `GmodPath` (Ok) or an error message (Err).
		 */
		[[nodiscard]] static std::unique_ptr<GmodParsePathResult> parseFullPathInternal(
			std::string_view pathStr,
			const Gmod& gmod,
			const Locations& locations );

		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief The VIS version associated with this path. */
		VisVersion m_visVersion;

		/** @brief The owned target node at the end of the path. Only valid if `!m_isEmpty`. */
		GmodNode m_ownedTargetNode;

		/** @brief Vector storing owned GmodNode instances for the parent nodes, ordered root towards target.
		 *         These nodes may have locations modified by the LocationSetsVisitor. */
		std::vector<GmodNode> m_ownedParentNodes;

		/** @brief Flag indicating if the path is empty (default constructed). */
		bool m_isEmpty{ true };
	};

	//=====================================================================
	// GmodIndividualizableSet Class
	//=====================================================================

	/**
	 * @class GmodIndividualizableSet
	 * @brief Represents a set of consecutive individualizable nodes within a `GmodPath`.
	 * @details Allows applying a common location to these nodes and retrieving the modified path.
	 *          This class operates on a mutable reference to the original `GmodPath`.
	 *          After `build()` is called, the set becomes invalid.
	 */
	class GmodIndividualizableSet final
	{
	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Construct a new individualizable set for a segment of a `GmodPath`.
		 * @param nodeIndices Vector of zero-based indices identifying the consecutive nodes in the set within the path.
		 * @param path A mutable reference to the `GmodPath` containing these nodes.
		 * @throws std::invalid_argument If indices are empty, invalid, non-consecutive,
		 *                               or if nodes are not individualizable or have inconsistent locations,
		 *                               or if the set doesn't contain a leaf/target node.
		 * @throws std::logic_error If the referenced path is empty.
		 * @throws std::runtime_error If the path contains null pointers at the specified indices.
		 */
		GmodIndividualizableSet( const std::vector<size_t>& nodeIndices, const GmodPath& path );

		/** @brief Default constructor. */
		GmodIndividualizableSet() = delete;

		/** @brief Copy constructor */
		GmodIndividualizableSet( const GmodIndividualizableSet& ) = default;

		/** @brief Move constructor */
		GmodIndividualizableSet( GmodIndividualizableSet&& ) = default;

		/** @brief Destructor */
		~GmodIndividualizableSet() = default;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodIndividualizableSet& operator=( const GmodIndividualizableSet& ) = default;

		/** @brief Move assignment operator */
		GmodIndividualizableSet& operator=( GmodIndividualizableSet&& ) = default;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets const references to the `GmodNode`s included in this set.
		 * @return A vector of `std::reference_wrapper<const GmodNode>` pointing to the nodes.
		 * @throws std::runtime_error If the internal path pointer is null (e.g., after `build()` was called)
		 *                            or if the path contains null pointers at the set's indices.
		 */
		[[nodiscard]] std::vector<std::reference_wrapper<const GmodNode>> nodes() const;

		/**
		 * @brief Get the zero-based indices of the nodes belonging to this set within the original path.
		 * @return Const reference to the vector of node indices.
		 */
		[[nodiscard]] const std::vector<size_t>& nodeIndices() const noexcept;

		/**
		 * @brief Get the common location currently associated with this set (if any).
		 * @details Reflects the location of the first node in the set upon construction or after `setLocation()`.
		 * @return The common location as `std::optional<Location>`. Returns `std::nullopt` if no location is set.
		 * @throws std::runtime_error If the internal path pointer is null or if the path contains a null pointer at the first index.
		 */
		[[nodiscard]] std::optional<Location> location() const;

		//----------------------------------------------
		// Mutators and Operations
		//----------------------------------------------

		void setLocation( std::optional<Location> newLocation );

		/**
		 * @brief Finalizes modifications and returns the modified `GmodPath` via move semantics.
		 * @details After calling `build()`, this set becomes invalid (internal path pointer is nullified)
		 *          and should not be used further. The original `GmodPath` object referenced during
		 *          construction is moved from and becomes invalid.
		 * @return The modified `GmodPath` object (moved).
		 * @throws std::runtime_error If the internal path pointer is already null.
		 */
		[[nodiscard]] GmodPath build();

		//----------------------------------------------
		// Conversion
		//----------------------------------------------

		/**
		 * @brief Get a string representation of the individualizable set (for debugging).
		 * @return A string describing the set, including node indices and current location.
		 */
		[[nodiscard]] std::string toString() const;

	private:
		std::vector<size_t> m_nodeIndices;
		GmodPath m_ownedPath;
	};

	//=====================================================================
	// GmodParsePathResult Class
	//=====================================================================

	/**
	 * @class GmodParsePathResult
	 * @brief Represents the outcome of a GmodPath parsing operation (success or failure).
	 * @details Used as the return type for internal parsing functions (`parseInternal`, `parseFullPathInternal`).
	 *          Provides a type-safe way to handle either a valid `GmodPath` or an error message.
	 *          Uses move semantics for efficiency. Acts as a base class for `Ok` and `Err`.
	 */
	class GmodParsePathResult
	{
	protected:
		/** @brief Protected default constructor to prevent direct instantiation of the base class. */
		GmodParsePathResult() = default;

	public:
		/** @brief Deleted copy constructor - results are move-only. */
		GmodParsePathResult( const GmodParsePathResult& ) = delete;

		/** @brief Default move constructor. */
		GmodParsePathResult( GmodParsePathResult&& ) = default;

		/** @brief Deleted copy assignment operator. */
		GmodParsePathResult& operator=( const GmodParsePathResult& ) = delete;

		/** @brief Default move assignment operator. */
		GmodParsePathResult& operator=( GmodParsePathResult&& ) = default;

		/** @brief Destructor */
		virtual ~GmodParsePathResult() = default;

		class Ok;
		class Err;
	};

	/**
	 * @class GmodParsePathResult::Ok
	 * @brief Represents a successful path parsing result. Contains the parsed `GmodPath`.
	 */
	class GmodParsePathResult::Ok final : public GmodParsePathResult
	{
	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Construct a successful result by moving the path.
		 * @param p The parsed path to move into the result.
		 */
		explicit Ok( GmodPath&& path );

		Ok( const GmodPath& path ) = delete;

		/** @brief Deleted copy constructor. */
		Ok( const Ok& ) = delete;

		/** @brief Default move constructor. */
		Ok( Ok&& other ) noexcept;

		virtual ~Ok() = default;

		//----------------------------------------------
		// Speclial Member Functions
		//----------------------------------------------

		/** @brief Deleted copy assignment. */
		Ok& operator=( const Ok& ) = delete;

		/** @brief Default move assignment. */
		Ok& operator=( Ok&& other ) noexcept = default;

		GmodPath& path();

	private:
		/** @brief The successfully parsed path (owned). */
		GmodPath m_path;
	};

	/**
	 * @class GmodParsePathResult::Err
	 * @brief Represents a failed path parsing result. Contains an error message.
	 */
	class GmodParsePathResult::Err final : public GmodParsePathResult
	{
	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Construct an error result.
		 * @param e The error message string.
		 */
		explicit Err( const std::string& errorMessage );

		/** @brief Deleted copy constructor. */
		Err( const Err& ) = delete;

		/** @brief Default move constructor. */
		Err( Err&& other ) noexcept;

		virtual ~Err() = default;

		//----------------------------------------------
		// Speclial Member Functions
		//----------------------------------------------

		/** @brief Deleted copy assignment. */
		Err& operator=( const Err& ) = delete;

		/** @brief Default move assignment. */
		Err& operator=( Err&& other ) noexcept = default;

		const std::string& error();

	private:
		/** @brief Error message describing the parsing failure. */
		std::string m_error;
	};
}
