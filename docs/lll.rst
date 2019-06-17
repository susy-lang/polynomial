###
LLL
###

.. _lll:

LLL is a low-level language for the SVM with an s-expressions syntax.

The Polynomial repository contains an LLL compiler, which shares the assembler subsystem with Polynomial.
However, apart from maintaining that it still compiles, no other improvements are made to it.

It is not built unless specifically requested:

.. code-block:: bash

    $ cmake -DLLL=ON ..
    $ cmake --build .

.. warning::

    The LLL codebase is deprecated and will be removed from the Polynomial repository in the future.
