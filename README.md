# ShaGa library
ShaGa library contains collection of functions that are helpful in cross-platform and cross-architecture development. It requires C++17 compiler and should compile in
GNU/Linux (primary platform), Windows and MacOS X.

Basic fields covered by this library:
* Binary operations - little and big endian
* String operations
* File-system operations
* IPv4/IPv6 address manipulation
* INI files
* Argument parsing
* Chunk operations
* Digital signatures (using [mbed TLS](https://tls.mbed.org/))
* CRC, SipHash, HalfSipHash
* Single producer single consumer queues
* Possibility to compile with or without multithreading support

There is a full and lite version of the library. Lite version should compile without additional libraries. Full version requires [mbed TLS](https://tls.mbed.org/).

## Usage
Four static libraries are produced. There are also four header files, that are to be used with these libraries:
* Full multi-thread:
	- Header: shaga_mt.h
	- Library: libshaga_mt.a
* Full single-thread:
	- Header: shaga_st.h
	- Library: libshaga_st.a
* Lite multi-thread (without mbedTLS):
	- Header: shagalite_mt.h
	- Library: libshagalite_mt.a
* Lite single-thread (without mbedTLS):
	- Header: shagalite_st.h
	- Library: libshagalite_st.a

## History
The library was developer as an internal project in 2012. Since then, it has been updated and modified several times, from C++98, C++03, C++11, C++14 to current C++17.
It is still being used by some old internal projects and therefore it (unfortunately) retains some of its original design. There is a major overhaul planned after C++20 that
will hopefully allow dropping of some outdated functions.

License was changed in 2014 to the [BSD license](LICENSE.md). It also contains code downloaded from the internet. All sources are hopefully documented and mentioned in header
files and also in the [license document](LICENSE.md).

## Single-thread vs. multi-thread version
You can compile using single-thread and/or multi-thread version of the library. This possibility was added to speed-up execution of applications when multi-threading
is not required. Using atomic operations and mutex locking is quite expensive on some operating systems and architectures. By providing single-thread version of the library,
the programmer may avoid using atomics and mutexes altogether and compile without pthread.

## Sanitizers and safety
It is possible to enable AddressSanitizer, LeakSanitizer and UndefinedBehaviorSanitizer by setting environment variable SHAGA_SANITY before calling make. This will add
these compiler options: `-fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer`. You need to install both libasan and libubsan for this to work
properly.

Code is compiled with `-Wall -Wextra -Wshadow -fstack-protector-strong` parameters and all warning are fixed during development and testing.
Since the library is intended to be used only as static, it is compiled with -fPIE and -pie.

## Current progress and future development
I am currently working on unit tests using [Google Test](https://github.com/google/googletest) that were missing in the original library and slowly modifying
source code to use C++17 types and structures (like string_view). I am also planning to add documentation in doxygen format and cmake building process.

## Help needed
If you are willing to help with documentation, unit tests or building process, please, fork this library. Any help is appreciated.
The goal is to be able to compile library using any C++17 compiler. Right now, I am using gcc9 and clang9 during development.

## Name
Name of the library comes from my favorite band [Shadow Gallery]( https://en.wikipedia.org/wiki/Shadow_Gallery).
I was basically listening to their music exclusively while working on the code.

## Contact
You can contact me using email address skupka@sageteam.eu or visit [my homepage](https://www.bwpow.eu/).

If you would like to support my work, send me some cryptocoins:
* BTC: bc1qmjr8rrft6adqv2xjj226kttwsmpy7shf4dufcj
* XMR: 82Uy7LbE411dNrTWSHjf6jKCQG59mveSNDh3ps1KFVj5NZ8Gzj1avNDTG4hJZpyDck5JcPbyowSV3RiUC7cC71zQ7y5a7km
* Doge: DTuiDgZyqobQS5wMWU2wCsubpGo5w7LDRn
