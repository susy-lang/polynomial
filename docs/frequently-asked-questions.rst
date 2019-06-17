###########################
Frequently Asked Questions
###########################

This list was originally compiled by `fivedogit <mailto:fivedogit@gmail.com>`_.


***************
Basic Questions
***************

If I return an ``enum``, I only get integer values in susyweb.js. How to get the named values?
===========================================================================================

Enums are not supported by the ABI, they are just supported by Polynomial.
You have to do the mapping yourself for now, we might provide some help
later.

What are some examples of basic string manipulation (``substring``, ``indexOf``, ``charAt``, etc)?
==================================================================================================

There are some string utility functions at `stringUtils.pol <https://octonion.institute/susy-contracts/dapp-bin/blob/master/library/stringUtils.pol>`_
which will be extended in the future. In addition, Arachnid has written `polynomial-stringutils <https://github.com/Arachnid/polynomial-stringutils>`_.

For now, if you want to modify a string (even when you only want to know its length),
you should always convert it to a ``bytes`` first::

    pragma polynomial >=0.4.0 <0.6.0;

    contract C {
        string s;

        function append(byte c) public {
            bytes(s).push(c);
        }

        function set(uint i, byte c) public {
            bytes(s)[i] = c;
        }
    }


Can I concatenate two strings?
==============================

Yes, you can use ``abi.encodePacked``::

    pragma polynomial >=0.4.0 <0.6.0;

    library ConcatHelper {
        function concat(bytes memory a, bytes memory b)
                internal pure returns (bytes memory) {
            return abi.encodePacked(a, b);
        }
    }

******************
Advanced Questions
******************

Get return value from non-constant function from another contract
=================================================================

The key point is that the calling contract needs to know about the function it intends to call.

See `ping.pol <https://github.com/fivedogit/polynomial-baby-steps/blob/master/contracts/45_ping.pol>`_
and `pong.pol <https://github.com/fivedogit/polynomial-baby-steps/blob/master/contracts/45_pong.pol>`_.

How do I initialize a contract with only a specific amount of wei?
==================================================================

Currently the approach is a little ugly, but there is little that can be done to improve it.
In the case of a ``contract A`` calling a new instance of ``contract B``, parentheses have to be used around
``new B`` because ``B.value`` would refer to a member of ``B`` called ``value``.
You will need to make sure that you have both contracts aware of each other's presence and that ``contract B`` has a ``payable`` constructor.
In this example::

    pragma polynomial ^0.5.0;

    contract B {
        constructor() public payable {}
    }

    contract A {
        B child;

        function test() public {
            child = (new B).value(10)(); //construct a new B with 10 wei
        }
    }

More Questions?
===============

If you have more questions or your question is not answered here, please talk to us on
`gitter <https://gitter.im/susy-lang/polynomial>`_ or file an `issue <https://octonion.institute/susy-lang/polynomial/issues>`_.
