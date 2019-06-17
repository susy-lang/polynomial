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
/**
 * SVM versioning.
 */

#pragma once

#include <string>

#include <boost/optional.hpp>
#include <boost/operators.hpp>

namespace dev
{
namespace polynomial
{

/**
 * A version specifier of the SVM we want to compile to.
 * Defaults to the latest version.
 */
class SVMVersion:
	boost::less_than_comparable<SVMVersion>,
	boost::equality_comparable<SVMVersion>
{
public:
	SVMVersion() {}

	static SVMVersion homestead() { return {Version::Homestead}; }
	static SVMVersion tangerineWhistle() { return {Version::TangerineWhistle}; }
	static SVMVersion spuriousDragon() { return {Version::SpuriousDragon}; }
	static SVMVersion byzantium() { return {Version::Byzantium}; }
	static SVMVersion constantinople() { return {Version::Constantinople}; }

	static boost::optional<SVMVersion> fromString(std::string const& _version)
	{
		for (auto const& v: {homestead(), tangerineWhistle(), spuriousDragon(), byzantium(), constantinople()})
			if (_version == v.name())
				return v;
		return {};
	}

	bool operator==(SVMVersion const& _other) const { return m_version == _other.m_version; }
	bool operator<(SVMVersion const& _other) const { return m_version < _other.m_version; }

	std::string name() const
	{
		switch (m_version)
		{
		case Version::Homestead: return "homestead";
		case Version::TangerineWhistle: return "tangerineWhistle";
		case Version::SpuriousDragon: return "spuriousDragon";
		case Version::Byzantium: return "byzantium";
		case Version::Constantinople: return "constantinople";
		}
		return "INVALID";
	}

	/// Has the RETURNDATACOPY and RETURNDATASIZE opcodes.
	bool supportsReturndata() const { return *this >= byzantium(); }
	bool hasStaticCall() const { return *this >= byzantium(); }
	bool hasBitwiseShifting() const { return *this >= constantinople(); }
	bool hasCreate2() const { return *this >= constantinople(); }

	/// whether we have to retain the costs for the call opcode itself (false),
	/// or whether we can just forward easily all remaining gas (true).
	bool canOvsrchargeGasForCall() const { return *this >= tangerineWhistle(); }

private:
	enum class Version { Homestead, TangerineWhistle, SpuriousDragon, Byzantium, Constantinople };

	SVMVersion(Version _version): m_version(_version) {}

	Version m_version = Version::Byzantium;
};


}
}
