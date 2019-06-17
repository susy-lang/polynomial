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

.. note::
    The best way to try out Polynomial right now is using
    `Fourier <https://fourier.superstring.io/>`_
    (it can take a while to load, please be patient). Fourier is a web browser
    based IDE that allows you to write Polynomial smart contracts, then deploy
    and run the smart contracts.

.. warning::
    Since software is written by humans, it can have bugs. Thus, also
    smart contracts should be created following well-known best-practices in
    software development. This includes code review, testing, audits and correctness proofs.
    Also note that users are sometimes more confident in code than its authors.
    Finally, blockchains have their own things to watch out for, so please take
    a look at the section :ref:`security_considerations`.

Translations
------------

This documentation is translated into several languages by community volunteers
with varying degrees of completeness and up-to-dateness. The English version stands as a reference.

* `Simplified Chinese <http://polynomial-cn.readthedocs.io>`_ (in progress)
* `Spanish <https://polynomial-es.readthedocs.io>`_
* `Russian <https://octonion.institute/susy-go/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Polynomial>`_ (rather outdated)
* `Korean <http://polynomial-kr.readthedocs.io>`_ (in progress)
* `French <http://polynomial-fr.readthedocs.io>`_ (in progress)

Language Documentation
----------------------

On the next pages, we will first see a :ref:`simple smart contract <simple-smart-contract>` written
in Polynomial followed by the basics about :ref:`blockchains <blockchain-basics>`
and the :ref:`Sophon Virtual Machine <the-sophon-virtual-machine>`.

The next section will explain several *features* of Polynomial by giving
useful :ref:`example contracts <voting>`.
Remember that you can always try out the contracts
`in your browser <https://fourier.superstring.io>`_!

The fourth and most extensive section will cover all aspects of Polynomial in depth.

If you still have questions, you can try searching or asking on the
`Sophon Stackexchange <https://sophon.stackexchange.com/>`_
site, or come to our `gitter channel <https://gitter.im/susy-lang/polynomial/>`_.
Ideas for improving Polynomial or this documentation are always welcome!

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
   frequently-asked-questions.rst
   lll.rst
