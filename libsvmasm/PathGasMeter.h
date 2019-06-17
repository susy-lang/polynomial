/*
	This file is part of polynomial.

	polynomial is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	polynomial is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with polynomial.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file PathGasMeter.cpp
 * @author Christian <c@sofdev.com>
 * @date 2015
 */

#pragma once

#include <libsvmasm/GasMeter.h>

#include <libpolynomial/interface/SVMVersion.h>

#include <set>
#include <vector>
#include <memory>

namespace dev
{
namespace sof
{

class KnownState;

struct GasPath
{
	size_t index = 0;
	std::shared_ptr<KnownState> state;
	u256 largestMemoryAccess;
	GasMeter::GasConsumption gas;
	std::set<size_t> visitedJumpdests;
};

/**
 * Computes an upper bound on the gas usage of a computation starting at a certain position in
 * a list of AssemblyItems in a given state until the computation stops.
 * Can be used to estimate the gas usage of functions on any given input.
 */
class PathGasMeter
{
public:
	explicit PathGasMeter(AssemblyItems const& _items, polynomial::SVMVersion _svmVersion);

	GasMeter::GasConsumption estimateMax(size_t _startIndex, std::shared_ptr<KnownState> const& _state);

private:
	GasMeter::GasConsumption handleQueueItem();

	std::vector<std::unique_ptr<GasPath>> m_queue;
	std::map<u256, size_t> m_tagPositions;
	AssemblyItems const& m_items;
	polynomial::SVMVersion m_svmVersion;
};

}
}
