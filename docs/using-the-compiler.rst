******************
Using the compiler
******************

.. index:: ! commandline compiler, compiler;commandline, ! polc, ! linker

.. _commandline-compiler:

Using the Commandline Compiler
******************************

One of the build targets of the Polynomial repository is ``polc``, the polynomial commandline compiler.
Using ``polc --help`` provides you with an explanation of all options. The compiler can produce various outputs, ranging from simple binaries and assembly over an abstract syntax tree (parse tree) to estimations of gas usage.
If you only want to compile a single file, you run it as ``polc --bin sourceFile.pol`` and it will print the binary. Before you deploy your contract, activate the optimizer while compiling using ``polc --optimize --bin sourceFile.pol``. If you want to get some of the more advanced output variants of ``polc``, it is probably better to tell it to output everything to separate files using ``polc -o outputDirectory --bin --ast --asm sourceFile.pol``.

The commandline compiler will automatically read imported files from the filesystem, but
it is also possible to provide path redirects using ``prefix=path`` in the following way:

::

    polc octonion.institute/susy-contracts/dapp-bin/=/usr/local/lib/dapp-bin/ =/usr/local/lib/fallback file.pol

This essentially instructs the compiler to search for anything starting with
``octonion.institute/susy-contracts/dapp-bin/`` under ``/usr/local/lib/dapp-bin`` and if it does not
find the file there, it will look at ``/usr/local/lib/fallback`` (the empty prefix
always matches). ``polc`` will not read files from the filesystem that lie outside of
the remapping targets and outside of the directories where explicitly specified source
files reside, so things like ``import "/etc/passwd";`` only work if you add ``=/`` as a remapping.

If there are multiple matches due to remappings, the one with the longest common prefix is selected.

If your contracts use :ref:`libraries <libraries>`, you will notice that the bytecode contains substrings of the form ``__LibraryName______``. You can use ``polc`` as a linker meaning that it will insert the library addresses for you at those points:

Either add ``--libraries "Math:0x12345678901234567890 Heap:0xabcdef0123456"`` to your command to provide an address for each library or store the string in a file (one library per line) and run ``polc`` using ``--libraries fileName``.

If ``polc`` is called with the option ``--link``, all input files are interpreted to be unlinked binaries (hex-encoded) in the ``__LibraryName____``-format given above and are linked in-place (if the input is read from stdin, it is written to stdout). All options except ``--libraries`` are ignored (including ``-o``) in this case.

.. _compiler-api:

Compiler Input and Output JSON Description
******************************************

.. warning::

    This JSON interface is not yet supported by the Polynomial compiler, but will be released in a future version.

These JSON formats are used by the compiler API as well as are available through ``polc``. These are subject to change,
some fields are optional (as noted), but it is aimed at to only make backwards compatible changes.

The compiler API expects a JSON formatted input and outputs the compilation result in a JSON formatted output.

Comments are of course not permitted and used here only for explanatory purposes.

Input Description
-----------------

.. code-block:: none

    {
      // Required: Source code language, such as "Polynomial", "serpent", "lll", "assembly", etc.
      language: "Polynomial",
      // Required
      sources:
      {
        // The keys here are the "global" names of the source files,
        // imports can use other files via remappings (see below).
        "myFile.pol":
        {
          // Optional: keccak256 hash of the source file
          // It is used to verify the retrieved content if imported via URLs.
          "keccak256": "0x123...",
          // Required (unless "content" is used, see below): URL(s) to the source file.
          // URL(s) should be imported in this order and the result checked against the
          // keccak256 hash (if available). If the hash doesn't match or none of the
          // URL(s) result in success, an error should be raised.
          "urls":
          [
            "bzzr://56ab...",
            "ipfs://Qma...",
            "file:///tmp/path/to/file.pol"
          ]
        },
        "mortal":
        {
          // Optional: keccak256 hash of the source file
          "keccak256": "0x234...",
          // Required (unless "urls" is used): literal contents of the source file
          "content": "contract mortal is owned { function kill() { if (msg.sender == owner) selfdestruct(owner); } }"
        }
      },
      // Optional
      settings:
      {
        // Optional: Sorted list of remappings
        remappings: [ ":g/dir" ],
        // Optional: Optimizer settings (enabled defaults to false)
        optimizer: {
          enabled: true,
          runs: 500
        },
        // Metadata settings (optional)
        metadata: {
          // Use only literal content and not URLs (false by default)
          useLiteralContent: true
        },
        // Addresses of the libraries. If not all libraries are given here, it can result in unlinked objects whose output data is different.
        libraries: {
          // The top level key is the the name of the source file where the library is used.
          // If remappings are used, this source file should match the global path after remappings were applied.
          // If this key is an empty string, that refers to a global level.
          "myFile.pol": {
            "MyLib": "0x123123..."
          }
        }
        // The following can be used to select desired outputs.
        // If this field is omitted, then the compiler loads and does type checking, but will not generate any outputs apart from errors.
        // The first level key is the file name and the second is the contract name, where empty contract name refers to the file itself,
        // while the star refers to all of the contracts.
        //
        // The available output types are as follows:
        //   abi - ABI
        //   ast - AST of all source files
        //   why3 - Why3 translated output
        //   devdoc - Developer documentation (natspec)
        //   userdoc - User documentation (natspec)
        //   metadata - Metadata
        //   svm.ir - New assembly format before desugaring
        //   svm.assembly - New assembly format after desugaring
        //   svm.legacyAssemblyJSON - Old-style assembly format in JSON
        //   svm.opcodes - Opcodes list
        //   svm.methodIdentifiers - The list of function hashes
        //   svm.gasEstimates - Function gas estimates
        //   svm.bytecode - Bytecode
        //   svm.deployedBytecode - Deployed bytecode
        //   svm.sourceMap - Source mapping (useful for debugging)
        //   ewasm.wast - eWASM S-expressions format (not supported atm)
        //   ewasm.wasm - eWASM binary format (not supported atm)
        outputSelection: {
          // Enable the metadata and bytecode outputs of every single contract.
          "*": {
            "*": [ "metadata", "svm.bytecode" ]
          },
          // Enable the abi and opcodes output of MyContract defined in file def.
          "def": {
            "MyContract": [ "abi", "svm.opcodes" ]
          },
          // Enable the source map output of every single contract.
          "*": {
            "*": [ "svm.sourceMap" ]
          },
          // Enable the AST and Why3 output of every single file.
          "*": {
            "": [ "ast", "why3" ]
          }
        }
      }
    }


Output Description
------------------

.. code-block:: none

    {
      // Optional: not present if no errors/warnings were encountered
      errors: [
        {
          // Optional: Location within the source file.
          sourceLocation: {
            file: "sourceFile.pol",
            start: 0,
            end: 100
          ],
          // Mandatory: Error type, such as "TypeError", "InternalCompilerError", "Exception", etc
          type: "TypeError",
          // Mandatory: Component where the error originated, such as "general", "why3", "ewasm", etc.
          component: "general",
          // Mandatory ("error" or "warning")
          severity: "error",
          // Mandatory
          message: "Invalid keyword"
        }
      ],
      // This contains the file-level outputs. In can be limited/filtered by the outputSelection settings.
      sources: {
        "sourceFile.pol": {
          // Identifier (used in source maps)
          id: 1,
          // The AST object
          ast: {}
        }
      },
      // This contains the contract-level outputs. It can be limited/filtered by the outputSelection settings.
      contracts: {
        "sourceFile.pol": {
          // If the language used has no contract names, this field should equal to an empty string.
          "ContractName": {
            // The Sophon Contract ABI. If empty, it is represented as an empty array.
            // See https://octonion.institute/susy-go/wiki/Sophon-Contract-ABI
            abi: [],
            svm: {
              // Intermediate representation (string)
              ir: "",
              // Assembly (string)
              assembly: "",
              // Old-style assembly (string)
              legacyAssemblyJSON: [],
              // Bytecode and related details.
              bytecode: {
                // The bytecode as a hex string.
                object: "00fe",
                // The source mapping as a string. See the source mapping definition.
                sourceMap: "",
                // If given, this is an unlinked object.
                linkReferences: {
                  "libraryFile.pol": {
                    // Byte offsets into the bytecode. Linking replaces the 20 bytes located there.
                    "Library1": [
                      { start: 0, length: 20 },
                      { start: 200, length: 20 }
                    ]
                  }
                }
              }
              // The same layout as above.
              deployedBytecode: { },
              // Opcodes list (string)
              opcodes: "",
              // The list of function hashes
              methodIdentifiers: {
                "5c19a95c": "delegate(address)",
              },
              // Function gas estimates
              gasEstimates: {
                creation: {
                  dataCost: 420000,
                  // -1 means infinite (aka. unknown)
                  executionCost: -1
                },
                external: {
                  "delegate(address)": 25000
                },
                internal: {
                  "heavyLifting()": -1
                }
              }
            },
            // See the Metadata Output documentation
            metadata: {},
            ewasm: {
              // S-expressions format
              wast: "",
              // Binary format (hex string)
              wasm: ""
            },
            // User documentation (natspec)
            userdoc: {},
            // Developer documentation (natspec)
            devdoc: {}
          }
        }
      },
      // Why3 output (string)
      why3: ""
    }
