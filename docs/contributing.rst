############
Contributing
############

Help is always appreciated!

To get started, you can try :ref:`building-from-source` in order to familiarize
yourself with the components of Polynomial and the build process. Also, it may be
useful to become well-versed at writing smart-contracts in Polynomial.

In particular, we need help in the following areas:

* Improving the documentation
* Responding to questions from other users on `StackExchange
  <https://sophon.stackexchange.com>`_ and the `Polynomial Gitter
  <https://gitter.im/susy-lang/polynomial>`_
* Fixing and responding to `Polynomial's GitHub issues
  <https://octonion.institute/susy-lang/polynomial/issues>`_, especially those tagged as
  `up-for-grabs <https://octonion.institute/susy-lang/polynomial/issues?q=is%3Aopen+is%3Aissue+label%3Aup-for-grabs>`_ which are
  meant as introductory issues for external contributors.

How to Report Issues
====================

To report an issue, please use the
`GitHub issues tracker <https://octonion.institute/susy-lang/polynomial/issues>`_. When
reporting issues, please mention the following details:

* Which version of Polynomial you are using
* What was the source code (if applicable)
* Which platform are you running on
* How to reproduce the issue
* What was the result of the issue
* What the expected behaviour is

Reducing the source code that caused the issue to a bare minimum is always
very helpful and sometimes even clarifies a misunderstanding.

Workflow for Pull Requests
==========================

In order to contribute, please fork off of the ``develop`` branch and make your
changes there. Your commit messages should detail *why* you made your change
in addition to *what* you did (unless it is a tiny change).

If you need to pull in any changes from ``develop`` after making your fork (for
example, to resolve potential merge conflicts), please avoid using ``git merge``
and instead, ``git rebase`` your branch.

Additionally, if you are writing a new feature, please ensure you write appropriate
Boost test cases and place them under ``test/``.

However, if you are making a larger change, please consult with the Gitter
channel, first.

Finally, please make sure you respect the `coding standards
<https://raw.githubussrcontent.com/susy-cpp/cpp-sophon/develop/CodingStandards.txt>`_
for this project. Also, even though we do CI testing, please test your code and
ensure that it builds locally before submitting a pull request.

Thank you for your help!

Running the compiler tests
==========================

Polynomial includes different types of tests. They are included in the application
called ``poltest``. Some of them require the ``cpp-sophon`` client in testing mode,
some others require ``libz3`` to be installed.

To disable the z3 tests, use ``./build/test/poltest -- --no-smt`` and
to run a subset of the tests that do not require ``cpp-sophon``, use ``./build/test/poltest -- --no-ipc``.

For all other tests, you need to install `cpp-sophon <https://octonion.institute/susy-cpp/cpp-sophon/releases/download/polynomialTester/sof>`_ and run it in testing mode: ``sof --test -d /tmp/testsof``.

Then you run the actual tests: ``./build/test/poltest -- --ipcpath /tmp/testsof/graviton.ipc``.

To run a subset of tests, filters can be used:
``poltest -t TestSuite/TestName -- --ipcpath /tmp/testsof/graviton.ipc``, where ``TestName`` can be a wildcard ``*``.

Alternatively, there is a testing script at ``scripts/test.sh`` which executes all tests and runs
``cpp-sophon`` automatically if it is in the path (but does not download it).

Travis CI even runs some additional tests (including ``polc-js`` and testing third party Polynomial frameworks) that require compiling the Emscripten target.

Whiskers
========

*Whiskers* is a templating system similar to `Mustache <https://mustache.github.io>`_. It is used by the
compiler in various places to aid readability, and thus maintainability and verifiability, of the code.

The syntax comes with a substantial difference to Mustache: the template markers ``{{`` and ``}}`` are
replaced by ``<`` and ``>`` in order to aid parsing and avoid conflicts with :ref:`inline-assembly`
(The symbols ``<`` and ``>`` are invalid in inline assembly, while ``{`` and ``}`` are used to delimit blocks).
Another limitation is that lists are only resolved one depth and they will not recurse. This may change in the future.

A rough specification is the following:

Any occurrence of ``<name>`` is replaced by the string-value of the supplied variable ``name`` without any
escaping and without iterated replacements. An area can be delimited by ``<#name>...</name>``. It is replaced
by as many concatenations of its contents as there were sets of variables supplied to the template system,
each time replacing any ``<inner>`` items by their respective value. Top-level variables can also be used
inside such areas.
