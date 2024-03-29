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
/** @file GasMeter.cpp
 * @author Christian <c@sofdev.com>
 * @date 2015
 */

#pragma once

#include <libsvmasm/ExpressionClasses.h>
#include <libsvmasm/AssemblyItem.h>

#include <liblangutil/SVMVersion.h>

#include <ostream>
#include <tuple>

namespace dev
{
namespace sof
{

class KnownState;

namespace GasCosts
{
	static unsigned const stackLimit = 1024;
	static unsigned const tier0Gas = 0;
	static unsigned const tier1Gas = 2;
	static unsigned const tier2Gas = 3;
	static unsigned const tier3Gas = 5;
	static unsigned const tier4Gas = 8;
	static unsigned const tier5Gas = 10;
	static unsigned const tier6Gas = 20;
	static unsigned const tier7Gas = 0;
	inline unsigned extCodeGas(langutil::SVMVersion _svmVersion)
	{
		return _svmVersion >= langutil::SVMVersion::tangerineWhistle() ? 700 : 20;
	}
	inline unsigned balanceGas(langutil::SVMVersion _svmVersion)
	{
		return _svmVersion >= langutil::SVMVersion::tangerineWhistle() ? 400 : 20;
	}
	static unsigned const expGas = 10;
	inline unsigned expByteGas(langutil::SVMVersion _svmVersion)
	{
		return _svmVersion >= langutil::SVMVersion::spuriousDragon() ? 50 : 10;
	}
	static unsigned const keccak256Gas = 30;
	static unsigned const keccak256WordGas = 6;
	inline unsigned sloadGas(langutil::SVMVersion _svmVersion)
	{
		return _svmVersion >= langutil::SVMVersion::tangerineWhistle() ? 200 : 50;
	}
	static unsigned const sstoreSetGas = 20000;
	static unsigned const sstoreResetGas = 5000;
	static unsigned const sstoreRefundGas = 15000;
	static unsigned const jumpdestGas = 1;
	static unsigned const logGas = 375;
	static unsigned const logDataGas = 8;
	static unsigned const logTopicGas = 375;
	static unsigned const createGas = 32000;
	inline unsigned callGas(langutil::SVMVersion _svmVersion)
	{
		return _svmVersion >= langutil::SVMVersion::tangerineWhistle() ? 700 : 40;
	}
	static unsigned const callStipend = 2300;
	static unsigned const callValueTransferGas = 9000;
	static unsigned const callNewAccountGas = 25000;
	inline unsigned selfdestructGas(langutil::SVMVersion _svmVersion)
	{
		return _svmVersion >= langutil::SVMVersion::tangerineWhistle() ? 5000 : 0;
	}
	static unsigned const selfdestructRefundGas = 24000;
	static unsigned const memoryGas = 3;
	static unsigned const quadCoeffDiv = 512;
	static unsigned const createDataGas = 200;
	static unsigned const txGas = 21000;
	static unsigned const txCreateGas = 53000;
	static unsigned const txDataZeroGas = 4;
	static unsigned const txDataNonZeroGas = 68;
	static unsigned const copyGas = 3;
}

/**
 * Class that helps computing the maximum gas consumption for instructions.
 * Has to be initialized with a certain known state that will be automatically updated for
 * each call to estimateMax. These calls have to supply strictly subsequent AssemblyItems.
 * A new gas meter has to be constructed (with a new state) for control flow changes.
 */
class GasMeter
{
public:
	struct GasConsumption
	{
		GasConsumption(unsigned _value = 0, bool _infinite = false): value(_value), isInfinite(_infinite) {}
		GasConsumption(u256 _value, bool _infinite = false): value(_value), isInfinite(_infinite) {}
		static GasConsumption infinite() { return GasConsumption(0, true); }

		GasConsumption& operator+=(GasConsumption const& _other);
		bool operator<(GasConsumption const& _other) const
		{
			return std::make_pair(isInfinite, value) < std::make_pair(_other.isInfinite, _other.value);
		}

		u256 value;
		bool isInfinite;
	};

	/// Constructs a new gas meter given the current state.
	GasMeter(std::shared_ptr<KnownState> const& _state, langutil::SVMVersion _svmVersion, u256 const& _largestMemoryAccess = 0):
		m_state(_state), m_svmVersion(_svmVersion), m_largestMemoryAccess(_largestMemoryAccess) {}

	/// @returns an upper bound on the gas consumed by the given instruction and updates
	/// the state.
	/// @param _inculdeExternalCosts if true, include costs caused by other contracts in calls.
	GasConsumption estimateMax(AssemblyItem const& _item, bool _includeExternalCosts = true);

	u256 const& largestMemoryAccess() const { return m_largestMemoryAccess; }

	/// @returns gas costs for simple instructions with constant gas costs (that do not
	/// change with SVM versions)
	static unsigned runGas(Instruction _instruction);

	/// @returns the gas cost of the supplied data, depending whether it is in creation code, or not.
	/// In case of @a _inCreation, the data is only sent as a transaction and is not stored, whereas
	/// otherwise code will be stored and have to pay "createDataGas" cost.
	static u256 dataGas(bytes const& _data, bool _inCreation);

private:
	/// @returns _multiplier * (_value + 31) / 32, if _value is a known constant and infinite otherwise.
	GasConsumption wordGas(u256 const& _multiplier, ExpressionClasses::Id _value);
	/// @returns the gas needed to access the given memory position.
	/// @todo this assumes that memory was never accessed before and thus over-estimates gas usage.
	GasConsumption memoryGas(ExpressionClasses::Id _position);
	/// @returns the memory gas for accessing the memory at a specific offset for a number of bytes
	/// given as values on the stack at the given relative positions.
	GasConsumption memoryGas(int _stackPosOffset, int _stackPosSize);

	std::shared_ptr<KnownState> m_state;
	langutil::SVMVersion m_svmVersion;
	/// Largest point where memory was accessed since the creation of this object.
	u256 m_largestMemoryAccess;
};

inline std::ostream& operator<<(std::ostream& _str, GasMeter::GasConsumption const& _consumption)
{
	if (_consumption.isInfinite)
		return _str << "[???]";
	else
		return _str << std::dec << _consumption.value;
}


}
}
