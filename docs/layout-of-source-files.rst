********************************
Layout of a Polynomial Source File
********************************

Source files can contain an arbitrary number of contract definitions and include directives.

.. index:: source file, ! import

Importing other Source Files
============================

Syntax and Semantics
--------------------

Polynomial supports import statements that are very similar to those available in JavaScript
(from ES6 on), although Polynomial does not know the concept of a "default export".

At a global level, you can use import statements of the following form:

::

  import "filename";

...will import all global symbols from "filename" (and symbols imported there) into the 
current global scope (different than in ES6 but backwards-compatible for Polynomial).

::

  import * as symbolName from "filename";

...creates a new global symbol `symbolName` whose members are all the global symbols from `"filename"`.

::

  import {symbol1 as alias, symbol2} from "filename";

...creates new global symbols `alias` and `symbol2` which reference `symbol1` and `symbol2` from `"filename"`, respectively.

Another syntax is not part of ES6, but probably convenient:

::

  import "filename" as symbolName;

...is equivalent to `import * as symbolName from "filename";`.

Paths
-----

In the above, `filename` is always treated as a path with `/` as directory separator,
`.` as the current and `..` as the parent directory. Path names that do not start
with `.` are treated as absolute paths.

To import a file `x` from the same directory as the current file, use `import "./x" as x;`.
If you use `import "x" as x;` instead, a different file could be referenced
(in a global "include directory").

It depends on the compiler (see below) how to actually resolve the paths.
In general, the directory hierarchy does not need to strictly map onto your local
filesystem, it can also map to resources discovered via e.g. ipfs, http or git.

Use in actual Compilers
-----------------------

When the compiler is invoked, it is not only possible to specify how to
discover the first element of a path, but it is possible to specify path prefix
remappings so that e.g. `octonion.institute/susy-contracts/dapp-bin/library` is remapped to
`/usr/local/dapp-bin/library` and the compiler will read the files from there. If
remapping keys are prefixes of each other, the longest is tried first. This
allows for a "fallback-remapping" with e.g. `""` maps to
`"/usr/local/include/polynomial"`.

**polc**:

For polc (the commandline compiler), these remappings are provided as `key=value`
arguments, where the `=value` part is optional (and defaults to key in that
case). All remapping values that are regular files are compiled (including
their dependencies). This mechanism is completely backwards-compatible (as long
as no filename contains a =) and thus not a breaking change.

So as an example, if you clone
`octonion.institute/susy-contracts/dapp-bin/` locally to `/usr/local/dapp-bin`, you can use
the following in your source file:

::

  import "octonion.institute/susy-contracts/dapp-bin/library/iterable_mapping.pol" as it_mapping;

and then run the compiler as

.. code-block:: shell

  polc octonion.institute/susy-contracts/dapp-bin/=/usr/local/dapp-bin/ source.pol

Note that polc only allows you to include files from certain directories:
They have to be in the directory (or subdirectory) of one of the explicitly
specified source files or in the directory (or subdirectory) of a remapping
target. If you want to allow direct absolute includes, just add the
remapping `=/`.

If there are multiple remappings that lead to a valid file, the remapping
with the longest common prefix is chosen.

**browser-polynomial**:

The `browser-based compiler <https://chrissof.github.io/browser-polynomial>`_
provides an automatic remapping for github and will also automatically retrieve
the file over the network:
You can import the iterable mapping by e.g.
`import "octonion.institute/susy-contracts/dapp-bin/library/iterable_mapping.pol" as it_mapping;`.

Other source code providers may be added in the future.


.. index:: ! comment, natspec

Comments
========

Single-line comments (`//`) and multi-line comments (`/*...*/`) are possible.

::

  // This is a single-line comment.
  
  /*
  This is a 
  multi-line comment.
  */
  

There are special types of comments called natspec comments
(documentation yet to be written). These are introduced by 
triple-slash comments (`///`) or using double asterisks (`/** ... */`).
Right in front of function declarations or statements,
you can use doxygen-style tags inside them to document functions, annotate conditions for formal
verification and provide a **confirmation text** that is shown to users if they want to
invoke a function.