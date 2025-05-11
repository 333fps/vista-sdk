#pragma once

#include "GmodNode.h"
#include "Locations.h"
#include "VisVersion.h"

namespace dnv::vista::sdk
{
	class Gmod;
}

namespace dnv::vista::sdk
{
	class GmodIndividualizableSet;

	class GmodPath final
	{
	public:
		class Enumerator;

	private:
		VisVersion m_visVersion;
		const Gmod& m_gmod;
		std::vector<GmodNode*> m_parents;
		GmodNode* m_node;
		std::vector<GmodNode*> m_ownedNodes;

	public:
		GmodPath( const Gmod& gmod, GmodNode* node, std::vector<GmodNode*> parents = {} );

		GmodPath( const GmodPath& other );
		GmodPath( GmodPath&& other ) noexcept;
		GmodPath& operator=( const GmodPath& other );
		GmodPath& operator=( GmodPath&& other ) noexcept;
		~GmodPath() = default;

		[[nodiscard]] bool operator==( const GmodPath& other ) const noexcept;
		[[nodiscard]] bool operator!=( const GmodPath& other ) const noexcept;

		VisVersion visVersion() const noexcept;
		size_t hashCode() const noexcept;
		[[nodiscard]] const Gmod& gmod() const noexcept;
		[[nodiscard]] GmodNode* node() const noexcept;
		[[nodiscard]] const std::vector<GmodNode*>& parents() const noexcept;
		[[nodiscard]] GmodNode* operator[]( size_t index ) const;
		[[nodiscard]] GmodNode*& operator[]( size_t index );
		[[nodiscard]] size_t length() const noexcept;
		[[nodiscard]] GmodNode* rootNode() const noexcept;
		[[nodiscard]] GmodNode* parentNode() const noexcept;
		[[nodiscard]] std::vector<GmodIndividualizableSet> individualizableNodes() const;

		[[nodiscard]] std::vector<std::pair<size_t, std::string>> commonNames() const;

		[[nodiscard]] std::string fullPathString() const;
		[[nodiscard]] std::vector<std::pair<int, std::reference_wrapper<const GmodNode>>> fullPath() const;
		[[nodiscard]] std::string shortPath() const;
		[[nodiscard]] std::string fullPathWithLocation() const;
		[[nodiscard]] std::string shortPathWithLocation() const;

		[[nodiscard]] std::string toString() const;
		[[nodiscard]] std::string toFullPathString() const;
		void toFullPathString( std::string& builder ) const;

		[[nodiscard]] static GmodPath parse( const Gmod& gmod, std::string_view pathString );
		[[nodiscard]] static bool tryParse( const Gmod& gmod, std::string_view pathString, GmodPath& outPath );
		[[nodiscard]] static GmodPath parseFromFullPath( const Gmod& gmod, std::string_view pathString );
		[[nodiscard]] static bool tryParseFromFullPath( const Gmod& gmod, std::string_view pathString, GmodPath& outPath );
		[[nodiscard]] static bool isValid( const std::vector<const GmodNode*>& parents, const GmodNode& node, size_t& missingLinkAt );
		[[nodiscard]] static bool isValid( const std::vector<const GmodNode*>& parents, const GmodNode& node );

		[[nodiscard]] Enumerator enumerator() const;

		class Enumerator final
		{
		private:
			friend class GmodPath;
			const GmodPath* m_pathInstance;
			size_t m_currentIndex;

			Enumerator( const GmodPath* pathInst );

		public:
			Enumerator() = delete;
			Enumerator( const Enumerator& ) = default;
			Enumerator( Enumerator&& ) noexcept = default;
			Enumerator& operator=( const Enumerator& ) = default;
			Enumerator& operator=( Enumerator&& ) noexcept = default;

			[[nodiscard]] GmodNode* current() const;
			bool next();
			void reset();
		};
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
		[[nodiscard]] std::optional<Location> lLocation() const;
		void setLocation( const std::optional<Location>& location );
		[[nodiscard]] std::string toString() const;
	};
}
