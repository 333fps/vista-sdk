#pragma once

#include "GmodNode.h"
#include "Locations.h"
#include "VIS.h"
#include "GmodTraversal.h"

namespace dnv::vista::sdk
{
	class Gmod;
	class GmodParsePathResult;
	class GmodIndividualizableSet;

	namespace internal
	{
		struct ParseContext;

		dnv::vista::sdk::TraversalHandlerResult parseInternalTraversalHandler(
			ParseContext& context,
			const std::vector<const GmodNode*>& traversedParents,
			const GmodNode& currentNode );
	}

	class GmodPath final
	{
		friend class GmodIndividualizableSet;

		friend dnv::vista::sdk::TraversalHandlerResult internal::parseInternalTraversalHandler(
			internal::ParseContext&,
			const std::vector<const GmodNode*>&,
			const GmodNode& );

	public:
		class Enumerator;

	public:
		GmodPath( const Gmod& gmod, GmodNode* node, std::vector<GmodNode*> parents = {} );

		GmodPath();
		GmodPath( const GmodPath& other );
		GmodPath( GmodPath&& other ) noexcept;
		GmodPath& operator=( const GmodPath& other );
		GmodPath& operator=( GmodPath&& other ) noexcept;
		~GmodPath();

		GmodPath build();

		[[nodiscard]] bool operator==( const GmodPath& other ) const noexcept;
		[[nodiscard]] bool operator!=( const GmodPath& other ) const noexcept;

		[[nodiscard]] GmodNode* operator[]( size_t index ) const;
		[[nodiscard]] GmodNode*& operator[]( size_t index );

		[[nodiscard]] static bool isValid( const std::vector<GmodNode*>& parents, const GmodNode& node );
		[[nodiscard]] static bool isValid( const std::vector<GmodNode*>& parents, const GmodNode& node, size_t& missingLinkAt );
		[[nodiscard]] bool isMappable();
		[[nodiscard]] bool isIndividualizable();

		VisVersion visVersion() const noexcept;
		size_t hashCode() const noexcept;

		[[nodiscard]] const Gmod* gmod() const noexcept;
		[[nodiscard]] GmodNode* node() const noexcept;
		[[nodiscard]] const std::vector<GmodNode*>& parents() const noexcept;

		[[nodiscard]] size_t length() const noexcept;
		[[nodiscard]] GmodNode* rootNode() const noexcept;
		[[nodiscard]] GmodNode* parentNode() const noexcept;
		[[nodiscard]] std::vector<GmodIndividualizableSet> individualizableNodes() const;

		[[nodiscard]] std::optional<std::string> normalAssignmentName( size_t nodeDepth ) const;
		[[nodiscard]] std::vector<std::pair<size_t, std::string>> commonNames() const;

		[[nodiscard]] std::string toString() const;
		void toString( std::stringstream& builder, char separator = '/' ) const;

		[[nodiscard]] std::string toStringDump() const;
		void toStringDump( std::stringstream& builder ) const;

		[[nodiscard]] std::string toFullPathString() const;
		void toFullPathString( std::stringstream& builder ) const;

		[[nodiscard]] GmodPath withoutLocations() const;

		[[nodiscard]] static GmodPath parse( std::string_view pathString, VisVersion visVersion );
		[[nodiscard]] static bool tryParse( std::string_view pathString, VisVersion visVersion, std::optional<GmodPath>& outPath );

		[[nodiscard]] static GmodPath parse( std::string_view pathString, const Gmod& gmod, const Locations& locations );
		[[nodiscard]] static bool tryParse( std::string_view pathString, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath );

		[[nodiscard]] static GmodPath parseFullPath( std::string_view pathString, VisVersion visVersion );
		[[nodiscard]] static bool tryParseFullPath( std::string_view pathString, VisVersion visVersion, std::optional<GmodPath>& outPath );
		[[nodiscard]] static bool tryParseFullPath( std::string_view pathString, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath );

		[[nodiscard]] Enumerator fullPath() const;
		[[nodiscard]] Enumerator fullPathFrom( size_t fromDepth ) const;

		class Enumerator final
		{
		private:
			friend class GmodPath;
			const GmodPath* m_pathInstance;
			size_t m_currentIndex;

			Enumerator( const GmodPath* pathInst, size_t startIndex = std::numeric_limits<size_t>::max() );

		public:
			using iterator_category = std::input_iterator_tag;
			using value_type = std::pair<size_t, GmodNode*>;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type*;
			using reference = const value_type&;

			bool operator!=( const Enumerator& other ) const;
			bool operator==( const Enumerator& other ) const;

			value_type operator*() const;
			Enumerator& operator++();

			Enumerator() = delete;
			Enumerator( const Enumerator& );
			Enumerator( Enumerator&& ) noexcept = default;
			Enumerator& operator=( const Enumerator& ) = default;
			Enumerator& operator=( Enumerator&& ) noexcept = default;

			Enumerator& begin();
			Enumerator end() const;

			[[nodiscard]] GmodNode* current() const;
			bool next();
			void reset();
		};

	private:
		VisVersion m_visVersion;
		const Gmod* m_gmod;
		GmodNode* m_node;
		std::vector<GmodNode*> m_parents;
		std::vector<GmodNode*> m_ownedNodes;

	private:
		static std::unique_ptr<GmodParsePathResult> parseInternal(
			std::string_view item, const Gmod& gmod, const Locations& locations );

		static std::unique_ptr<GmodParsePathResult> parseFullPathInternal(
			std::string_view item, const Gmod& gmod, const Locations& locations );
	};

	class GmodIndividualizableSet final
	{
	private:
		std::vector<int> m_nodeIndices;
		GmodPath m_path;

	public:
		GmodIndividualizableSet( const std::vector<int>& nodeIndices, const GmodPath& sourcePath );

		[[nodiscard]] std::vector<GmodNode*> nodes() const;
		[[nodiscard]] const std::vector<int>& nodeIndices() const noexcept;
		[[nodiscard]] std::optional<Location> location() const;
		void setLocation( const std::optional<Location>& location );
		[[nodiscard]] std::string toString() const;
	};

	class GmodParsePathResult
	{
	protected:
		GmodParsePathResult() = default;

		GmodParsePathResult( const GmodParsePathResult& ) = delete;
		GmodParsePathResult& operator=( const GmodParsePathResult& ) = delete;
		GmodParsePathResult( GmodParsePathResult&& ) = delete;
		GmodParsePathResult& operator=( GmodParsePathResult&& ) = delete;

	public:
		virtual ~GmodParsePathResult() = default;

		class Ok;
		class Err;
	};

	class GmodParsePathResult::Ok : public GmodParsePathResult
	{
	public:
		GmodPath m_path;

		explicit Ok( GmodPath path );

		Ok( const Ok& ) = delete;
		Ok( Ok&& ) noexcept = delete;
		Ok& operator=( const Ok& ) = delete;
		Ok& operator=( Ok&& ) noexcept = delete;
	};

	class GmodParsePathResult::Err : public GmodParsePathResult
	{
	public:
		std::string error;

		explicit Err( std::string errorString );

		Err( const Err& ) = delete;
		Err( Err&& ) noexcept = delete;
		Err& operator=( const Err& ) = delete;
		Err& operator=( Err&& ) noexcept = delete;
	};
}
