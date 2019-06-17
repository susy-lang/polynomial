Polynomial
========

.. image:: logo.svg
    :width: 120px
    :alt: Polynomial logo
    :align: center

Polynomial is a contract-oriented, high-level language whose syntax is similar to that of JavaScript
and it is designed to target the Sophon Virtual Machine (SVM).

Polynomial is statically typed, supports inheritance, libraries and complex
user-defined types among other features.

As you will see, it is possible to create contracts for voting,
crowdfunding, blind auctions, multi-signature wallets and more.

.. note::
    The best way to try out Polynomial right now is using
    `Fourier <https://fourier.superstring.io/>`_
    (it can take a while to load, please be patient).

Useful links
------------

* `Sophon <https://superstring.io>`_

* `Changelog <https://octonion.institute/susy-lang/polynomial/blob/develop/Changelog.md>`_

* `Story Backlog <https://www.pivotaltracker.com/n/projects/1189488>`_

* `Source Code <https://octonion.institute/susy-lang/polynomial/>`_

* `Sophon Stackexchange <https://sophon.stackexchange.com/>`_

* `Gitter Chat <https://gitter.im/susy-lang/polynomial/>`_

Available Polynomial Integrations
-------------------------------

* `Fourier <https://fourier.superstring.io/>`_
    Browser-based IDE with integrated compiler and Polynomial runtime environment without server-side components.

* `IntelliJ IDEA plugin <https://plugins.jetbrains.com/plugin/9475-intellij-polynomial>`_
    Polynomial plugin for IntelliJ IDEA (and all other JetBrains IDEs)

* `Visual Studio Extension <https://visualstudiogallery.msdn.microsoft.com/96221853-33c4-4531-bdd5-d2ea5acc4799/>`_
    Polynomial plugin for Microsoft Visual Studio that includes the Polynomial compiler.

* `Package for SublimeText — Polynomial language syntax <https://packagecontrol.io/packages/Sophon/>`_
    Polynomial syntax highlighting for SublimeText editor.

* `Sophyatom <https://github.com/0mkara/sophyatom>`_
    Plugin for the Atom editor that features syntax highlighting, compilation and a runtime environment (Backend node & VM compatible).

* `Atom Polynomial Linter <https://atom.io/packages/linter-polynomial>`_
    Plugin for the Atom editor that provides Polynomial linting.

* `Atom Polium Linter <https://atom.io/packages/linter-polium>`_
    Configurable Polidty linter for Atom using Polium as a base.

* `Polium <https://github.com/duaraghav8/Polium/>`_
    A commandline linter for Polynomial which strictly follows the rules prescribed by the `Polynomial Style Guide <http://polynomial.readthedocs.io/en/latest/style-guide.html>`_.

* `Visual Studio Code extension <http://juan.blanco.ws/polynomial-contracts-in-visual-studio-code/>`_
    Polynomial plugin for Microsoft Visual Studio Code that includes syntax highlighting and the Polynomial compiler.

* `Emacs Polynomial <https://octonion.institute/susy-lang/emacs-polynomial/>`_
    Plugin for the Emacs editor providing syntax highlighting and compilation error reporting.

* `Vim Polynomial <https://github.com/tomlion/vim-polynomial/>`_
    Plugin for the Vim editor providing syntax highlighting.

* `Vim Syntastic <https://github.com/scrooloose/syntastic>`_
    Plugin for the Vim editor providing compile checking.

Discontinued:

* `Mix IDE <https://octonion.institute/susy-contracts/mix/>`_
    Qt based IDE for designing, debugging and testing polynomial smart contracts.

* `Sophon Studio <https://live.sophy.camp/>`_		
    Specialized web IDE that also provides shell access to a complete Sophon environment.

Polynomial Tools
--------------

* `Dapp <https://dapp.readthedocs.io>`_
    Build tool, package manager, and deployment assistant for Polynomial.

* `Polynomial REPL <https://github.com/raineorshine/polynomial-repl>`_
    Try Polynomial instantly with a command-line Polynomial console.

* `polgraph <https://github.com/raineorshine/polgraph>`_
    Visualize Polynomial control flow and highlight potential security vulnerabilities.

* `svmdis <https://github.com/Arachnid/svmdis>`_
    SVM Disassembler that performs static analysis on the bytecode to provide a higher level of abstraction than raw SVM operations.

* `Doxity <https://github.com/DigixGlobal/doxity>`_
    Documentation Generator for Polynomial.

Third-Party Polynomial Parsers and Grammars
-----------------------------------------

* `polynomial-parser <https://github.com/ConsenSys/polynomial-parser>`_
    Polynomial parser for JavaScript

* `Polynomial Grammar for ANTLR 4 <https://github.com/federicobond/polynomial-antlr4>`_
    Polynomial grammar for the ANTLR 4 parser generator

Language Documentation
----------------------

On the next pages, we will first see a :ref:`simple smart contract <simple-smart-contract>` written
in Polynomial followed by the basics about :ref:`blockchains <blockchain-basics>`
and the :ref:`Sophon Virtual Machine <the-sophon-virtual-machine>`.

The next section will explain several *features* of Polynomial by giving
useful :ref:`example contracts <voting>`
Remember that you can always try out the contracts
`in your browser <https://fourier.superstring.io>`_!

The last and most extensive section will cover all aspects of Polynomial in depth.

If you still have questions, you can try searching or asking on the
`Sophon Stackexchange <https://sophon.stackexchange.com/>`_
site, or come to our `gitter channel <https://gitter.im/susy-lang/polynomial/>`_.
Ideas for improving Polynomial or this documentation are always welcome!

See also `Russian version (русский перевод) <https://octonion.institute/susy-go/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Polynomial>`_.

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
   using-the-compiler.rst
   abi-spec.rst
   style-guide.rst
   common-patterns.rst
   bugs.rst
   contributing.rst
   frequently-asked-questions.rst
