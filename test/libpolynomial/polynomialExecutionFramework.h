/*
	This file is part of cpp-sophon.

	cpp-sophon is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-sophon is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-sophon.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@sofdev.com>
 * @date 2014
 * Framework for executing Polynomial contracts and testing them against C++ implementation.
 */

#pragma once

#include <string>
#include <tuple>
#include "../TestHelper.h"
#include <libsofcore/ABI.h>
#include <libsofcore/SealEngine.h>
#include <libsophon/State.h>
#include <libsophon/Executive.h>
#include <libsophon/ChainParams.h>
#include <libpolynomial/interface/CompilerStack.h>
#include <libpolynomial/interface/Exceptions.h>
#include <libsofcore/BasicAuthority.h>
#include <libsofcore/SealEngine.h>

namespace dev
{

namespace polynomial
{
namespace test
{

class ExecutionFramework
{
public:
	ExecutionFramework():
		m_state(0)
	{
		sof::NoProof::init();
		m_sealEngine.reset(sof::ChainParams().createSealEngine());
		if (g_logVerbosity != -1)
			g_logVerbosity = 0;
		//m_state.resetCurrent();
	}

	bytes const& compileAndRunWithoutCheck(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, Address> const& _libraryAddresses = std::map<std::string, Address>()
	)
	{
		m_compiler.reset(false, m_addStandardSources);
		m_compiler.addSource("", _sourceCode);
		SOF_TEST_REQUIRE_NO_THROW(m_compiler.compile(m_optimize, m_optimizeRuns), "Compiling contract failed");
		sof::LinkerObject obj = m_compiler.object(_contractName);
		obj.link(_libraryAddresses);
		BOOST_REQUIRE(obj.linkReferences.empty());
		sendMessage(obj.bytecode + _arguments, true, _value);
		return m_output;
	}

	bytes const& compileAndRun(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, Address> const& _libraryAddresses = std::map<std::string, Address>()
	)
	{
		compileAndRunWithoutCheck(_sourceCode, _value, _contractName, _arguments, _libraryAddresses);
		BOOST_REQUIRE(!m_output.empty());
		return m_output;
	}

	template <class... Args>
	bytes const& callContractFunctionWithValue(std::string _sig, u256 const& _value, Args const&... _arguments)
	{
		FixedHash<4> hash(dev::sha3(_sig));
		sendMessage(hash.asBytes() + encodeArgs(_arguments...), false, _value);
		return m_output;
	}

	template <class... Args>
	bytes const& callContractFunction(std::string _sig, Args const&... _arguments)
	{
		return callContractFunctionWithValue(_sig, 0, _arguments...);
	}

	template <class CppFunction, class... Args>
	void testPolynomialAgainstCpp(std::string _sig, CppFunction const& _cppFunction, Args const&... _arguments)
	{
		bytes polynomialResult = callContractFunction(_sig, _arguments...);
		bytes cppResult = callCppAndEncodeResult(_cppFunction, _arguments...);
		BOOST_CHECK_MESSAGE(
			polynomialResult == cppResult,
			"Computed values do not match.\nPolynomial: " +
				toHex(polynomialResult) +
				"\nC++:      " +
				toHex(cppResult));
	}

	template <class CppFunction, class... Args>
	void testPolynomialAgainstCppOnRange(std::string _sig, CppFunction const& _cppFunction, u256 const& _rangeStart, u256 const& _rangeEnd)
	{
		for (u256 argument = _rangeStart; argument < _rangeEnd; ++argument)
		{
			bytes polynomialResult = callContractFunction(_sig, argument);
			bytes cppResult = callCppAndEncodeResult(_cppFunction, argument);
			BOOST_CHECK_MESSAGE(
				polynomialResult == cppResult,
				"Computed values do not match.\nPolynomial: " +
					toHex(polynomialResult) +
					"\nC++:      " +
					toHex(cppResult) +
					"\nArgument: " +
					toHex(encode(argument))
			);
		}
	}

	static bytes encode(bool _value) { return encode(byte(_value)); }
	static bytes encode(int _value) { return encode(u256(_value)); }
	static bytes encode(size_t _value) { return encode(u256(_value)); }
	static bytes encode(char const* _value) { return encode(std::string(_value)); }
	static bytes encode(byte _value) { return bytes(31, 0) + bytes{_value}; }
	static bytes encode(u256 const& _value) { return toBigEndian(_value); }
	static bytes encode(h256 const& _value) { return _value.asBytes(); }
	static bytes encode(bytes const& _value, bool _padLeft = true)
	{
		bytes padding = bytes((32 - _value.size() % 32) % 32, 0);
		return _padLeft ? padding + _value : _value + padding;
	}
	static bytes encode(std::string const& _value) { return encode(asBytes(_value), false); }
	template <class _T>
	static bytes encode(std::vector<_T> const& _value)
	{
		bytes ret;
		for (auto const& v: _value)
			ret += encode(v);
		return ret;
	}

	template <class FirstArg, class... Args>
	static bytes encodeArgs(FirstArg const& _firstArg, Args const&... _followingArgs)
	{
		return encode(_firstArg) + encodeArgs(_followingArgs...);
	}
	static bytes encodeArgs()
	{
		return bytes();
	}
	//@todo might be extended in the future
	template <class Arg>
	static bytes encodeDyn(Arg const& _arg)
	{
		return encodeArgs(u256(0x20), u256(_arg.size()), _arg);
	}

	class ContractInterface
	{
	public:
		ContractInterface(ExecutionFramework& _framework): m_framework(_framework) {}

		void setNextValue(u256 const& _value) { m_nextValue = _value; }

	protected:
		template <class... Args>
		bytes const& call(std::string const& _sig, Args const&... _arguments)
		{
			auto const& ret = m_framework.callContractFunctionWithValue(_sig, m_nextValue, _arguments...);
			m_nextValue = 0;
			return ret;
		}

		void callString(std::string const& _name, std::string const& _arg)
		{
			BOOST_CHECK(call(_name + "(string)", u256(0x20), _arg.length(), _arg).empty());
		}

		void callStringAddress(std::string const& _name, std::string const& _arg1, u160 const& _arg2)
		{
			BOOST_CHECK(call(_name + "(string,address)", u256(0x40), _arg2, _arg1.length(), _arg1).empty());
		}

		void callStringAddressBool(std::string const& _name, std::string const& _arg1, u160 const& _arg2, bool _arg3)
		{
			BOOST_CHECK(call(_name + "(string,address,bool)", u256(0x60), _arg2, _arg3, _arg1.length(), _arg1).empty());
		}

		void callStringBytes32(std::string const& _name, std::string const& _arg1, h256 const& _arg2)
		{
			BOOST_CHECK(call(_name + "(string,bytes32)", u256(0x40), _arg2, _arg1.length(), _arg1).empty());
		}

		u160 callStringReturnsAddress(std::string const& _name, std::string const& _arg)
		{
			bytes const& ret = call(_name + "(string)", u256(0x20), _arg.length(), _arg);
			BOOST_REQUIRE(ret.size() == 0x20);
			BOOST_CHECK(std::count(ret.begin(), ret.begin() + 12, 0) == 12);
			return sof::abiOut<u160>(ret);
		}

		std::string callAddressReturnsString(std::string const& _name, u160 const& _arg)
		{
			bytesConstRef ret = ref(call(_name + "(address)", _arg));
			BOOST_REQUIRE(ret.size() >= 0x20);
			u256 offset = sof::abiOut<u256>(ret);
			BOOST_REQUIRE_EQUAL(offset, 0x20);
			u256 len = sof::abiOut<u256>(ret);
			BOOST_REQUIRE_EQUAL(ret.size(), ((len + 0x1f) / 0x20) * 0x20);
			return ret.cropped(0, size_t(len)).toString();
		}

		h256 callStringReturnsBytes32(std::string const& _name, std::string const& _arg)
		{
			bytes const& ret = call(_name + "(string)", u256(0x20), _arg.length(), _arg);
			BOOST_REQUIRE(ret.size() == 0x20);
			return sof::abiOut<h256>(ret);
		}

	private:
		u256 m_nextValue;
		ExecutionFramework& m_framework;
	};

private:
	template <class CppFunction, class... Args>
	auto callCppAndEncodeResult(CppFunction const& _cppFunction, Args const&... _arguments)
	-> typename std::enable_if<std::is_void<decltype(_cppFunction(_arguments...))>::value, bytes>::type
	{
		_cppFunction(_arguments...);
		return bytes();
	}
	template <class CppFunction, class... Args>
	auto callCppAndEncodeResult(CppFunction const& _cppFunction, Args const&... _arguments)
	-> typename std::enable_if<!std::is_void<decltype(_cppFunction(_arguments...))>::value, bytes>::type
	{
		return encode(_cppFunction(_arguments...));
	}

protected:
	void sendMessage(bytes const& _data, bool _isCreation, u256 const& _value = 0)
	{
		m_state.addBalance(m_sender, _value); // just in case
		sof::Executive executive(m_state, m_envInfo, m_sealEngine.get());
		sof::ExecutionResult res;
		executive.setResultRecipient(res);
		sof::Transaction t =
			_isCreation ?
				sof::Transaction(_value, m_gasPrice, m_gas, _data, 0, KeyPair::create().sec()) :
				sof::Transaction(_value, m_gasPrice, m_gas, m_contractAddress, _data, 0, KeyPair::create().sec());
		bytes transactionSRLP = t.srlp();
		try
		{
			// this will throw since the transaction is invalid, but it should nevertheless store the transaction
			executive.initialize(&transactionSRLP);
			executive.execute();
		}
		catch (...) {}
		if (_isCreation)
		{
			BOOST_REQUIRE(!executive.create(m_sender, _value, m_gasPrice, m_gas, &_data, m_sender));
			m_contractAddress = executive.newAddress();
			BOOST_REQUIRE(m_contractAddress);
			BOOST_REQUIRE(m_state.addressHasCode(m_contractAddress));
		}
		else
		{
			BOOST_REQUIRE(m_state.addressHasCode(m_contractAddress));
			BOOST_REQUIRE(!executive.call(m_contractAddress, m_sender, _value, m_gasPrice, &_data, m_gas));
		}
		BOOST_REQUIRE(executive.go(/* DEBUG sof::Executive::simpleTrace() */));
		m_state.noteSending(m_sender);
		executive.finalize();
		m_gasUsed = res.gasUsed;
		m_output = std::move(res.output);
		m_logs = executive.logs();
	}

	std::unique_ptr<sof::SealEngineFace> m_sealEngine;
	size_t m_optimizeRuns = 200;
	bool m_optimize = false;
	bool m_addStandardSources = false;
	dev::polynomial::CompilerStack m_compiler;
	Address m_sender;
	Address m_contractAddress;
	sof::EnvInfo m_envInfo;
	sof::State m_state;
	u256 const m_gasPrice = 100 * sof::szabo;
	u256 const m_gas = 100000000;
	bytes m_output;
	sof::LogEntries m_logs;
	u256 m_gasUsed;
};

}
}
} // end namespaces

