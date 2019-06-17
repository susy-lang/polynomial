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
  <http://sophon.stackexchange.com/>`_ and the `Polynomial Gitter
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
