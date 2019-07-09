# IVT

IVT is a tool that helps sharing polymorphic objects.

## The solved problem:

Polymorphic objects contain pointers (vptr) that point to virtual tables (vtable).
Therefore, polymorphic objects can not be shared through shared memory since these pointers make no sense to another process.
By using our IVT (Indexed Virtual Table), we can share polymorphic objects efficiently.

## Environment:

LLVM version 7.0.0, boost 1.54 and Ubuntu 14.04 are used when testing.

Please let me know if there are issues on other versions.

## Compilation:

Simply type "make" under the source directory.
If everything goes well, you will see two executables, i.e. "ivt_change" and "ivt_gen".

## Usage:

Write a specification (ivt.ivt in the following example) of the classes that involved in sharing.

Use LLVM tools (e.g. clang++ -emit-llvm -c) to compile each source code (.cpp) to bit code (.bc).

Use ivt_change to modify the bit code (.bc) to a changed bit code file (.bcc). In this .bcc file, objects use indexes instead of pointers to find their corresponding vtables.

Use ivt_gen to generate a source file (ivt.cpp in the following example) that contains the definition of the VPTA (virtual pointer array).

Use LLVM tools to continue compilation and linking (e.g. llc and clang++).

## Example:

Under the /example directory, there is an example.

cls.cpp and cls.hpp define several classes.

snd.cpp creates some objects in a shared memory region, and rec.cpp tries to find them and call a method.

### Normal procedure:

Type "make" under the /example directory, two executables (snd and rec) are created.

Run snd. It will create some objects in a shared memory region and wait for keyboard input. Just leave it here until we see the result of rec.

Run rec. It can find the objects in the shared memory region, but fail to call any methods (Segment Fault triggered). This is because the pointers to virtual tables stored in the objects do not make sense to the rec process.

### IVT procedure:

Open the /example/ivt.ivt file. It is the specification of classes involved in sharing. There are some comments that explain itself.

This file will be modified by the ivt_change during the following procedure. You can make a backup of this file if you want to see the modifications. We have already make a backup at /example/ivt.bak, and use it to replace /example/ivt.ivt when "make clean" is performed.

Type "make ivt" under the /example directory, two new executables (snd_ivt and rec_ivt) are created.

Run snd_ivt, and leave it waiting for keyboard input until we see the result of rec_ivt.

Run rec_ivt. If everything goes well, you can see the virtual methods are successfully called. This result shows that IVT works well.

## Work on progress:

Currently, the specification file is written by developer to designate the classes that involved in sharing.

Static analysis tool could help to generate this specification. Since IVT does not introduce much overhead, we could simply generate a specification that includes all polymorphic classes.
