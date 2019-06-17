Polynomial
========

.. image:: logo.svg
    :width: 120px
    :alt: Polynomial logo
    :align: center

Polynomial is an object-oriented, high-level language for implementing smart
contracts. Smart contracts are programs which govern the behaviour of accounts
within the Sophon state.

Polynomial was influenced by C++, Python and JavaScript and is designed to target
the Sophon Virtual Machine (SVM).

Polynomial is statically typed, supports inheritance, libraries and complex
user-defined types among other features.

With Polynomial you can create contracts for uses such as voting, crowdfunding, blind auctions,
and multi-signature wallets.

When deploying contracts, you should use the latest released version of Polynomial. This is because breaking changes as well as new features and bug fixes are introduced regularly. We currently use a 0.x version number `to indicate this fast pace of change <https://semver.org/#spec-item-4>`_.

Language Documentation
----------------------

If you are new to the concept of smart contracts we recommend you start with
:ref:`an example smart contract <simple-smart-contract>` written
in Polynomial. When you are ready for more detail, we recommend you read the
:doc:`"Polynomial by Example" <polynomial-by-example>` and :doc:`"Polynomial in Depth" <polynomial-in-depth>` sections to learn the core concepts of the language.

For further reading, try :ref:`the basics of blockchains <blockchain-basics>`
and details of the :ref:`Sophon Virtual Machine <the-sophon-virtual-machine>`.

.. hint::
  You can always try out code examples in your browser with the
  `Fourier IDE <https://fourier.superstring.io>`_. Fourier is a web browser based IDE
  that allows you to write Polynomial smart contracts, then deploy and run the
  smart contracts. It can take a while to load, so please be patient.

.. warning::
    As humans write software, it can have bugs. You should follow established
    software development best-practices when writing your smart contracts, this
    includes code review, testing, audits, and correctness proofs. Smart contract
    users are sometimes more confident with code than their authors, and
    blockchains and smart contracts have their own unique issues to
    watch out for, so before working on production code, make sure you read the
    :ref:`security_considerations` section.

If you have any questions, you can try searching for answers or asking on the
`Sophon Stackexchange <https://sophon.stackexchange.com/>`_, or our `gitter channel <https://gitter.im/susy-lang/polynomial/>`_.

Ideas for improving Polynomial or this documentation are always welcome, read our :doc:`contributors guide <contributing>` for more details.

.. _translations:

Translations
------------

Community volunteers help translate this documentation into several languages.
They have varying degrees of completeness and up-to-dateness. The English
version stands as a reference.

* `Simplified Chinese <http://polynomial-cn.readthedocs.io>`_ (in progress)
* `Spanish <https://polynomial-es.readthedocs.io>`_
* `Russian <https://octonion.institute/susy-go/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Polynomial>`_ (rather outdated)
* `Korean <http://polynomial-kr.readthedocs.io>`_ (in progress)
* `French <http://polynomial-fr.readthedocs.io>`_ (in progress)

Contents
========

:ref:`Keyword Index <genindex>`, :ref:`Search Page <search>`

.. toctree::
   :maxdepth: 2

   introduction-to-smart-contracts.rst
   installing-polynomial.rst
   polynomial-by-example.rst
   polynomial-in-depth.rst
   security-considerations.rst
   resources.rst
   using-the-compiler.rst
   metadata.rst
   abi-spec.rst
   yul.rst
   style-guide.rst
   common-patterns.rst
   bugs.rst
   contributing.rst
   lll.rst
