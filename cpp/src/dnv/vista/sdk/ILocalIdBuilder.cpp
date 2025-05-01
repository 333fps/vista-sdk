/**
 * @file ILocalIdBuilder.cpp
 * @brief Template instantiation for ILocalIdBuilder.
 */

#include "pch.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/ILocalIdBuilder.hpp"

namespace dnv::vista::sdk
{
	template class ILocalIdBuilder<LocalIdBuilder, LocalId>;
}
