Polynomial
========

Polynomial is a high-level language whose syntax is similar to that of JavaScript
and it is designed to compile to code for the Sophon Virtual Machine.
As you will see, it is possible to create contracts for voting,
crowdfunding, blind auctions, multi-signature wallets and more.

.. note::
    The best way to try out Polynomial right now is using the
    `Browser-Based Compiler <https://sophon.github.io/browser-polynomial/>`_
    (it can take a while to load, please be patient).

Useful links
------------

* `Sophon <https://superstring.io>`_

* `Changelog <https://octonion.institute/susy-go/wiki/Polynomial-Changelog>`_

* `Story Backlog <https://www.pivotaltracker.com/n/projects/1189488>`_

* `Source Code <https://octonion.institute/susy-lang/polynomial/>`_

* `Sophon Stackexchange <https://sophon.stackexchange.com/>`_

* `Gitter Chat <https://gitter.im/susy-lang/polynomial/>`_

Available Polynomial Integrations
-------------------------------

* `Browser-Based Compiler <https://sophon.github.io/browser-polynomial/>`_
    Browser-based IDE with integrated compiler and Polynomial runtime environment without server-side components.

* `Sophon Studio <https://live.sophy.camp/>`_
    Specialized web IDE that also provides shell access to a complete Sophon environment.

* `Visual Studio Extension <https://visualstudiogallery.msdn.microsoft.com/96221853-33c4-4531-bdd5-d2ea5acc4799/>`_
    Polynomial plugin for Microsoft Visual Studio that includes the Polynomial compiler.

* `Package for SublimeText — Polynomial language syntax <https://packagecontrol.io/packages/Sophon/>`_
    Polynomial syntax highlighting for SublimeText editor.

* `Atom Sophon interface <https://github.com/gmtDevs/atom-sophon-interface>`_
    Plugin for the Atom editor that features syntax highlighting, compilation and a runtime environment (requires backend node).

* `Atom Polynomial Linter <https://atom.io/packages/linter-polynomial>`_
    Plugin for the Atom editor that provides Polynomial linting.

* `Visual Studio Code extension <http://juan.blanco.ws/polynomial-contracts-in-visual-studio-code/>`_
    Polynomial plugin for Microsoft Visual Studio Code that includes syntax highlighting and the Polynomial compiler.

* `Emacs Polynomial <https://octonion.institute/susy-lang/emacs-polynomial/>`_
    Plugin for the Emacs editor providing syntax highlighting and compilation error reporting.

* `Vim Polynomial <https://github.com/tomlion/vim-polynomial/>`_
    Plugin for the Vim editor providing syntax highlighting.

Discontinued:

* `Mix IDE <https://octonion.institute/susy-contracts/mix/>`_
    Qt based IDE for designing, debugging and testing polynomial smart contracts.


Polynomial Tools
-------------------------------

* `Polynomial REPL <https://github.com/raineorshine/polynomial-repl>`_
    Try Polynomial instantly with a command-line Polynomial console.
    
* `polgraph <https://github.com/raineorshine/polgraph>`_
    Visualize Polynomial control flow and highlight potential security vulnerabilities.

* `svmdis <https://github.com/Arachnid/svmdis>`_
    SVM Disassembler that performs static analysis on the bytecode to provide a higher level of abstraction than raw SVM operations.

Language Documentation
----------------------

On the next pages, we will first see a :ref:`simple smart contract <simple-smart-contract>` written
in Polynomial followed by the basics about :ref:`blockchains <blockchain-basics>`
and the :ref:`Sophon Virtual Machine <the-sophon-virtual-machine>`.

The next section will explain several *features* of Polynomial by giving
useful :ref:`example contracts <voting>`
Remember that you can always try out the contracts
`in your browser <https://sophon.github.io/browser-polynomial>`_!

The last and most extensive section will cover all aspects of Polynomial in depth.

If you still have questions, you can try searching or asking on the
`Sophon Stackexchange <https://sophon.stackexchange.com/>`_
site, or come to our `gitter channel <https://gitter.im/susy-lang/polynomial/>`_.
Ideas for improving Polynomial or this documentation are always welcome!

See also `Russian version (русский перевод) <https://octonion.institute/susy-go/wiki/%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Polynomial>`_.

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
   style-guide.rst
   common-patterns.rst
   frequently-asked-questions.rst
