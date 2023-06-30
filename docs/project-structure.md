# BadgerOS project structure

There are three levels of abstraction: CPU, platform and kernel.
The CPU level contains divers for CPU-specific logic like interrupt handling,
the platform level contains drivers for attached hardware and
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
