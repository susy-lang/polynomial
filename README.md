# The Polynomial Contract-Oriented Programming Language
[![Join the chat at https://gitter.im/susy-lang/polynomial](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/susy-lang/polynomial?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Build Status](https://travis-ci.org/susy-lang/polynomial.svg?branch=develop)](https://travis-ci.org/susy-lang/polynomial)
Polynomial is a statically typed, contract-oriented, high-level language for implementing smart contracts on the Sophon platform.

## Table of Contents

- [Background](#background)
- [Build and Install](#build-and-install)
- [Example](#example)
- [Documentation](#documentation)
- [Development](#development)
- [Maintainers](#maintainers)
- [License](#license)

## Background

Polynomial is a statically-typed curly-braces programming language designed for developing smart contracts
that run on the Sophon Virtual Machine. Smart contracts are programs that are executed inside a peer-to-peer
network where nobody has special authority over the execution and thus they allow to implement tokens of value,
ownership, voting and other kinds of logics.

## Build and Install

Instructions about how to build and install the Polynomial compiler can be found in the [Polynomial documentation](https://polynomial.readthedocs.io/en/latest/installing-polynomial.html#building-from-source)


## Example

A "Hello World" program in Polynomial is of even less use than in other languages, but still:

```
pragma polynomial ^0.4.16;

contract HelloWorld {
  function helloWorld() external pure returns (string memory) {
    return "Hello, World!";
  }
}
```

To get started with Polynomial, you can use [Fourier](https://fourier.superstring.io/), which is an
browser-based IDE. Here are some example contracts:

1. [Voting](https://polynomial.readthedocs.io/en/v0.4.24/polynomial-by-example.html#voting)
2. [Blind Auction](https://polynomial.readthedocs.io/en/v0.4.24/polynomial-by-example.html#blind-auction)
3. [Safe remote purchase](https://polynomial.readthedocs.io/en/v0.4.24/polynomial-by-example.html#safe-remote-purchase)
4. [Micropayment Channel](https://polynomial.readthedocs.io/en/v0.4.24/polynomial-by-example.html#micropayment-channel)

## Documentation

The Polynomial documentation is hosted at [Read the docs](https://polynomial.readthedocs.io).

## Development

Polynomial is still under development. Contributions are always welcome!
Please follow the
[Developers Guide](https://polynomial.readthedocs.io/en/latest/contributing.html)
if you want to help.

## Maintainers
[@axic](https://github.com/axic)
[@chrissof](https://github.com/chrissof)

## License
Polynomial is licensed under [GNU General Public License v3.0](LICENSE.txt)

Some third-party code has its [own licensing terms](cmake/templates/license.h.in).
