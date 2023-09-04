# BadgerOS project structure

There are three levels of abstraction: CPU, platform and kernel.
The CPU level contains divers for CPU-specific logic like interrupt handling,
the platform level contains drivers for hardware inside the CPU and
the kernel level contains the business logic that allows applications to run.

The folder structure reflects this:
| path           | purpose
| :------------- | :------
| src            | Kernel-implemented source
| include        | Kernel-declared includes
| cpu/*/src      | CPU-implemented source
| cpu/*/include  | CPU-declared include
| port/*/src     | Platform-implemented source
| port/*/include | Platform-declared include

In the future, there will be more details on the structure these folders will have internally but for now it's flat.

## The source folders
This applies to `src`, `cpu/*/src` and `port/*/src`.

The source folders contain exclusively C files and files to embed into the kernel verbatim.
If you're not sure where to put a new function, consider:
- Does the function depend on the target platform?
    - If so, it should be in `port/*/src`
- Does the function depend on the CPU, but not the target platform?
    - If so, it should be in `cpu/*/src`
- Can the function be described in a platform- and CPU-independent way?
    - If so, it should be in `src`

## The include folders
This applies to `include`, `cpu/*/include` and `port/*/include`.

The include folders contain exclusively C header files.
If you're not sure where to put a new definition, consider:
- Does the value of a it or signature of a function depend on the target platform?
    - If so, it should be in `port/*/include/*`
- Does the value of a it or signature of a function depend on the CPU, but not the target platform?
    - If so, it should be in `cpu/*/include/*`
- Can the it be described in a platform- and CPU-independent way?
    - If so, it should be in `include`

_Note: The value of `*` may change, but is always the same in a path: `port/abc/include/abc` is valid, but `port/one/include/another` is not._
